#include "validator.h"
#include <QDebug>

Validator::Validator(QSettings& settings,Mode mode, QString basePath) :
    _good(true), pattern(settings.value("dirFormat","${artist:start}/${artist} - ${title}/${artist} - ${title}!{COV:_COV}!{BG:_BG}.${suffix}").toString()),
    fixMode(mode),  basePath(basePath)
{
    //Compiling pattern ...
    //Find possible tags ...

    int lastPos = 0;
    int idx = 0;
    QRegExp onlyLettersOrNumbers("^\\w*$");
    QString genPattern = pattern;
    QRegExp conditionals("!\\{(BG|VID|COV|TXT|MP3):([^\\}]*)\\}",Qt::CaseInsensitive);
    while (genPattern.contains(conditionals)) genPattern.replace(conditionals,"\\2");

    while ( (idx = pattern.indexOf("${",lastPos)) != -1) {
        int end = pattern.indexOf("}",idx);
        if (end == -1) {
            qWarning() << "Missing closing brace!";
            _good = false;
            return;
        }
        QString tag = pattern.mid(idx+2,end-idx-2);
        QList<QString>* target;
        if (!tag.contains(':')) {
            if (!tag.contains(onlyLettersOrNumbers)) {
                qWarning() << "Tags may only contain letters or numbers! Did you forget a closing brace?" << tag;
                _good = false;
                return;
            }
            target = &exact;
        } else {
            QString type = tag.mid(tag.indexOf(':')+1);
            tag = tag.left(tag.indexOf(':'));
            if (type.toUpper() == "START") target = &start;
            else if (type.toUpper() == "UPPER") target = &upper;
            else if (type.toUpper() == "LOWER") target = &lower;
            else {
                qWarning() << "INVALID Tag in string!"; //TODO! Capture other invalid cases!
                _good = false;
                return;
            }
        }
        if (!target->contains(tag)) {
            target->append(tag);
        }
        lastPos = end;
    }
//    qWarning() << "DONE!";
    QRegExp txtConditional("!\\{TXT:([^\\}]*)\\}",Qt::CaseInsensitive);
    QRegExp mp3Conditional("!\\{MP3:([^\\}]*)\\}",Qt::CaseInsensitive);
    QRegExp covConditional("!\\{COV:([^\\}]*)\\}",Qt::CaseInsensitive);
    QRegExp vidConditional("!\\{VID:([^\\}]*)\\}",Qt::CaseInsensitive);
    QRegExp bgConditional("!\\{BG:([^\\}]*)\\}",Qt::CaseInsensitive);
    QRegExp allConditional("!\\{[^\\}]*\\}",Qt::CaseInsensitive);
    txtPattern = mp3Pattern = covPattern = vidPattern = bgPattern = pattern;
    while (txtPattern.contains(txtConditional)) txtPattern.replace(txtConditional,"\\1");
    while (mp3Pattern.contains(mp3Conditional)) mp3Pattern.replace(mp3Conditional,"\\1");
    while (covPattern.contains(covConditional)) covPattern.replace(covConditional,"\\1");
    while (vidPattern.contains(vidConditional)) vidPattern.replace(vidConditional,"\\1");
    while (bgPattern.contains(bgConditional)) bgPattern.replace(bgConditional,"\\1");
    for (QString* s : {&txtPattern, &mp3Pattern, &covPattern, &vidPattern, &bgPattern}) {
        s->remove(allConditional);
    }
}


QString Validator::songPattern(Type t, Song *song) {
    QString p;
    QFileInfo f;
    switch (t) {
        case Type::MP3: p = mp3Pattern;
            f = song->mp3();
            break;
        case Type::BG: p = bgPattern;
            f = song->bg();
            break;
        case Type::COV: p = covPattern;
            f = song->cov();
            break;
        case Type::TXT: p = txtPattern;
            f = song->txt();
            break;
        case Type::VID: p = vidPattern;
            f = song->vid();
            break;
        case Type::ALL:
            qWarning() << "Invalid parameter! Can not generated pattern for all!";
            return "false{}";
    }
    p.replace("${suffix}",f.suffix());
    //TODO: Handle what happens if tag does not exist!
    for (QString tag : exact)
        p.replace(QString("${%1}").arg(tag),song->tag(tag));
    for (QString tag : upper)
        p.replace(QString("${%1}").arg(tag),song->tag(tag).toUpper());
    for (QString tag : lower)
        p.replace(QString("${%1}").arg(tag),song->tag(tag).toLower());
    for (QString tag : title)
        p.replace(QString("${%1}").arg(tag),song->tag(tag).toLower());

    return p;
}

bool Validator::validatePath(Type t, Song *song, QString path) {
    QString pattern = QRegExp::escape(songPattern(t,song));
    pattern.replace("\\$\\{","${");
    pattern.replace("\\}","}"); //This might lead to problems!
    for (QString s : start) {
        QStringList sl = song->tag(s).split(' ');
        QString matcher = sl.front();
        sl.removeFirst();
        for (QString part : sl) {
            matcher.append(QString("( %1").arg(QRegExp::escape(part)));
        }
        for (int i = 0; i < sl.size(); i++) {
            matcher.append(")?");
        }
        pattern.replace(QString("${%1:start}").arg(s),matcher,Qt::CaseInsensitive);
    }
    qWarning() << "Valid path regex:" << pattern;
    qWarning() << "Compared path:   " << path;
    pattern.prepend('^');
    pattern.append('$');
    return path.contains(QRegExp(pattern,Qt::CaseSensitive));
}

static QStringList pathReplace(QList<QString> needles,QString pattern, Song* song) {
    if (needles.empty()) return QStringList();
    QStringList res;
    QString curNeedle = needles.first();
    needles.removeFirst();
    //TODO: Need to handle missing needle?
    QStringList sl = song->tag(curNeedle).split(' ');
    QString currentVal = sl.first();
    sl.removeFirst();
    for (QString part : sl) {
        QString localPattern = pattern;
        localPattern.replace(QString("{%1:start}").arg(curNeedle),currentVal,Qt::CaseInsensitive);
        res.append(pathReplace(needles,localPattern,song));
        currentVal.append(QString(" %1").arg(part));
    }
    QString localPattern = pattern;
    localPattern.replace(QString("{%1:start}").arg(curNeedle),currentVal,Qt::CaseInsensitive);
    res.append(pathReplace(needles,localPattern,song));
    return res;
}

QStringList Validator::possiblePaths(Song *song, Type t) {
    QString pattern = songPattern(t,song);
    return pathReplace(start,pattern,song);
}

QString Validator::reducePath(QString path) {
    if (!path.startsWith(basePath)) {
        qWarning() << "This is not a valid song for this validator!";
        return "false{}";
    }
    path = path.right(path.length()-basePath.length());
    if (path.startsWith('/')) path = path.right(path.length()-1);
    return path;
}

bool Validator::validate(Song *song, Type t) {
    if (!_good) return false;
    //Check only txt for now ...
    qWarning() << "Got pattern:     " << songPattern(t,song);
    if (t == Type::ALL) {
        if (!validatePath(Type::TXT,song,reducePath(song->txt().filePath()))) return false;
        if (!validatePath(Type::MP3,song,reducePath(song->mp3().filePath()))) return false;
        if (song->hasBG() && !validatePath(Type::BG,song,reducePath(song->bg().filePath()))) return false;
        if (song->hasVideo() && !validatePath(Type::VID,song,reducePath(song->vid().filePath()))) return false;
        if (song->hasCover() && !validatePath(Type::COV,song,reducePath(song->cov().filePath()))) return false;
        return true;
    }
    switch (t) {
        case Type::MP3: return validatePath(Type::MP3,song,reducePath(song->mp3().filePath()));
        case Type::TXT: return validatePath(Type::TXT,song,reducePath(song->txt().filePath()));
        case Type::BG:  return validatePath(Type::BG,song,reducePath(song->bg().filePath()));
        case Type::VID: return validatePath(Type::VID,song,reducePath(song->vid().filePath()));
        case Type::COV: return validatePath(Type::COV,song,reducePath(song->cov().filePath()));
        case Type::ALL: __builtin_unreachable();
    }
    __builtin_unreachable();
}

bool Validator::good() {
    return _good;
}

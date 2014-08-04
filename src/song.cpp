#include "song.h"
#include "validator.h"
#include <pathinstanceselector.h>
#include <QMessageBox>
#include "sylabel.h"

QStringList Song::seenTags;
QPixmap* Song::_noCover = nullptr;
QPixmap* Song::_coverMissing = nullptr;

bool Song::answeredToAll = false;
bool Song::yesToAll;

Song::Song(const QFileInfo& source, Validator* val, const QString basePath) :
    _bpm(0), _gap(0), _basePath(basePath),  _txt(source), validator(val)
{
    if (!source.exists()) {
        qWarning() << "Tried to load non-existing song" << source.filePath() << "! Unable to determine canonical path";
        valid = false;
        return;
    }
    QFile ifile(source.canonicalFilePath());
    if (!ifile.open(QIODevice::ReadOnly)) {
        valid = false;
        return;
    }
    QTextStream in(&ifile);
    bool endOfTags = false;
    int lineNo = 0;
    int curPlayer = 1;
    while (!in.atEnd()) {
        lineNo++;
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        if (line.startsWith('#')) {
            if (endOfTags) {
                wellFormed = false;
                qWarning() << QString("[%1:%2]: Tags after end of tag section. Discarding Tag!").arg(source.filePath()).arg(lineNo);
                continue;
            }
            line.remove(0,1);
            QStringList elements = line.split(':');
            if (elements.length() != 2) {
                wellFormed = false;
                qWarning() << QString("[%1:%2]: Malformed Tag! Skipping.").arg(source.filePath()).arg(lineNo);
                continue;
            }
            addTag(elements.first(),elements.last());
        } else if (line.startsWith(':') ||
                   line.startsWith('*') ||
                   line.startsWith('F') ||
                   line.startsWith('-')) { //Linebreak (this should not be the first! TODO?)
            endOfTags = true;
            if (_bpm == -1) {
                wellFormed = false;
                qWarning() << QString("[%1]: Contains no BPM!").arg(source.filePath());
                valid =false;
                return;
            }
            Sylabel* syl = new Sylabel(line,curPlayer,this);
            connect(syl,&Sylabel::updated,this,&Song::updated);
            musicAndLyrics.append(syl);
        } else if (line.startsWith('E')) { //End of file
            if (!in.atEnd()) {
                qWarning() << QString("[%1]: Data beyond end of file!").arg(source.filePath());
                wellFormed = false;
                break;
            }
        } else if (line.startsWith('P')) { //TODO: This does not implement nested players! And we do not check the player count tag ...
            bool ok;
            int newPlayer = line.trimmed().remove(0,1).toInt(&ok);
            if (!ok) {
//                qWarning() << QString("[%1:%2]: Malformed playernumber!!").arg(source.filePath()).arg(lineNo);
                wellFormed = false;
            }
            if (_players+1 != newPlayer) {
//                qWarning() << QString("[%1:%2]: Found player number %3 without having a number %4!!").
//                              arg(source.filePath()).arg(lineNo).arg(newPlayer).arg(newPlayer-1);
                wellFormed = false;
            }
            curPlayer = newPlayer;
	    _players = newPlayer;
        } else {
            qWarning() << QString("[%1]: Line \"%2\" could not be interpreted!").arg(source.filePath(),line);
            wellFormed = false;
        }
    }
    std::sort(musicAndLyrics.begin(),musicAndLyrics.end(),[this] (const Sylabel* s1, const Sylabel* s2) {
        if (s1->beat() == s2->beat()) {
            if (s1->isLineBreak()) return true;
            if (s2->isLineBreak()) return false;
            qWarning() << "This can not be. Two notes share the same beat";
            wellFormed = false;
        }
        return s1->beat() < s2->beat();
    });
    initialized = true;
    int avg = 0;
    for (Sylabel* s : musicAndLyrics) {
        avg += static_cast<signed char>(s->key());
    }
    avg/=musicAndLyrics.size();
    qWarning() << title() << avg;
    if (avg < 30) {
        bool answer = false;
        if (!answeredToAll) {
            int res = QMessageBox::question(nullptr,"Wrong base pitch",
                          QString("The song <b>%1</b> by <b>%2</b> seems to be set at a way too low average key of %3.\n"
                                  "The usual cause for this is that the song creator assumed that 0 equals to C-1 instead of C4."
                                  "Should I automatically transpose the song for 5 octaves?\n"
                                  "You can always do this manually in the notes view, but until you do this view may look broken!")
                          .arg(title()).arg(artist()).arg(avg),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::YesAll | QMessageBox::NoAll);
            switch (res) {
                case QMessageBox::YesAll:
                    answeredToAll = true;
                    yesToAll = true;
                case QMessageBox::Yes:
                    answer = true;
                    break;
                case QMessageBox::NoAll:
                    answeredToAll = true;
                    yesToAll = false;
                case QMessageBox::No:
                    answer = false;
                    break;
                default:
                    answer = false;
            }
        } else {
            answer = yesToAll;
        }
        if (answer) {
            for (Sylabel* s : musicAndLyrics)
                s->transpose(60);
        }
    }
}

bool Song::setFile(QFileInfo &info,const QString& path) {
    info.setFile(_txt.dir(),path);
    if (!info.exists()) {
        qWarning() << QString("[%1]: File %2 not found!").arg(_txt.filePath(),path);
        wellFormed = false;
    }
    return info.exists();
}

bool Song::toDouble(const QString &value, double& target) {
    bool ok;
    target = QString(value).replace(",",".").toDouble(&ok);
    if (!ok) {
        qWarning() << QString("[%1]: %2 is not a double!").arg(_txt.filePath(),value);
        valid =false;
    }
    return ok;
}

//This may fail when moving in Filesystem atm!
bool Song::setTag(const QString &tag, const QString &value) {
    if (value.trimmed().isEmpty()) {
        qWarning() << QString("[%1]: Tag (\"%2\") without value! Skipping.").arg(_txt.filePath()).arg(tag);
        wellFormed = false;
        return false;
    }
    tags[tag] = value;
    if (tag == "MP3")        if (!setFile(_mp3,value)) return false;
    if (tag == "COVER")      if (!setFile(_cov,value)) return false;
    if (tag == "BACKGROUND") if (!setFile(_bg,value))  return false;
    if (tag == "VIDEO")      if (!setFile(_vid,value)) return false;
    if (tag == "BPM")        if (!toDouble(value,_bpm)) return false;
    if (tag == "GAP")        if (!toDouble(value,_gap)) return false;

    emit updated();
    if (initialized) {
        if (validator->isPathTag(tag)) {
            //We only do this if initialization went well!
            //check if we need reselection of path (due to changed variable tag)
            //Moving is tbd ...TODO!
            if (0 && validator->isVariable(tag)) {
                qWarning() << "Current TXT path: " << _txt.absoluteFilePath().remove(0,_basePath.length());
                QStringList choice = validator->possiblePaths(this,Validator::Type::TXT);
                qWarning() << "Updated TXT path: " << choice;
                assert(choice.size() >= 1);
                QString sel;
                if (choice.size() == 1) {
                    sel = choice.first();
                } else {
                    PathInstanceSelector pis(choice);
                    if (pis.exec() == QDialog::Accepted) {
                        sel = pis.getSelected();
                        qWarning() << "Chosen path: " << pis.getSelected();
                    } else {
                        qWarning() << "Abort not possible at the moment!"; //TODO
                    }
                }
            }
        }
    }
    return true;
}

QString Song::rawLyrics() {
    if (_rawTextCache.isEmpty()) {
        for (const Sylabel* s : musicAndLyrics) {
            if (s->isLineBreak()) {
                _rawTextCache.append("\n");
            } else {
                _rawTextCache.append(s->text());
            }
        }
    }
    return _rawTextCache;
}

void Song::updateDataCache() {
    //TODO: THis produces garbage on invalid / not well formed data. Check!
    //We need windows newlines here (as microsoft obviously still believes we live in the age of typewriters)
    _rawDataCache.clear();
    for (const QString& key : tags.keys()) {
        _rawDataCache += QString("#%1: %2\r\n").arg(key,tags[key]);
    }
    for (const Sylabel* s: musicAndLyrics) {
        switch (s->type()) {
            case Sylabel::Type::Bad:        _rawDataCache.append("!"); break;
            case Sylabel::Type::Freestyle:  _rawDataCache.append("F"); break;
            case Sylabel::Type::Golden:     _rawDataCache.append("*"); break;
            case Sylabel::Type::SimpleLineBreak:
            case Sylabel::Type::LineBreak:  _rawDataCache.append("-"); break;
            case Sylabel::Type::Regular:    _rawDataCache.append(":"); break;
        }
        _rawDataCache += QString(" %1").arg(s->beat());
        if (s->isLineBreak()) {
            if (s->type() == Sylabel::Type::LineBreak) {
                _rawDataCache.append(QString(" %1").arg(s->beats()));
            }
            _rawDataCache.append("\r\n");
            continue; //This breaks 2 data line breaks!
        }
        _rawDataCache += QString(" %1 %2 %3\r\n").arg(s->beats()).arg(s->key()).arg(s->text());
    }
}

//WARNING! THis will fail horribly on multiplayer songs atm!


QString Song::rawData() {
    //The cache will be updated by the song itself when it changes (at least in the future ;) )
    updateDataCache();
    return _rawDataCache;
}

bool Song::addTag(const QString &tag, const QString &value) {
    QString _tag = tag.toUpper();
    if (!seenTags.contains(_tag)) seenTags.append(_tag);
    if (tags.contains(_tag)) {
        qWarning() << "The tag" << tag << "already exists in" << _txt.canonicalFilePath() <<". Ignoring!";
        return false;
    }
    return setTag(_tag,value);
}

bool Song::updateTag(const QString &tag, const QString &value) {
    QString _tag = tag.toUpper();
    if (!tags.contains(_tag)) {
        qWarning() << "The tag" << tag << "does not exist in" << _txt.canonicalFilePath() <<" and can not be updated. Ignoring!";
        return false;
    }
    return setTag(_tag,value);
}

bool Song::missingBG() const {
    return hasBG() && !_bg.exists();
}

bool Song::missingVideo() const {
    return hasVideo() && !_vid.exists();
}

bool Song::missingCover() const {
    return hasCover() && !_cov.exists();
}

QString Song::title() const
{
    if (!valid) return _txt.path();
    return tags["TITLE"];
}
QString Song::artist() const {
    return tags["ARTIST"];
}

QString Song::tag(const QString &tag) const {
    if (tags.contains(tag.toUpper()))  return tags[tag.toUpper()];
    return QString::null;
}

bool Song::hasTag(const QString &tag) const {
    return tags.contains(tag.toUpper());
}

QPixmap Song::cover() {
    if (!tag("COVER").isNull()) {
        if (_covPM.isNull()) {
            QPixmap covPM(_cov.canonicalFilePath());
            if (!covPM.isNull()) {
                _covPM = covPM.scaled(96,96,Qt::KeepAspectRatio);
                return _covPM;
            } else {
                if (_coverMissing == nullptr)
                    _coverMissing = new QPixmap(QPixmap(":/images/nocover_notFound.png").scaled(96,96,Qt::KeepAspectRatio));
                return *_coverMissing;
            }
        }
        return _covPM;
    }
    if (_noCover == nullptr)
        _noCover = new QPixmap(QPixmap(":/images/nocover.png").scaled(96,96,Qt::KeepAspectRatio));
    return *_noCover; //TODO: Dummy image!
}

void Song::playing(int ms) {
    //ms to from / to
    int line = 0;
    static int last_line = -1;
    int from = 0;
    ms -= _gap;
    if (ms/1000.0 < musicAndLyrics.first()->time()) return;
    for (Sylabel* s : musicAndLyrics) {
        if (s->isLineBreak()) {
            line++; from++; //Accounts for newline
            continue;
        }
        if (s->time() <= ms/1000.0 && s->time()+s->duration() >= ms/1000.0) {
            emit playingSylabel(from,from+s->text().length());
            emit playingSylabel(s);
            if (line != last_line) {
                emit lineChanged(line);
                last_line = line;
            }
            return;
        }
        from += s->text().length();
    }
}

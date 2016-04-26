#include "song.h"
#include <pathinstanceselector.h>
#include <QMessageBox>
#include "sylabel.h"

#include <exceptions/songparseexception.h>
#include <exceptions/sylabelformatexception.h>
#include <actions/convertrelative.h>
#include <actions/transposesong.h>

QStringList Song::_seenTags;
QPixmap* Song::_noCover = nullptr;
QPixmap* Song::_coverMissing = nullptr;

//using namespace std::rel_ops;

Song::Song(const QFileInfo& source,const QString basePath) :
    _bpm(0), _gap(0), _basePath(basePath),  _txt(source)
{
    using Reason = SongParseException::Reason;

    if (!source.exists())
        throw SongParseException(source.fileName(),Reason::FileDoesNotExist);

    QFile ifile(source.canonicalFilePath());
    if (!ifile.open(QIODevice::ReadOnly))
        throw SongParseException(source.fileName(),Reason::PermissionDenied);

    connect(this,&Song::updated,this,&Song::updateRawLyrics);

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
                _errors << QString("%1 Tags after end of tags section. Discarding tag!").arg(lineNo);
                continue;
            }
            line.remove(0,1);
            QString tag = line.left(line.indexOf(':'));
            QString value = line.remove(0,tag.length()+1);
            addTag(tag,value);
        } else if (line.contains(QRegExp("^[F*:-]"))) {
            endOfTags = true;
            if (_bpm == -1) {
                _errors << "Contains no BPM!";
                //TODO: does this have implications? We should check in the users if
                //this is -1 and then say that its unplayable maybe? Or use an unplayable member
                //return;
            }

            //This may throw upon parse error
            Sylabel* syl = new Sylabel(line,curPlayer,this);
            connect(syl,&Sylabel::updated,this,&Song::updated);
            musicAndLyrics.append(syl);
        } else if (line.startsWith('E')) { //End of file
            if (!in.atEnd()) {
                _warnings << "Data beyond end of file!";
                break;
            }
        } else if (line.startsWith('P')) {
            _warnings << "Multiplayer files are not supported at the Moment.";
        } else {
            throw SongParseException(_txt.canonicalFilePath(),Reason::UnreadableLine);
        }
    }
    if (relativeSource() && !performAction(std::make_unique<ConvertRelative>())) {
        //Is this a failure? I think not ... even though some checks can't run now
        return;
    }

    std::sort(musicAndLyrics.begin(),musicAndLyrics.end(),[this] (const Sylabel* s1, const Sylabel* s2) {
        if (s1->beat() == s2->beat()) {
            if (s1->isLineBreak()) return true;
            if (s2->isLineBreak()) return false;
            _warnings << "This can not be. Two notes share the same beat";
        }
        return s1->beat() < s2->beat();
    });
    //Verify overlapping notes ...
    int last = 0;
    for (Sylabel* s : musicAndLyrics) {
        if (s->beat() < last) _warnings << QString("Sylabel at %1 overlaps with previous!").arg(s->beat());
        last = s->beat();
        if (!s->isLineBreak())
            last += s->beats();
    }
    int avg = 0;
    for (Sylabel* s : musicAndLyrics)
        if (!s->isLineBreak())
            avg += static_cast<signed char>(s->key());
    avg/=musicAndLyrics.size();
    if (avg < 30) performAction(std::make_unique<TransposeSong>(60));

    emit updated(); //Trigger all actions after creation
}

bool Song::performAction(std::unique_ptr<Action> action) {
    if (!action->perform(this)) return false;
    undoneActions_.clear(); //no redo after new action!
    performedActions_.push_back(std::move(action));
    emit updated();
    return true;
}

QStringList Song::undoneActions() const {
    QStringList res;
    for (auto& pa : undoneActions_) {
        res.push_back(pa->description());
    }
    return res;
}

QStringList Song::performedActions() const {
    QStringList res;
    for (auto& pa : performedActions_) {
        res.push_front(pa->description());
    }
    return res;
}

bool Song::undo(unsigned int num) {
    Q_ASSERT(num <= performedActions_.size());
    for (unsigned i = 0; i < num; i++) {
        std::unique_ptr<Action> a = std::move(performedActions_.back());
        if (a->undo(this)) {
            performedActions_.pop_back();
            undoneActions_.push_back(std::move(a));
        } else {
            performedActions_.push_back(std::move(a));
            qDebug() << "UNDO Failed!";
            emit updated();
            return false;
        }
    }
    emit updated();
    return true;
}

bool Song::redo(unsigned int num) {
    Q_ASSERT(num <= undoneActions_.size());
    for (unsigned i = 0; i < num; i++) {
        std::unique_ptr<Action> a = std::move(undoneActions_.back());
        if (a->perform(this)) {
            undoneActions_.pop_back();
            performedActions_.push_back(std::move(a));
        } else {
            undoneActions_.push_back(std::move(a));
            qDebug() << "REDO Failed!";
            emit updated();
            return false;
        }
    }
    emit updated();
    return true;
}

bool Song::relativeSource() const {
    return tags.contains("RELATIVE") && tags["RELATIVE"].toUpper().trimmed() == "YES";
}

bool Song::setFile(QFileInfo &info,const QString& path) {
    info.setFile(_txt.dir(),path);
    if (!info.exists()) _warnings << QString("File %1 not found!").arg(path);
    return info.exists();
}

bool Song::toDouble(const QString &value, double& target) {
    bool ok;
    target = QString(value).replace(",",".").toDouble(&ok);
    if (!ok) _errors << QString("%2 is not a double!").arg(value);
    return ok;
}

//This may fail when moving in Filesystem atm!
bool Song::setTag(const QString &tag, const QString &value) {
    if (value.trimmed().isEmpty()) {
        _warnings << QString("Tag (\"%1\") without value! Skipping.").arg(tag);
        return false;
    }
    if (tag == "MP3" && !setFile(_mp3,value)) return false;
    if (tag == "COVER" && !setFile(_cov,value)) return false;
    if (tag == "BACKGROUND" && !setFile(_bg,value))  return false;
    if (tag == "VIDEO" && !setFile(_vid,value)) return false;
    if (tag == "BPM" && !toDouble(value,_bpm)) return false;
    if (tag == "GAP" && !toDouble(value,_gap)) return false;
    tags[tag] = value;

    return true;
}

void Song::updateRawLyrics() {
    _rawTextCache.clear();
    for (const Sylabel* s : musicAndLyrics) {
        if (s->isLineBreak()) {
            _rawTextCache.append("\n");
        } else {
            _rawTextCache.append(s->text());
        }
    }
}

QString Song::rawLyrics() const  {
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
            continue;
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
    if (!_seenTags.contains(_tag)) _seenTags.append(_tag);
    if (tags.contains(_tag)) return false;
    return setTag(_tag,value);
}
/*
bool Song::updateTag(const QString &tag, const QString &value) {
    QString _tag = tag.toUpper();
    if (!tags.contains(_tag)) {
        qWarning() << "The tag" << tag << "does not exist in" << _txt.canonicalFilePath() <<" and can not be updated. Ignoring!";
        return false;
    }
    return setTag(_tag,value);
}

bool Song::removeTag(const QString &tag) {
    auto _tag = tag.toUpper();
    if (!tags.contains(_tag)) {
        qWarning() << "The tag" << tag << "does not exist in " << _txt.canonicalFilePath() << " and can not be deleted. Ignoring!";
        return false;
    }
    tags.remove(_tag);
    return true;
}*/

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

bool Song::isModified() const {
    return !performedActions_.empty();
}

//bool Song::operator==(const Song& rhs) const {
//    if (rhs.musicAndLyrics.size() != musicAndLyrics.size()) return false;
//    for (int i = 0; i < rhs.musicAndLyrics.size(); i++)
//        if (*(rhs.musicAndLyrics[i]) != *(musicAndLyrics[i])) return false;
//    return rhs.tags == tags;
//}

#include "song.h"
#include <QTextStream>
#include <QDebug>
#include <QPixmap>
#include <QDir>

QStringList Song::seenTags;
QPixmap* Song::_noCover = nullptr;
QPixmap* Song::_coverMissing = nullptr;

Song::Song(const QFileInfo& source) : _txt(source)
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
            QString tag = elements.first();
            QString value = elements.last().trimmed();
            addTag(elements.first(),elements.last());
        } else if (line.startsWith(':')) {
            //Note!
            endOfTags = true;
        } else if (line.startsWith('*')) {
            //Golden Note
            _golden = true;
            endOfTags = true;
        } else if (line.startsWith('F')) {
            //Freestyle
            _freestyle = true;
            endOfTags = true;
        } else if (line.startsWith('-')) {
            //Linebreak (this should not be the first!)
            endOfTags = true;
        } else if (line.startsWith('E')) {
            //End of file
            if (!in.atEnd()) {
//                qWarning() << QString("[%1:%2]: Data beyond end of file!").arg(source.filePath()).arg(lineNo);
                wellFormed = false;
                break;
            }
        } else if (line.startsWith('P')) {
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
	    _players = newPlayer;
        } else {
//            qWarning() << QString("[%1:%2]: Line could not be interpreted!").arg(source.filePath()).arg(lineNo);
            wellFormed = false;
        }
    }
}

//This may fail when moving in Filesystem!
bool Song::setTag(const QString &tag, const QString &value) {
    if (value.trimmed().isEmpty()) {
        qWarning() << QString("[%1]: Tag (\"%2\") without value! Skipping.").arg(_txt.filePath()).arg(tag);
        wellFormed = false;
        return false;
    }
    if (tag == "TITLE") _title = value;
    if (tag == "ARTIST") _artist = value;
    if (tag == "MP3") {
        _mp3.setFile(_txt.dir(),value);
        if (!_mp3.exists()) {
//                    qWarning() << QString("[%1:%2]: MP3 File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
            wellFormed = false;
        }
    }
    if (tag == "COVER") {
        _cov.setFile(_txt.dir(),value);
        if (!_cov.exists()) {
//                    qWarning() << QString("[%1:%2]: COVER File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
            wellFormed = false;
        }
    }
    if (tag == "BACKGROUND") {
        _bg.setFile(_txt.dir(),value);
        if (!_bg.exists()) {
//                    qWarning() << QString("[%1:%2]: BACKGROUND File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
            wellFormed = false;
        }
    }
    if (tag == "VIDEO") {
        _vid.setFile(_txt.dir(),value);
        if (!_vid.exists()) {
//                    qWarning() << QString("[%1:%2]: VIDEO File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
            wellFormed = false;
        }
    }
    //TODO: qWarning() << "Unknown Tag " << tag;
    tags[tag] = value;
    emit updated();
    return true;
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

bool Song::hasBG() const {
    return !tag("BACKGROUND").isNull();
}

bool Song::missingBG() const {
    return hasBG() && !_bg.exists();
}

bool Song::hasVideo() const {
    return !tag("VIDEO").isNull();
}

bool Song::missingVideo() const {
    return hasVideo() && !_vid.exists();
}

bool Song::hasCover() const {
    return !tag("COVER").isNull();
}

bool Song::missingCover() const {
    return hasCover() && !_cov.exists();
}

bool Song::isWellFormed() const {
    return valid && wellFormed;
}

bool Song::isValid() const {
    return valid;
}

const QFileInfo& Song::txt() const {
    return _txt;
}

const QFileInfo& Song::cov() const {
    return _cov;
}

const QFileInfo& Song::mp3() const {
    return _mp3;
}

const QFileInfo& Song::vid() const {
    return _vid;
}

const QFileInfo& Song::bg() const {
    return _bg;
}

QString Song::title() const
{
    if (!valid) return _txt.path();
    return _title;
}
const QString &Song::artist() const {
    return _artist;
}

QString Song::tag(const QString &tag) const {
//    qWarning() << tags;
    if (tags.contains(tag.toUpper()))  return tags[tag.toUpper()];
    return QString::null;
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

//Todo: Whatever needs to be constructed on copy

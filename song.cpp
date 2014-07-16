#include "song.h"
#include <QTextStream>
#include <QDebug>
#include <QPixmap>
#include <QDir>

QStringList Song::seenTags;
QPixmap* Song::noCover = nullptr;

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
//                qWarning() << QString("[%1:%2]: Tags after end of tag section. Discarding Tag!").arg(source.filePath()).arg(lineNo);
                continue;
            }
            line.remove(0,1);
            QStringList elements = line.split(':');
            if (elements.length() != 2) {
                wellFormed = false;
//                qWarning() << QString("[%1:%2]: Malformed Tag! Skipping.").arg(source.filePath()).arg(lineNo);
                continue;
            }
            QString tag = elements.first().toUpper();
            QString value = elements.last().trimmed();
            if (value.trimmed().isEmpty()) {
//                qWarning() << QString("[%1:%2]: Tag without parameter! Skipping.").arg(source.filePath()).arg(lineNo);
                wellFormed = false;
                continue;
            }
            if (!seenTags.contains(tag)) seenTags.append(tag);
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
                _hasCov = true;
                if (!_cov.exists()) {
//                    qWarning() << QString("[%1:%2]: COVER File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
                    wellFormed = false;
                }
            }
            if (tag == "BACKGROUND") {
                _bg.setFile(_txt.dir(),value);
                _hasBg = true;
                if (!_bg.exists()) {
//                    qWarning() << QString("[%1:%2]: BACKGROUND File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
                    wellFormed = false;
                }
            }
            if (tag == "VIDEO") {
                _vid.setFile(_txt.dir(),value);
                _hasVid = true;
                if (!_vid.exists()) {
//                    qWarning() << QString("[%1:%2]: VIDEO File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
                    wellFormed = false;
                }
            }
            //TODO: qWarning() << "Unknown Tag " << tag;
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

bool Song::hasBG() const {
    return _hasBg;
}

bool Song::missingBG() const {
    return hasBG() && !_bg.exists();
}

bool Song::hasVideo() const {
    return _hasVid;
}

bool Song::missingVideo() const {
    return hasVideo() && !_vid.exists();
}

bool Song::hasCover() const {
    return _hasCov;
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

const QFileInfo& Song::mp3() const {
    return _mp3;
}

const QString Song::title() const
{
    if (!valid) return _txt.path();
    return _title;
}
const QString &Song::artist() const { return _artist; }

QPixmap Song::cover() {
    if (_hasCov) {
        if (_covPM.isNull())
            _covPM = QPixmap(_cov.canonicalFilePath()).scaled(96,96,Qt::KeepAspectRatio);
        return _covPM;
    }
    if (noCover == nullptr)
        noCover = new QPixmap(QPixmap(":/images/nocover.png").scaled(96,96,Qt::KeepAspectRatio));
    return *noCover; //TODO: Dummy image!
}

//Todo: Whatever needs to be constructed on copy

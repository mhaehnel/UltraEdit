#include "song.h"
#include <QTextStream>
#include <QDebug>
#include <QDir>

QStringList Song::seenTags;

Song::Song(const QFileInfo& source) : txt(source)
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
            QString tag = elements.first().toUpper();
            QString value = elements.last();
            if (value.trimmed().isEmpty()) {
                qWarning() << QString("[%1:%2]: Tag without parameter! Skipping.").arg(source.filePath()).arg(lineNo);
                wellFormed = false;
                continue;
            }
            if (!seenTags.contains(tag)) seenTags.append(tag);
            if (tag == "TITLE") _title = value.trimmed();
            if (tag == "ARTIST") _artist = value.trimmed();
            if (tag == "MP3") {
                mp3.setFile(txt.dir(),value);
                if (!mp3.isFile()) {
                    qWarning() << QString("[%1:%2]: MP3 File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
                    wellFormed = false;
                }
            }
            if (tag == "COVER") {
                cov.setFile(txt.dir(),value);
                if (!cov.isFile()) {
                    qWarning() << QString("[%1:%2]: COVER File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
                    wellFormed = false;
                }
            }
            if (tag == "BACKGROUND") {
                bg.setFile(txt.dir(),value);
                if (!bg.isFile()) {
                    qWarning() << QString("[%1:%2]: BACKGROUND File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
                    wellFormed = false;
                }
            }
            if (tag == "VIDEO") {
                vid.setFile(txt.dir(),value);
                if (!vid.isFile()) {
                    qWarning() << QString("[%1:%2]: VIDEO File %3 not found!").arg(source.filePath()).arg(lineNo).arg(value);
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
                qWarning() << QString("[%1:%2]: Data beyond end of file!").arg(source.filePath()).arg(lineNo);
                wellFormed = false;
                break;
            }
        } else if (line.startsWith('P')) {
            bool ok;
            int newPlayer = line.trimmed().remove(0,1).toInt(&ok);
            if (!ok) {
                qWarning() << QString("[%1:%2]: Malformed playernumber!!").arg(source.filePath()).arg(lineNo);
                wellFormed = false;
            }
            if (_players+1 != newPlayer) {
                qWarning() << QString("[%1:%2]: Found player number %3 without having a number %4!!").
                              arg(source.filePath()).arg(lineNo).arg(newPlayer).arg(newPlayer-1);
                wellFormed = false;
            }
        } else {
            qWarning() << QString("[%1:%2]: Line could not be interpreted!").arg(source.filePath()).arg(lineNo);
            wellFormed = false;
        }
    }
}

bool Song::hasBG() const {
    return valid && _bg;
}

bool Song::hasVideo() const {
    return valid && _vid;
}

bool Song::hasCover() const {
    return valid && _cov;
}

bool Song::isWellFormed() const {
    return valid && wellFormed;
}

bool Song::isValid() const {
    return valid;
}

QString Song::title() const { return _title; }
QString Song::artist() const { return _artist; }

//Todo: Whatever needs to be constructed on copy

#include "song.h"
#include <QTextStream>
#include <QDebug>
#include <QPixmap>
#include <QDir>
#include "validator.h"
#include <pathinstanceselector.h>
#include <cassert>
#include <QMessageBox>

QStringList Song::seenTags;
QPixmap* Song::_noCover = nullptr;
QPixmap* Song::_coverMissing = nullptr;

bool Song::answeredToAll = false;
bool Song::yesToAll;

Song::Song(const QFileInfo& source, Validator* val, const QString basePath) : _basePath(basePath),  _txt(source), validator(val)
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
    std::sort(musicAndLyrics.begin(),musicAndLyrics.end(),[] (const Sylabel* s1, const Sylabel* s2) {
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

//This may fail when moving in Filesystem atm!
bool Song::setTag(const QString &tag, const QString &value) {
    if (value.trimmed().isEmpty()) {
        qWarning() << QString("[%1]: Tag (\"%2\") without value! Skipping.").arg(_txt.filePath()).arg(tag);
        wellFormed = false;
        return false;
    }
    tags[tag] = value;
//    if (tag == "TITLE") _title = value;
//    if (tag == "ARTIST") _artist = value;
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
    bool ok;
    if (tag == "BPM") {
        _bpm = QString(value).replace(",",".").toDouble(&ok);
        if (!ok) {
            qWarning() << QString("[%1]: BPM is not a double!").arg(_txt.filePath());
            valid =false;
            return false;
        }
    }
    if (tag == "GAP") {
        _gap = QString(value).replace(",",".").toDouble(&ok);
        if (!ok) {
            qWarning() << QString("[%1]: GAP is not a double!").arg(_txt.filePath());
            valid =false;
            return false;
        }
    }
    //TODO: qWarning() << "Unknown Tag " << tag;
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
            if (s->type() == Sylabel::Type::LineBreak) {
                _rawTextCache.append("\n");
            } else {
                _rawTextCache.append(s->text());
            }
        }
    }
    return _rawTextCache;
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

QList<Sylabel*>& Song::sylabels() {
    return musicAndLyrics;
}

QString Song::title() const
{
    if (!valid) return _txt.path();
    return tags["TITLE"];
}
QString Song::artist() const {
    return tags["ARTIST"];
}

QString Song::basePath() const {
    return _basePath;
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

void Song::playing(int ms) {
    //ms to from / to
    int line = 0;
    int lineStart = 0;
    static int last_line = -1;
    int from = 0;
    ms -= _gap;
    if (ms/1000.0 < musicAndLyrics.first()->time()) return;
    for (int i = 0; i < musicAndLyrics.size(); i++) {
        Sylabel* s = musicAndLyrics[i];
        if (s->type() == Sylabel::Type::LineBreak) {
            line++;
            lineStart = i+1;
            from++;
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

//Todo: Whatever needs to be constructed on copy

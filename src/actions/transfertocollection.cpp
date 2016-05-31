#include "actions/transfertocollection.h"
#include <actions/action.h>

Song::TransferToCollection::TransferToCollection(Collection *target, Type t)
    : newCollection(target), t_(t)
{
    Q_ASSERT(newCollection != nullptr);
}

//TODO: The error handling of this whole function is very shaky

bool Song::TransferToCollection::perform(Song &song) {
    QFileInfo newTXT = newCollection->path(&song,"txt");
    QFileInfo newMP3 = newCollection->path(&song,"mp3");
    QFileInfo newVID = newCollection->path(&song,song.vid().suffix());
    QFileInfo newBGR = newCollection->path(&song,QString("[BG].")+song.bg().suffix());
    QFileInfo newCOV = newCollection->path(&song,QString("[CO].")+song.cov().suffix());

    //Check if this would overwrite anything. This should not happen
    if ((song.txt() != newTXT && newTXT.exists())
            || (song.mp3() != newMP3 && newMP3.exists())
            || (song.hasVideo() && song.vid() != newVID && newVID.exists())
            || (song.hasCover() && song.cov() != newCOV && newCOV.exists())
            || (song.hasBG() && song.bg() != newBGR && newBGR.exists())
            )
        return false;
    //Check if everything we want to transfer really exists
    if (!song.txt().exists() || !song.mp3().exists()
            || (song.hasVideo() && !song.vid().exists())
            || (song.hasCover() && !song.cov().exists())
            || (song.hasBG() && !song.bg().exists())) return false;

    QDir d(newTXT.absolutePath());
    QString origTxt = song.txt().absoluteFilePath();
    song._txt = newTXT;
    auto transfer = (t_ == Type::Copy)?
                        static_cast<bool(*)(const QString&, const QString&)>(&QFile::copy):
                        static_cast<bool(*)(const QString&, const QString&)>(&QFile::rename);
    d.mkpath(newMP3.absolutePath());
    if (!transfer(song.mp3().absoluteFilePath(),newMP3.absoluteFilePath())) {
        qDebug() << song.mp3().absoluteFilePath() << "=>" << newMP3.absoluteFilePath() << "failed";
    } else {
        song.setTag("MP3",d.relativeFilePath(newMP3.absoluteFilePath()));
    }
    if (song.vid().exists()) {
        d.mkpath(newVID.absolutePath());
        if (!transfer(song.vid().absoluteFilePath(),newVID.absoluteFilePath())) {
            qDebug() << song.vid().absoluteFilePath() << "=>" << newVID.absoluteFilePath() << "failed";
        } else {
            song.setTag("VIDEO",d.relativeFilePath(newVID.absoluteFilePath()));
        }
    }
    if (song.cov().exists()) {
        d.mkpath(newCOV.absolutePath());
        qDebug() << song.cov().absoluteFilePath() << "=>" << newCOV.absoluteFilePath();
        if (!transfer(song.cov().absoluteFilePath(),newCOV.absoluteFilePath())) {
            qDebug() << song.cov().absoluteFilePath() << "=>" << newCOV.absoluteFilePath() << "failed";
        } else {
            qDebug() << "Setting Tag" << d.relativeFilePath(newCOV.absoluteFilePath());
            song.setTag("COVER",d.relativeFilePath(newCOV.absoluteFilePath()));
        }
    }
    if (song.bg().exists()) {
        d.mkpath(newBGR.absolutePath());
        if (!transfer(song.bg().absoluteFilePath(),newBGR.absoluteFilePath())) {;
            qDebug() << song.bg().absoluteFilePath() << "=>" << newBGR.absoluteFilePath() << "failed";
        } else {
            song.setTag("BACKGROUND",d.relativeFilePath(newBGR.absoluteFilePath()));
        }
    }
    //Write out txzt ...
    d.mkpath(newTXT.absolutePath());
    if (!song.saveSong()) return false;
    if (t_ ==Type::Move) {
        if (!QFile(origTxt).remove()) {
            qDebug() << "Removing source txt file failed. Non fatal!";
        } else {
            QFileInfo fi(QFileInfo(origTxt).dir().absolutePath());
            fi.dir().rmpath(fi.fileName());
        }
    }
    //TODO: remove parent dir on move?. Save it? What if multiple parent dirs?
    oldCollection = song.collection_;
    if (oldCollection != nullptr) oldCollection->removeSong(&song);
    song.collection_ = newCollection;
    newCollection->addSong(&song);
    return true;
}

bool Song::TransferToCollection::undo(Song &) {
    if (oldCollection == nullptr) {
        qDebug() << "Can not undo import!";
        return false;
    }
    return false;
}

QString Song::TransferToCollection::shortName() const {
    return QString((t_ == Type::Copy)?"Copy":"Move")+" to collection";
}

QString Song::TransferToCollection::description() const {
    if (oldCollection != nullptr)
        return QString((t_ == Type::Copy)?"Copy":"Move")+" from collection '" +
                oldCollection->name()+"'' to '"+newCollection->name()+"'";
    return QString((t_ == Type::Copy)?"Copy":"Move")+"-Import to collection '" +
            newCollection->name()+"'";

}

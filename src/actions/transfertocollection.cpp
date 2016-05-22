#include "actions/transfertocollection.h"
#include <actions/action.h>

Song::TransferToCollection::TransferToCollection(const Collection *target, Type t)
    : newCollection(target), t_(t)
{}

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

    bool ret = true;
    QDir d("/");
    auto transfer = (t_ == Type::Copy)?
                        static_cast<bool(*)(const QString&, const QString&)>(&QFile::copy):
                        static_cast<bool(*)(const QString&, const QString&)>(&QFile::rename);
    d.mkpath(newMP3.absolutePath());
    qDebug() << song.mp3().absoluteFilePath() << "=>" << newMP3.absoluteFilePath();
    ret &= transfer(song.mp3().absoluteFilePath(),newMP3.absoluteFilePath());
    d.mkpath(newTXT.absolutePath());
    qDebug() << song.txt().absoluteFilePath() << "=>" << newTXT.absoluteFilePath();
    ret &= transfer(song.txt().absoluteFilePath(),newTXT.absoluteFilePath());
    if (song.hasVideo()) {
        d.mkpath(newVID.absolutePath());
        qDebug() << song.vid().absoluteFilePath() << "=>" << newVID.absoluteFilePath();
        ret &= transfer(song.vid().absoluteFilePath(),newVID.absoluteFilePath());
    }
    if (song.hasCover()) {
        d.mkpath(newCOV.absolutePath());
        qDebug() << song.cov().absoluteFilePath() << "=>" << newCOV.absoluteFilePath();
        ret &= transfer(song.cov().absoluteFilePath(),newCOV.absoluteFilePath());
    }
    if (song.hasBG()) {
        d.mkpath(newBGR.absolutePath());
        qDebug() << song.bg().absoluteFilePath() << "=>" << newBGR.absoluteFilePath();
        ret &= transfer(song.bg().absoluteFilePath(),newBGR.absoluteFilePath());
    }
    //TODO: remove parent dir on move?. Save it? What if multiple parent dirs?
    //Can we handle this properly?
    if (!ret) {
        qDebug() << "This should not have happened! Import failed!";
    } else {
        oldCollection = song.collection_;
        song.collection_ = newCollection;
    }
    return ret;
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

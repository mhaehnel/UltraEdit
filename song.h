#ifndef SONG_H
#define SONG_H

#include <QFileInfo>
#include <QMap>

class Song : public QObject
{

Q_OBJECT
private:
    bool wellFormed = true;
    bool valid = true;
    bool _bg = false;
    bool _cov = false;
    bool _vid = false;
    bool _golden = false;
    bool _freestyle = false;
    int _players = 0; //stays 0 for simple file
    QString _title, _artist;
    QFileInfo txt,mp3,vid,cov,bg;
    QMap<QString,QString> tags;
    static QStringList seenTags;

public:
    Song(const QFileInfo& source);
    Song(const Song&) = delete;
    Song operator=(const Song&) = delete;
    bool isValid() const;
    bool hasVideo() const;
    bool hasBG() const;
    bool hasCover() const;
    bool hasGoldenNotes() const;
    bool hasFreestyle() const;
    bool isWellFormed() const;
    bool isMultiplayer() const;
    int players() const;
    QString artist() const;
    QString title() const;
    QPixmap cover() const;
    QPixmap background() const;
};

#endif // SONG_H

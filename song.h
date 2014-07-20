#ifndef SONG_H
#define SONG_H

#include <QFileInfo>
#include <QMap>
#include <QPixmap>

class Validator;

class Song : public QObject
{

Q_OBJECT
private:
    bool wellFormed = true;
    bool initialized = false;
    bool valid = true;
    bool _golden = false;
    bool _freestyle = false;
    int _players = 0; //stays 0 for simple file
    QString _title, _artist, _basePath;
    QFileInfo _txt,_mp3,_vid,_cov,_bg;
    QPixmap _covPM;
    QMap<QString,QString> tags;
    static QStringList seenTags;
    static QPixmap *_noCover, *_coverMissing;
    bool setTag(const QString& tag, const QString& value);
    Validator* validator;
public:
    Song(const QFileInfo& source, Validator* val, const QString basePath);
    Song(const Song&) = delete;
    Song operator=(const Song&) = delete;
    bool addTag(const QString& tag, const QString& value);
    bool updateTag(const QString& tag, const QString& value);
    bool isValid() const;
    bool hasVideo() const;
    bool missingVideo() const;
    bool hasBG() const;
    bool missingBG() const;
    bool hasCover() const;
    bool missingCover() const;
    bool hasGoldenNotes() const;
    bool hasFreestyle() const;
    bool isWellFormed() const;
    bool isMultiplayer() const;
    int players() const;
    const QFileInfo& txt() const;
    const QFileInfo& mp3() const;
    const QFileInfo& vid() const;
    const QFileInfo& cov() const;
    const QFileInfo& bg() const;
    const QString& artist() const;
    QString title() const;
    QString basePath() const;
    QString tag(const QString& tag) const;
    QPixmap cover();
    QPixmap background() const;

signals:
    void updated();
};

Q_DECLARE_METATYPE(Song*);

#endif // SONG_H

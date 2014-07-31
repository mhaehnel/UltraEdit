#ifndef SONG_H
#define SONG_H

#include <QFileInfo>
#include <sylabel.h>
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
    double _bpm = 0;
    double _gap = 0;
    QString _basePath, _rawTextCache;
    QFileInfo _txt,_mp3,_vid,_cov,_bg;
    QPixmap _covPM;
    QMap<QString,QString> tags;
    QMap<QString,int> components;
    QMap<QString,int> agreedComponentCount;
    //Player -> (time -> text)
    QList<Sylabel> musicAndLyrics;
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
    QString rawLyrics();
    QString artist() const;
    QString title() const;
    QString basePath() const;
    double bpm() const { return _bpm; }
    double gap() const { return _gap; }
    QString tag(const QString& tag) const;
    QPixmap cover();
    QPixmap background() const;
    QList<Sylabel>& sylabels();

signals:
    void updated();
    void playingSylabel(int from, int to);
    void playingSylabel(const Sylabel& s);
    void lineChanged(int line); //, QList<Sylabel> notes);

public slots:
    void playing(int ms);
};

Q_DECLARE_METATYPE(Song*);

#endif // SONG_H

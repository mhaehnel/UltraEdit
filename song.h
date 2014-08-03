#ifndef SONG_H
#define SONG_H

#include <QFileInfo>
#include <QMap>
#include <QPixmap>
#include <sylabel.h>

class Validator;

class Song : public QObject
{

Q_OBJECT
private:
    bool setFile(QFileInfo& info, const QString& path);
    bool setTag(const QString& tag, const QString& value);
    bool toDouble(const QString& value, double& target);

    static bool yesToAll, answeredToAll;

    bool wellFormed = true;   //has no minor errors
    bool initialized = false; //has been initialized. Internal state!
    bool valid = true;        //has no major errors
    bool _golden = false;     //has golden notes
    bool _freestyle = false;  //has freestyle notes
    int _players = 0;         //stays 0 for simple file (TODO! This is not really implemented ATM!)
    double _bpm, _gap;
    QString _basePath, _rawTextCache;
    QFileInfo _txt,_mp3,_vid,_cov,_bg;
    QPixmap _covPM;
    QMap<QString,QString> tags;
    QMap<QString,int> components;
    QMap<QString,int> agreedComponentCount;
    QList<Sylabel*> musicAndLyrics; //Todo: This is not multiplayer capable at the moment!
    static QStringList seenTags; //Static overview of all tags seen in all files
    static QPixmap *_noCover, *_coverMissing; //the dummy covers
    Validator* validator;
public:
    Song(const QFileInfo& source, Validator* val, const QString basePath);
    Song(const Song&) = delete;
    Song operator=(const Song&) = delete;

    bool addTag(const QString &tag, const QString& value);
    bool hasTag(const QString &tag) const;
    QString tag(const QString &tag) const;
    bool updateTag(const QString& tag, const QString& value);

    inline const QFileInfo& txt() const { return _txt; }
    inline const QFileInfo& mp3() const { return _mp3; }
    inline const QFileInfo& vid() const { return _vid; }
    inline const QFileInfo& cov() const { return _cov; }
    inline const QFileInfo& bg() const  { return _bg;  }

    inline bool hasVideo() const       { return hasTag("VIDEO"); }
    inline bool hasCover() const       { return hasTag("COVER"); }
    inline bool hasBG() const          { return hasTag("BACKGROUND"); }
    inline bool hasGoldenNotes() const { return _golden; }
    inline bool hasFreestyle() const   { return _freestyle; }
    inline bool isValid() const        { return valid; }
    inline bool isWellFormed() const   { return valid && wellFormed; }

    inline double bpm() const { return _bpm; }
    inline double gap() const { return _gap; }

    bool missingVideo() const;
    bool missingBG() const;
    bool missingCover() const;
    bool isMultiplayer() const;
    int players() const;

    inline QString basePath() const { return _basePath; }
    inline QList<Sylabel*>& sylabels() { return musicAndLyrics; }

    QString rawLyrics();
    QString artist() const;
    QString title() const;
    QPixmap cover();
    QPixmap background() const;

signals:
    void updated();
    void playingSylabel(int from, int to);
    void playingSylabel(Sylabel* s);
    void lineChanged(int line);

public slots:
    void playing(int ms);
};

#endif // SONG_H

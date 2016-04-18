#pragma once
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
    void adjustRelative(Sylabel* syl);
    void updateDataCache();

    static bool yesToAll, answeredToAll;

    //bool wellFormed = true;   //has no minor errors
    bool initialized = false; //has been initialized. Internal state!
    //bool valid = true;        //has no major errors that prevent playback (only ignorable ones)
    bool _golden = false;     //has golden notes
    bool _freestyle = false;  //has freestyle notes
    int _players = 0;         //stays 0 for simple file (TODO! This is not really implemented ATM!)
    double _bpm, _gap;
    QString _basePath, _rawTextCache, _rawDataCache;
    QFileInfo _txt,_mp3,_vid,_cov,_bg;
    QPixmap _covPM;
    QMap<QString,QString> tags;
    QMap<QString,int> components;
    QStringList _errors, _warnings, _fatals;
    QMap<QString,int> agreedComponentCount;
    QList<Sylabel*> musicAndLyrics; //Todo: This is not multiplayer capable at the moment!
    static QStringList _seenTags; //Static overview of all tags seen in all files
    static QPixmap *_noCover, *_coverMissing; //the dummy covers
    Validator* validator;
    //No transform is used for internal comparison of files without checking for empty lines and so on
    Song(const QFileInfo& source, Validator* val, const QString basePath, bool noTransform);
public:
    Song(const QFileInfo& source, Validator* val, const QString basePath);
    Song(const Song&) = delete;
    Song operator=(const Song&) = delete;
    bool operator==(const Song& rhs) const;

    bool addTag(const QString &tag, const QString& value);
    bool hasTag(const QString &tag) const;
    QString tag(const QString &tag) const;
    bool updateTag(const QString& tag, const QString& value);

    inline const QFileInfo& txt() const { return _txt; }
    inline const QFileInfo& mp3() const { return _mp3; }
    inline const QFileInfo& vid() const { return _vid; }
    inline const QFileInfo& cov() const { return _cov; }
    inline const QFileInfo& bg() const  { return _bg;  }
    inline const QStringList& errors() const { return _errors; }
    inline const QStringList& warnings() const { return _warnings; }
    inline const QStringList& fatals() const { return _fatals; }

    inline bool hasVideo() const       { return hasTag("VIDEO"); }
    inline bool hasCover() const       { return hasTag("COVER"); }
    inline bool hasBG() const          { return hasTag("BACKGROUND"); }
    inline bool hasGoldenNotes() const { return _golden; }
    inline bool hasFreestyle() const   { return _freestyle; }
    inline bool isValid() const        { return _fatals.size() == 0; }
    inline bool isWellFormed() const   { return isValid() && _warnings.size() == 0 && _errors.size() == 0; }

    inline double bpm() const { return _bpm; }
    inline double gap() const { return _gap; }

    bool missingVideo() const;
    bool missingBG() const;
    bool missingCover() const;
    bool isModified() const;
    bool isMultiplayer() const;
    bool relativeSource() const;
    int players() const;

    inline QString basePath() const { return _basePath; }
    static inline const QStringList& seenTags() { return _seenTags; }
    inline QList<Sylabel*>& sylabels() { return musicAndLyrics; }

    QString rawLyrics();
    QString rawData(); //will be const once updateDataCache is done automatically
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

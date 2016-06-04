#pragma once
#include <memory>
#include <experimental/optional>
#include <QFileInfo>
#include <QMap>
#include <QPixmap>
#include <sylabel.h>
#include <actions/action.h>
#include <collection.h>

class Song : public QObject
{
Q_OBJECT
    friend class SongImportDialog; //Hacky ...
private:
    bool setFile(QFileInfo& info, const QString& path);
    bool setTag(const QString& tag, const QString& value);
    bool toDouble(const QString& value, double& target);
    bool toInt(const QString& value, qint64& target);
    void adjustRelative(Sylabel* syl);
    bool checkAndRename(QFileInfo& file, QDir txtDir, const Collection* col, QString suffix, QString tag);
    void updateDataCache();

    std::list<std::unique_ptr<Action>> performedActions_;
    std::list<std::unique_ptr<Action>> undoneActions_;
    std::list<std::shared_ptr<ActionItem>> actionItems_;

    //bool wellFormed = true;   //has no minor errors
    //bool initialized = false; //has been initialized. Internal state!
    //bool valid = true;        //has no major errors that prevent playback (only ignorable ones)
    bool _golden = false;     //has golden notes
    bool _freestyle = false;  //has freestyle notes
    std::experimental::optional<size_t> actionLevelSaved = 0;
    int _players = 0;         //stays 0 for simple file (TODO! This is not really implemented ATM!)
    double _bpm;
    qint64 _gap, _videoGap, _bpmFactor;
    QString _basePath, _rawTextCache, _rawDataCache;
    QFileInfo _txt,_mp3,_vid,_cov,_bg;
    QPixmap _covPM;
    QMap<QString,QString> tags;
    QMap<QString,int> components;
    QStringList _errors, _warnings; //, _fatals;
    QMap<QString,int> agreedComponentCount;
    QList<Sylabel*> musicAndLyrics; //Todo: This is not multiplayer capable at the moment!
    std::vector<int> _linesIdx;
    static QStringList _seenTags; //Static overview of all tags seen in all files
    static QPixmap *_noCover, *_coverMissing; //the dummy covers
    Collection* collection_;
/*    bool removeTag(const QString &tag);
    bool updateTag(const QString& tag, const QString& value);
*/
    bool addTag(const QString &tag, const QString& value);
private slots:
    //Cache updates
    void updateRawLyrics();


public:
    //TheActions
    class ConvertRelative;
    class TransposeSong;
    class ModifyTag;
    class OrderLyrics;
    class ModifyGap;
    class TransferToCollection;

    Song(const QFileInfo& source, Collection* col = nullptr);
    virtual ~Song();

    QStringList performedActions() const;
    QStringList undoneActions() const;
    const std::list<std::shared_ptr<ActionItem>>& actionItems() const;

    //These 3 *must* be the only functions allowed to modify Song
    //This enforces undoability and usage of the action scheme
    bool undo(unsigned num);
    bool redo(unsigned num);
    bool performAction(std::unique_ptr<Action> action);

    bool canUndo() const;

    bool hasTag(const QString &tag) const;
    QString tag(const QString &tag) const;

    const QFileInfo& txt() const { return _txt; }
    const QFileInfo& mp3() const { return _mp3; }
    const QFileInfo& vid() const { return _vid; }
    const QFileInfo& cov() const { return _cov; }
    const QFileInfo& bg() const  { return _bg;  }
    const QStringList& errors() const { return _errors; }
    const QStringList& warnings() const { return _warnings; }

    bool hasVideo() const       { return hasTag("VIDEO"); }
    bool hasCover() const       { return hasTag("COVER"); }
    bool hasBG() const          { return hasTag("BACKGROUND"); }
    bool hasGoldenNotes() const { return _golden; }
    bool hasFreestyle() const   { return _freestyle; }
    bool isWellFormed() const   { return _warnings.isEmpty() && _errors.isEmpty(); }

    double bpm() const { return _bpm; }
    qint64 bpmFactor() const { return _bpmFactor; }
    qint64 gap() const { return _gap; }
    qint64 videoGap() const { return _videoGap; }

    bool missingVideo() const;
    bool missingBG() const;
    bool missingCover() const;
    bool isModified() const;
    bool isMultiplayer() const;
    bool relativeSource() const;
    int players() const;
    int lines() const;
    quint64 timeAtLine(int line) const;

    static const QStringList& seenTags() { return _seenTags; }
    const QList<Sylabel*>& sylabels() const { return musicAndLyrics; }

    //These will be const later once actions are better implemented ...
    QString rawLyrics() const;
    QString rawData(); //will be const once updateDataCache is done automatically
    QString artist() const;
    QString title() const;
    QPixmap cover();
    QPixmap background() const;

signals:
    void updated() const;
    void playingSylabel(int from, int to);
    void playingSylabel(Sylabel* s);
    void lineChanged(int line);

public slots:
    void playing(int ms);
    bool saveSong();
};

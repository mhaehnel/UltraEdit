#ifndef COLLECTIONSCANNER_H
#define COLLECTIONSCANNER_H

#include <QStringList>
#include <QObject>
#include "song.h"

class CollectionScanner : public QObject
{
    Q_OBJECT
public:
    explicit CollectionScanner(QObject *parent = 0);
    bool scanInProgress;

signals:
    void scanFinished();
    void scanStarted();
    void foundSong(Song* song);

public slots:
    void scanCollection(const QStringList &paths);
};

#endif // COLLECTIONSCANNER_H

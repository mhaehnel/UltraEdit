#include "collectionscanner.h"
#include <QDebug>
#include <QApplication>
#include <QDirIterator>

CollectionScanner::CollectionScanner(QObject *parent) :
    QObject(parent), scanInProgress(false)
{}

void CollectionScanner::scanCollection(const QStringList& paths) {
    emit scanStarted();
    qDebug() << "Go ahead :)";
    for (QString d : paths) {
        qDebug() << "Scannning:"  << d;
        QDirIterator di(d,QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (di.hasNext()) {
            QFileInfo fi(di.next());
            if (fi.suffix().compare("txt",Qt::CaseInsensitive)) continue;
            emit foundSong(new Song(fi));
        };
    }
    emit scanFinished();
}

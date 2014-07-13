#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <collectionscanner.h>
#include "song.h"
#include "songframe.h"
#include <QThread>
#include <QProgressBar>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    bool filter(const Song* song);
    ~MainWindow();

signals:
    void rescanCollection(const QStringList& paths);

private slots:
    void on_actionSources_triggered();
    void addSong(Song* song);
    void refreshList();
    void regroupList();

private:
    QString getGroup(Song* song);
    void sortList(QList<SongFrame*> &lst);

    int notWellFormedCount, invalidCount;
    QList<Song*> songList;
    QList<SongFrame*> songFrames;
    QMap<QString,QList<SongFrame*>> groupedFrames;
    Ui::MainWindow *ui;
    QSettings config;
    QThread scanThread;
    CollectionScanner scanner;
    QProgressBar statusProgress;
};

#endif // MAINWINDOW_H

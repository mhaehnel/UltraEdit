#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <collectionscanner.h>
#include "song.h"
#include "songframe.h"
#include "songgroup.h"
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
    void selectionChanged();

private slots:
    void on_actionSources_triggered();
    void addSong(Song* song);
    //void refreshList();
    void regroupList();
    void resortList();
    void filterList();
    void selectFrame(SongFrame* sf);

    void on_actionChecker_Settings_triggered();

private:
    QString getGroup(Song* song);
    void sortFrames(QList<SongFrame*> &sf);

    int notWellFormedCount, invalidCount;
    QList<Song*> songList;
    QList<SongFrame*> selectedFrames;
    QList<SongFrame*> songFrames;
    QMap<QString,QList<SongFrame*>> groupedFrames;
    QMap<QString,SongGroup*> songGroups;
    Ui::MainWindow *ui;
    QSettings config;
    QThread scanThread;
    CollectionScanner scanner;
    QProgressBar statusProgress;
};

#endif // MAINWINDOW_H

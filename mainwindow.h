#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
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
    ~MainWindow();

signals:
    void selectionChanged();

private slots:
    void addSong(Song* song);

    void filterList();
    void regroupList();
    void rescanCollection();
    void resortList();
    void selectFrame(SongFrame* sf);

    void on_actionChecker_Settings_triggered();
    void on_actionSources_triggered();

private:
    bool filter(const Song* song);
    QString getGroup(Song* song);
    void sortFrames(QList<SongFrame*> &sf);

    int notWellFormedCount, invalidCount;
    QList<Song*> songList;
    QList<SongFrame*> selectedFrames, songFrames;
    QMap<QString,QList<SongFrame*>> groupedFrames;
    QMap<QString,SongGroup*> songGroups;
    Ui::MainWindow *ui;
    QSettings config;
    QProgressBar statusProgress;
};

#endif // MAINWINDOW_H

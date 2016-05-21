#pragma once
#include <QMainWindow>
#include <QSettings>
#include "song.h"
#include "songframe.h"
#include "songgroup.h"
#include <QThread>
#include <QProgressBar>
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

signals:
    void selectionChanged();

private slots:
    void addSong(Song* song);

    void filterList();
    void regroupList();
    void rescanCollection();
    void resortList();
    void selectFrame(SongFrame* sf);
    void popOut(int idx);

    void on_actionSources_triggered();
    void on_actionMidi_Output_triggered();
    void on_actionImport_Directory_triggered();
    void on_actionImport_Song_triggered();

private:
    bool filter(const Song* song);
    void playWelcome();
    QString getGroup(Song* song);
    void sortFrames(QList<SongFrame*> &sf);

    int notWellFormedCount, invalidCount;
    QList<Song*> songList;
    QList<SongFrame*> selectedFrames, songFrames;
    QMap<QString,QList<SongFrame*>> groupedFrames;
    QMap<QString,SongGroup*> songGroups;
    std::unique_ptr<Ui::MainWindow> ui;
    QSettings config;
    QProgressBar statusProgress;
    QByteArray splitterState;
};

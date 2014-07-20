#include <QDebug>
#include <QSemaphore>
#include "mainwindow.h"
#include "selectsongdirs.h"
#include "ui_mainwindow.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QMutex>
#include <atomic>
#include <cassert>
#include <functional>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), notWellFormedCount(0), invalidCount(0),
    ui(new Ui::MainWindow),
  config("MH-Development","UltraEdit")
{
    //Initialize
    ui->setupUi(this);
    qRegisterMetaType<Song*>();
    ui->hasVideo->setCheckState(Qt::PartiallyChecked);
    ui->hasBackground->setCheckState(Qt::PartiallyChecked);
    ui->hasUnknownTags->setCheckState(Qt::PartiallyChecked);
    ui->missingBackgroundFile->setCheckState(Qt::PartiallyChecked);
    ui->missingCoverFile->setCheckState(Qt::PartiallyChecked);
    ui->missingMandatoryTags->setCheckState(Qt::PartiallyChecked);
    ui->missingMP3File->setCheckState(Qt::Checked);
    ui->missingSongtextFile->setCheckState(Qt::PartiallyChecked);
    ui->missingVideoFile->setCheckState(Qt::PartiallyChecked);
    ui->startsAt0->setCheckState(Qt::PartiallyChecked);
    ui->wellFormed->setCheckState(Qt::PartiallyChecked);
    ui->isValid->setCheckState(Qt::PartiallyChecked);
    ui->songDetails->setSelection(&selectedFrames);

    scanner.moveToThread(&scanThread);
    ui->songList->setLayout(new QBoxLayout(QBoxLayout::Down));
    statusBar()->addPermanentWidget(&statusProgress);

    connect(this,&MainWindow::selectionChanged,ui->songDetails,&SongInfo::selectionUpdated);
    connect(this,&MainWindow::rescanCollection,&scanner,&CollectionScanner::scanCollection);
    connect(&scanner,&CollectionScanner::foundSong,this,&MainWindow::addSong);
    connect(&scanner,&CollectionScanner::scanFinished, [this] {
        ((QBoxLayout*)ui->songList->layout())->addStretch(1);
        QMetaObject::invokeMethod(this,"regroupList");
        selectedFrames.clear();
        emit selectionChanged();
    });
    connect(&scanner,&CollectionScanner::scanStarted,[this] {
       statusProgress.setRange(0,0);
       statusProgress.setVisible(true);
       statusBar()->showMessage("Scanning ... 0 Found");
    });
    emit rescanCollection(config.value("songdirs").toStringList());
    scanThread.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool filterState(const QCheckBox* cb, std::function<bool(void)> func) {
    switch (cb->checkState()) {
        case Qt::PartiallyChecked:
            return true;
        case Qt::Checked:
            return func();
        case Qt::Unchecked:
            return !func();
    }
}

bool MainWindow::filter(const Song *song) {
    if (!filterState(ui->missingBackgroundFile,std::bind(&Song::missingBG,song))) return false;
    if (!filterState(ui->missingCoverFile,std::bind(&Song::missingCover,song))) return false;
    if (!filterState(ui->missingVideoFile,std::bind(&Song::missingVideo,song))) return false;
    if (!filterState(ui->hasBackground,std::bind(&Song::hasBG,song))) return false;
    if (!filterState(ui->hasVideo,std::bind(&Song::hasVideo,song))) return false;
    if (!filterState(ui->wellFormed,std::bind(&Song::isWellFormed,song))) return false;
    if (!filterState(ui->isValid,std::bind(&Song::isValid,song))) return false;
    if (!filterState(ui->missingMP3File,[&song] { return !song->mp3().exists(); })) return false;

    if (!ui->titleFilter->text().isEmpty() && !song->title().contains(ui->titleFilter->text(),Qt::CaseInsensitive)) return false;
    if (!ui->artistFilter->text().isEmpty() && !song->artist().contains(ui->artistFilter->text(),Qt::CaseInsensitive)) return false;
    return true; //In!
    //TODO: freestyle / goldennote / players
}

QString MainWindow::getGroup(Song *song) {
    if (!ui->groupList->isChecked()) return "*";
    //we need to group
    if (ui->groupArtist->isChecked()) return song->artist();
}

//This does currently not support rapid regroups! Lock it to make it work!
void MainWindow::regroupList() {
    qDebug() << "regroupList";
    groupedFrames.clear();  //Frames are only once instanciated. Do not delete!
    for (SongGroup* sg : songGroups) delete sg; //We govern these. Must be deleted!
    songGroups.clear();

    for (SongFrame* sf : songFrames) {
        QString group = getGroup(sf->song());
        groupedFrames[group].append(sf);
        songGroups[group] = new SongGroup(group);
        songGroups[group]->hide();
    }
    for (SongGroup* sg : songGroups) ui->songList->layout()->addWidget(sg);
    //Sort in group
    qDebug() << "Starting sort ..";
    resortList();
    qDebug() << "Starting filter ...";
    //hide filtered frames / empty groups
    filterList();
    qDebug() << "Finished!";
}

//This is expensive since it has to readd all layouts! For all groups ... kinde sucks ... so ... regroup is costly!
void MainWindow::resortList() {
    qDebug() << "resortList";
    QList<QString> keys = groupedFrames.keys();
    QFuture<void> fut = QtConcurrent::map(keys,[this] (QString const& k) { sortFrames(groupedFrames[k]); });
    fut.waitForFinished();

    //add frames to group widgets (in sort order)
    statusProgress.setRange(0,songGroups.keys().count());
    int sp = 0;
    for (QString g : songGroups.keys()) {
        QLayoutItem* li = nullptr;
        bool old = songGroups[g]->isVisible();
        songGroups[g]->setVisible(false);
        while ((li = songGroups[g]->layout().takeAt(0)) != nullptr) {
            songGroups[g]->layout().removeItem(li);
        }
        for (SongFrame* f : groupedFrames[g]) {
            qApp->processEvents();
            songGroups[g]->layout().addWidget(f);
        }
        songGroups[g]->setVisible(old);
        statusProgress.setValue(++sp);
    }
}

void MainWindow::filterList() {
    qDebug() << "filterList";
    static std::atomic<bool> running(false);
    static QMutex mutex(QMutex::Recursive);
    static bool redo = false;
    mutex.lock();
    if (running.exchange(true)) {
        //Someone was running ... too bad ;) We signal redo and return
        qWarning() << "Redo!";
        redo = true;
        return;
    }
    mutex.unlock();

    do {
        redo = false;
        //Filter
        statusProgress.setRange(0,songFrames.count());
        int sp = 0;

        for (QString group : songGroups.keys()) {
            QList<SongFrame*> sl = groupedFrames[group];
            int cnt = 0;
            for (SongFrame* sf : sl) {
                qApp->processEvents();
                statusProgress.setValue(++sp);
                bool flt = filter(sf->song());
                sf->setVisible(flt);
                if (flt) {
                    cnt++;
                    sf->deselect();
                    selectedFrames.removeAll(sf);
                }
                if (redo) break;
            }
            if (redo) break;
            songGroups[group]->setVisible(cnt != 0);
        }


        mutex.lock();
        if (!redo) {
            if (!running.exchange(false)) qFatal("Broken lock state! This can not happen");
            mutex.unlock();
            return;
        }
        mutex.unlock(); //Redo :/
    } while (redo); //could also be while true!
    __builtin_unreachable();
}

void MainWindow::on_actionSources_triggered()
{
    SelectSongDirs ssd;
    ssd.setPaths(config.value("songdirs").toStringList());
    ssd.exec();
    config.setValue("songdirs",ssd.getPaths());
    if (ssd.changed())
        emit rescanCollection(config.value("songdirs").toStringList());
}

void MainWindow::sortFrames(QList<SongFrame*> &sf) {
    std::sort(sf.begin(),sf.end(),[this] (SongFrame* s1, SongFrame* s2)->bool {
        if (!ui->reverseSort->isChecked()) {
            if (ui->sortTitle->isChecked()) return (s1->song()->title() < s2->song()->title());
            if (ui->sortArtist->isChecked()) return (s1->song()->artist() < s2->song()->artist());
        } else {
            if (ui->sortTitle->isChecked()) return (s1->song()->title() > s2->song()->title());
            if (ui->sortArtist->isChecked()) return (s1->song()->artist() > s2->song()->artist());
        }
        qDebug() << "Fallback";
        return s1 < s2;
    });
}

void MainWindow::selectFrame(SongFrame *sf) {
    if (selectedFrames.contains(sf)) {
        sf->deselect();
        disconnect(sf->song(),&Song::updated,ui->songDetails,&SongInfo::selectionUpdated);
        selectedFrames.removeAll(sf);
    } else {
        //Enforce single selection for now!
        if (selectedFrames.size() == 1) {
            disconnect(selectedFrames.first()->song(),&Song::updated,ui->songDetails,&SongInfo::selectionUpdated);
            selectedFrames.first()->deselect();
            selectedFrames.removeFirst();
            qWarning() << "TODO: Multi selection not implemented ATM";
        }
        connect(sf->song(),&Song::updated,ui->songDetails,&SongInfo::selectionUpdated);
        sf->select();
        selectedFrames.append(sf);
    }
//    if (selectedFrames.count() > 0) { //THIS looks ugly ATM. Fix?
//        if (selectedFrames.first()->song()->hasBG())
//            ui->centralWidget->setStyleSheet(QString("background-image: url(%1)").arg(selectedFrames.first()->song()->bg().absoluteFilePath()));
//    }
    emit selectionChanged();
}

void MainWindow::addSong(Song *song) {
    songList.append(song);
    SongFrame* sf = new SongFrame(song,this);
    sf->hide();
    songFrames.append(sf);
    sf->connect(sf,&SongFrame::clicked,this,&MainWindow::selectFrame);
    sf->connect(sf,&SongFrame::playSong,ui->musicPlayer,&AudioPlayer::playSong);
//    ui->songList->layout()->addWidget(sw);
//    qApp->processEvents();
    if (!song->isValid()) invalidCount++;
    if (!song->isWellFormed() && song->isValid()) notWellFormedCount++;
    statusBar()->showMessage(QString("Scanning ... %1 Found, %2 invalid, %3 not well formed").arg(songList.size()).arg(invalidCount).arg(notWellFormedCount));
}

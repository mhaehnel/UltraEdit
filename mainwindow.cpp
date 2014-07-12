#include <QDebug>
#include <QSemaphore>
#include "mainwindow.h"
#include "selectsongdirs.h"
#include "songwidget.h"
#include "ui_mainwindow.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QMutex>
#include <atomic>

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
    scanner.moveToThread(&scanThread);
    ui->songList->setLayout(new QBoxLayout(QBoxLayout::Down));
    statusBar()->addPermanentWidget(&statusProgress);

    connect(this,&MainWindow::rescanCollection,&scanner,&CollectionScanner::scanCollection);
    connect(&scanner,&CollectionScanner::foundSong,this,&MainWindow::addSong);
    connect(&scanner,&CollectionScanner::scanFinished, this, &MainWindow::refreshList);
    connect(ui->actionRefresh_List,&QAction::triggered,this,&MainWindow::refreshList);
    connect(&scanner,&CollectionScanner::scanFinished, [this] {
//        statusProgress.setValue(100);
//        statusProgress.setRange(0,100);
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

bool MainWindow::filter(const Song *song) {
    switch (ui->missingBackgroundFile->checkState())
    {
      case Qt::Checked:   if (song->hasBG()) return false; break;
      case Qt::Unchecked: if (!song->hasBG()) return false; break;
    }
    switch (ui->missingCoverFile->checkState())
    {
      case Qt::Checked:   if (song->hasCover()) return false; break;
      case Qt::Unchecked: if (!song->hasCover()) return false; break;
    }
    switch (ui->missingVideoFile->checkState())
    {
      case Qt::Checked:   if (song->hasVideo()) return false; break;
      case Qt::Unchecked: if (!song->hasVideo()) return false; break;
    }
    switch (ui->missingMP3File->checkState())
    {
      case Qt::Checked:   if (song->mp3().exists()) return false; break;
      case Qt::Unchecked: if (!song->mp3().exists()) return false; break;
    }
    switch (ui->wellFormed->checkState()) {
        case Qt::Checked:   if (!song->isWellFormed()) return false; break;
        case Qt::Unchecked: if (song->isWellFormed()) return false; break;
    }
    switch (ui->isValid->checkState()) {
        case Qt::Checked:   if (!song->isValid()) return false; break;
        case Qt::Unchecked: if (song->isValid()) return false; break;
    }
    return true;
    //TODO: freestyle / goldennote / players
}

void MainWindow::refreshList() {
    QLayoutItem *li;
    static std::atomic<bool> running(false);
    static QMutex mutex(QMutex::Recursive);
    static bool redo = false;


    if (ui->songList->layout() == nullptr) return;
    mutex.lock();
    if (running.exchange(true)) {
        //Someone was running ... too bad ;) We signal redo and return
        qWarning() << "Redo!";
        redo = true;
        return;
    }
    mutex.unlock();

    qWarning() << "Creating items ...";
    do {
        redo = false;
        QFuture<Song*> sl = QtConcurrent::filtered(songlist,[this] (Song* s) -> bool { return filter(s); });
        int cnt = 0;
        sl.waitForFinished();
        statusProgress.setRange(0,songwidgets.count()); //sl.resultCount());
        for (SongWidget *sw : songwidgets) {
            statusProgress.setValue(cnt++);
            if (sl.results().contains(sw->song()) && !sw->isVisible()) {
                sw->show();
            } else if (!sl.results().contains(sw->song()) && sw->isVisible()) {
                sw->hide();
            }
            if (redo) break;
            qApp->processEvents();
        }
        mutex.lock();
        if (!redo) {
            if (!running.exchange(false)) qWarning() << "Broken lock state! This can not happen";
            qWarning() << "DONE!" << sl.resultCount();
            statusProgress.setValue(sl.resultCount());
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

void MainWindow::addSong(Song *song) {
    songlist.append(song);
    SongWidget* sw = new SongWidget(song,ui->songList);
    sw->hide();
    songwidgets.append(sw);
    ui->songList->layout()->addWidget(sw);
    if (!song->isValid()) invalidCount++;
    if (!song->isWellFormed() && song->isValid()) notWellFormedCount++;
    statusBar()->showMessage(QString("Scanning ... %1 Found, %2 invalid, %3 not well formed").arg(songlist.size()).arg(invalidCount).arg(notWellFormedCount));
}

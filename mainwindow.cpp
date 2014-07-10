#include <QDebug>
#include "mainwindow.h"
#include "selectsongdirs.h"
#include "songwidget.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), notWellFormedCount(0), invalidCount(0),
    ui(new Ui::MainWindow),
  config("MH-Development","UltraEdit")
{
    //Initialize
    ui->setupUi(this);
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
    connect(&scanner,&CollectionScanner::scanFinished, [this] {
        statusProgress.setVisible(false);
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
    //scanThread.terminate();
    delete ui;
}

bool MainWindow::filter(const Song &song) {
    switch (ui->missingBackgroundFile->checkState())
    {
      case Qt::Checked:   if (song.hasBG()) return false; break;
      case Qt::Unchecked: if (!song.hasBG()) return false; break;
    }
    switch (ui->missingCoverFile->checkState())
    {
      case Qt::Checked:   if (song.hasCover()) return false; break;
      case Qt::Unchecked: if (!song.hasCover()) return false; break;
    }
    switch (ui->missingVideoFile->checkState())
    {
      case Qt::Checked:   if (song.hasVideo()) return false; break;
      case Qt::Unchecked: if (!song.hasVideo()) return false; break;
    }
    switch (ui->missingMP3File->checkState())
    {
      case Qt::Checked:   if (song.mp3().exists()) return false; break;
      case Qt::Unchecked: if (!song.mp3().exists()) return false; break;
    }
    switch (ui->wellFormed->checkState()) {
        case Qt::Checked:   if (!song.isWellFormed()) return false; break;
        case Qt::Unchecked: if (song.isWellFormed()) return false; break;
    }
    switch (ui->isValid->checkState()) {
        case Qt::Checked:   if (!song.isValid()) return false; break;
        case Qt::Unchecked: if (song.isValid()) return false; break;
    }
    return true;
    //TODO: freestyle / goldennote / players
}

void MainWindow::refreshList() {
    QLayoutItem *li;
    while ((li = ui->songList->layout()->takeAt(0))) {
        delete li;
    }
    qWarning() << "Creating items ...";
    int cnt = 0;
    ui->songList->setUpdatesEnabled(false);
    for (Song* s : songlist) {
        if (!filter(*s)) continue;
        cnt++;
        //TODO: Filter!
        ui->songList->layout()->addWidget(new SongWidget(*s));
    }
    ui->songList->setUpdatesEnabled(true);
    qWarning() << "DONE!" << cnt;
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
    if (!song->isValid()) invalidCount++;
    if (!song->isWellFormed() && song->isValid()) notWellFormedCount++;
    statusBar()->showMessage(QString("Scanning ... %1 Found, %2 invalid, %3 not well formed").arg(songlist.size()).arg(invalidCount).arg(notWellFormedCount));
}

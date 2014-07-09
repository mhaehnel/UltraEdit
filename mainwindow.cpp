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
    scanThread.terminate();
    delete ui;
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
        //TODO: Filter!
        ui->songList->layout()->addWidget(new SongWidget(*s));
    }
    ui->songList->setUpdatesEnabled(true);
    qWarning() << "DONE!";
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

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
#include <QProgressDialog>
#include <QInputDialog>

#include <exceptions/songparseexception.h>
#include <exceptions/sylabelformatexception.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), notWellFormedCount(0), invalidCount(0),
    ui(std::make_unique<Ui::MainWindow>()),
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
    ui->missingMP3File->setCheckState(Qt::PartiallyChecked);
    ui->missingSongtextFile->setCheckState(Qt::PartiallyChecked);
    ui->missingVideoFile->setCheckState(Qt::PartiallyChecked);
    ui->startsAt0->setCheckState(Qt::PartiallyChecked);
    ui->wellFormed->setCheckState(Qt::PartiallyChecked);
    ui->isValid->setCheckState(Qt::PartiallyChecked);
    ui->songDetails->setSelection(&selectedFrames);
    ui->songList->setLayout(new QBoxLayout(QBoxLayout::Down));
    ui->artistFilter->setText("Lily");
    statusBar()->addPermanentWidget(&statusProgress);
    connect(this,&MainWindow::selectionChanged,ui->songDetails,&SongInfo::selectionChanged);
    connect(ui->songDetails,&SongInfo::seek,ui->musicPlayer,&AudioPlayer::seek);
    connect(ui->musicPlayer,&AudioPlayer::seeking, ui->songDetails, &SongInfo::seekTo);
    connect(ui->songDetails,&SongInfo::play,ui->musicPlayer,&AudioPlayer::play);
    connect(ui->musicPlayer,&AudioPlayer::started, ui->songDetails, &SongInfo::startPlayback);
    connect(ui->songDetails,&SongInfo::pause,ui->musicPlayer,&AudioPlayer::pause);
    connect(ui->musicPlayer,&AudioPlayer::paused, ui->songDetails, &SongInfo::pausePlayback);
    QTimer::singleShot(0,this,SLOT(rescanCollection()));
    ui->songDetails->setMidiPort(config.value("midiPort","").toString());

}


MainWindow::~MainWindow() {
    ui->songDetails->pausePlayback();
}

void MainWindow::rescanCollection() {
    QProgressDialog progress("Scanning collection ..","Cancel",0,0,this);
    progress.setWindowModality(Qt::WindowModal);
    QStringList paths = config.value("songdirs").toStringList();
    qDebug() << "Go ahead :)";
    for (QString d : paths) {
        //TODO: This is leaking on rescan!
        qDebug() << "Scannning:"  << d;
        QDirIterator di(d,QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (di.hasNext()) {
            QFileInfo fi(di.next());
            if (fi.suffix().compare("txt",Qt::CaseInsensitive)) continue;
            try {
                addSong(new Song(fi,d));
            } catch (SongParseException& e) {
                qDebug() << "Error: " << e.what();
            } catch (SylabelFormatException& e) {
                qDebug() << "Error: " << e.what();
            }
            qApp->processEvents();
        };
    }
    ((QBoxLayout*)ui->songList->layout())->addStretch(1);
    ui->groupBy->clear();
    ui->groupBy->addItems(Song::seenTags());
    if (Song::seenTags().contains("ARTIST"))  {
        ui->groupBy->setCurrentIndex(Song::seenTags().indexOf("ARTIST"));
    } else {
        ui->groupBy->setCurrentIndex(0);
    }
    //regroupList();
    selectedFrames.clear();

    emit selectionChanged();
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
    __builtin_unreachable();
}

bool MainWindow::filter(const Song *song) {
    if (!filterState(ui->missingBackgroundFile,std::bind(&Song::missingBG,song))) return false;
    if (!filterState(ui->missingCoverFile,std::bind(&Song::missingCover,song))) return false;
    if (!filterState(ui->missingVideoFile,std::bind(&Song::missingVideo,song))) return false;
    if (!filterState(ui->hasBackground,std::bind(&Song::hasBG,song))) return false;
    if (!filterState(ui->hasVideo,std::bind(&Song::hasVideo,song))) return false;
    if (!filterState(ui->wellFormed,std::bind(&Song::isWellFormed,song))) return false;
//TODO:    if (!filterState(ui->isValid,std::bind(&Song::isValid,song))) return false;
    if (!filterState(ui->missingMP3File,[&song] { return !song->mp3().exists(); })) return false;

    if (!ui->titleFilter->text().isEmpty() && !song->title().contains(ui->titleFilter->text(),Qt::CaseInsensitive)) return false;
    if (!ui->artistFilter->text().isEmpty() && !song->artist().contains(ui->artistFilter->text(),Qt::CaseInsensitive)) return false;
    return true; //In!
    //TODO: freestyle / goldennote / players
}

QString MainWindow::getGroup(Song *song) {
    if (!ui->groupList->isChecked()) return "*";
    //we need to group
    QString tag = ui->groupBy->currentText();
    if (song->hasTag(tag))
        return song->tag(tag);
    else
        return "*";
}

//This does currently not support rapid regroups! Lock it to make it work!
void MainWindow::regroupList() {
    static QMutex mutex(QMutex::Recursive);
    mutex.lock();
    groupedFrames.clear();  //Frames are only once instanciated. Do not delete!
    for (SongGroup* sg : songGroups) sg->deleteLater();
    songGroups.clear();

    for (SongFrame* sf : songFrames) {
        QString group = getGroup(sf->song);
        groupedFrames[group].append(sf);
        songGroups[group] = new SongGroup(group);
        songGroups[group]->hide();
    }
    for (SongGroup* sg : songGroups) ui->songList->layout()->addWidget(sg);
    //Sort in group
    resortList();
    //hide filtered frames / empty groups
    filterList();
    mutex.unlock();
}

//This is expensive since it has to readd all layouts! For all groups ... kinde sucks ... so ... regroup is costly!
void MainWindow::resortList() {
    static bool done;
    done = false;
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
            if (done) return;
            songGroups[g]->layout().addWidget(f);
        }
        songGroups[g]->setVisible(old);
        statusProgress.setValue(++sp);
    }
    done = true;
}

void MainWindow::filterList() {
    static bool done;
    done = false;

    //Filter
    statusProgress.setRange(0,songFrames.count());
    int sp = 0;

    for (QString group : songGroups.keys()) {
        QList<SongFrame*> sl = groupedFrames[group];
        int cnt = 0;
        for (SongFrame* sf : sl) {
            qApp->processEvents();
            if (done) return;
            statusProgress.setValue(++sp);
            bool flt = filter(sf->song);
            sf->setVisible(flt);
            if (flt) {
                cnt++;
                sf->deselect();
                selectedFrames.removeAll(sf);
            }
        }
        songGroups[group]->setVisible(cnt != 0);
    }
    done = true;
}

void MainWindow::sortFrames(QList<SongFrame*> &sf) {
    std::sort(sf.begin(),sf.end(),[this] (SongFrame* s1, SongFrame* s2)->bool {
        if (!ui->reverseSort->isChecked()) {
            if (ui->sortTitle->isChecked()) return (s1->song->title() < s2->song->title());
            if (ui->sortArtist->isChecked()) return (s1->song->artist() < s2->song->artist());
        } else {
            if (ui->sortTitle->isChecked()) return (s1->song->title() > s2->song->title());
            if (ui->sortArtist->isChecked()) return (s1->song->artist() > s2->song->artist());
        }
        qDebug() << "Fallback";
        return s1 < s2;
    });
}

void MainWindow::selectFrame(SongFrame *sf) {
    if (selectedFrames.contains(sf)) {
        sf->deselect();
        disconnect(sf->song,&Song::updated,ui->songDetails,&SongInfo::selectionUpdated);
        selectedFrames.removeAll(sf);
        ui->musicPlayer->stop();
        ui->songDetails->pausePlayback();
    } else {
        //Enforce single selection for now!
        if (selectedFrames.size() == 1) {
            disconnect(selectedFrames.first()->song,&Song::updated,ui->songDetails,&SongInfo::selectionUpdated);
            selectedFrames.first()->deselect();
            selectedFrames.removeFirst();
            qWarning() << "TODO: Multi selection not implemented ATM";
        }
        connect(sf->song,&Song::updated,ui->songDetails,&SongInfo::selectionUpdated);
        sf->select();
        ui->musicPlayer->setSong(sf->song);
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
    sf->connect(sf,&SongFrame::playSong,[this,sf] (Song* ) { if (!selectedFrames.contains(sf)) selectFrame(sf); ui->musicPlayer->play(); });
    //TODO: if (!song->isValid()) invalidCount++;
    if (!song->isWellFormed()) notWellFormedCount++;
    statusBar()->showMessage(QString("Scanning ... %1 Found, %2 invalid, %3 not well formed").arg(songList.size()).arg(invalidCount).arg(notWellFormedCount));
}

void MainWindow::on_actionSources_triggered()
{
    SelectSongDirs ssd;
    ssd.setPaths(config.value("songdirs").toStringList());
    ssd.exec();
    config.setValue("songdirs",ssd.getPaths());
    if (ssd.changed())
        rescanCollection();
}

void MainWindow::on_actionMidi_Output_triggered()
{
    QStringList items = ui->songDetails->getMidiPorts();
    int current = items.indexOf(config.value("midiPort","").toString());
    bool ok;
    QString item = QInputDialog::getItem(this, "Player subscription",
                                         "Output port:", items,
                                         current, false, &ok);
    if (ok && !item.isEmpty()) {
        config.setValue("midiPort",item);
        ui->songDetails->setMidiPort(item);
    }
}

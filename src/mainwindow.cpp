#include <QDebug>
#include <QSemaphore>
#include "mainwindow.h"
#include "collectioneditor.h"
#include "ui_mainwindow.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QMutex>
#include <atomic>
#include <cassert>
#include <functional>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>


#include <songimportdialog.h>
#include <chrono>

#include <exceptions/songparseexception.h>
#include <exceptions/sylabelformatexception.h>

using namespace std::chrono_literals;

static QList<Collection*> deserializeCollectionList(const QStringList& sl) {
    QList<Collection*> cols;
    for (const QString&  s : sl) {
        QStringList parts = s.split('|');
        if (parts.size() != 3) {
            qDebug() << "Invalid collection" << s;
            continue;
        }
        cols.append(new Collection(parts[0],parts[1],parts[2]));
    }
    return cols;
}

static QStringList serializeCollectionList(const QList<Collection*>& cl) {
    QStringList colStrings;
    for (const Collection* c : cl) {
        colStrings.push_back(c->name()+"|"+c->basePath()+"|"+c->pathRule());
    }
    return colStrings;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), notWellFormedCount(0), invalidCount(0),
    ui(new Ui::MainWindow())
{
    ui->setupUi(this);
    qRegisterMetaType<Song*>();
    for (auto cb : ui->filter->findChildren<QCheckBox *>())
        cb->setCheckState(Qt::PartiallyChecked);
    ui->songDetails->setSelection(&selectedFrames);
    ui->songList->setLayout(new QBoxLayout(QBoxLayout::Down));
    statusBar()->addPermanentWidget(&statusProgress);
    connect(this,&MainWindow::selectionChanged,ui->songDetails,&SongInfo::selectionChanged);
    connect(ui->songDetails,&SongInfo::seek,ui->musicPlayer,&MediaPlayer::seek);
    connect(ui->songDetails,&SongInfo::play,ui->musicPlayer,&MediaPlayer::play);
    connect(ui->songDetails,&SongInfo::pause,ui->musicPlayer,&MediaPlayer::pause);
    connect(ui->songDetails,&SongInfo::reloadMedia,ui->musicPlayer,&MediaPlayer::reloadMedia);
    connect(ui->songDetails,&SongInfo::popOut, [this] { popOut(2); });
    QTimer::singleShot(0,this,SLOT(rescanCollections()));
    ui->musicPlayer->connectMidiPort(config.value("midiPort","").toString());
    ui->musicPlayer->setVideoOutput(ui->songDetails->videoWidget());
}


MainWindow::~MainWindow() {
    ui->musicPlayer->stop();
    for (Collection* c : collections) delete c;
    delete ui;
}


template<typename T>
auto seconds_since(T base) {
    return std::chrono::duration<double>(T::clock::now() - base).count();
}

void MainWindow::collectionUpdated() {
    ui->groupBy->clear();
    ui->groupBy->addItems(Song::seenTags());
    auto begin = std::chrono::steady_clock::now();
    if (Song::seenTags().contains("ARTIST"))  {
        ui->groupBy->setCurrentIndex(Song::seenTags().indexOf("ARTIST"));
    } else {
        ui->groupBy->setCurrentIndex(0);
    }
    qDebug() << "Grouped in " << seconds_since(begin) << "s";
    begin = std::chrono::steady_clock::now();
    selectedFrames.clear();

    emit selectionChanged();
}

void MainWindow::rescanCollections() {
    for (Collection* c : collections) delete c; //remove old collections (and songs)
    collections = deserializeCollectionList(config.value("collections").toStringList());
    for (SongFrame* s : songFrames) delete s;
    songFrames.clear();

    auto start = std::chrono::steady_clock::now();
    auto begin = start;
    for (Collection* d : collections) {
        qDebug() << "Scannning:"  << d->name();
        QDirIterator di(d->basePath(),QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
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
            auto cur = std::chrono::steady_clock::now();
            if (cur - start >= 300ms) {
                start = cur;
                qApp->processEvents();
            }
        };
    }
    ((QBoxLayout*)ui->songList->layout())->addStretch(1);
    qDebug() << "Scanned in " << seconds_since(begin) << "s";
    collectionUpdated();
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

void MainWindow::popOut(int idx) {
    if (splitterState.isEmpty()) {
        splitterState = ui->splitter->saveState();
        QList<int> sizes;
        for (int i = 0; i < ui->splitter->count(); i++)
            sizes.push_back((i==idx));
        ui->splitter->setSizes(sizes);
        this->menuBar()->hide();
        this->statusBar()->hide();
        for (auto tb : this->findChildren<QToolBar *>())
            tb->hide();
    } else {
        ui->splitter->restoreState(splitterState);
        splitterState.clear();
        this->menuBar()->show();
        this->statusBar()->show();
        for (auto tb : this->findChildren<QToolBar *>())
            tb->show();
    }

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
    auto start = std::chrono::steady_clock::now();
    for (QString g : songGroups.keys()) {
        QLayoutItem* li = nullptr;
        bool old = songGroups[g]->isVisible();
        songGroups[g]->setVisible(false);
        while ((li = songGroups[g]->layout().takeAt(0)) != nullptr) {
            songGroups[g]->layout().removeItem(li);
        }
        for (SongFrame* f : groupedFrames[g]) {
            if (done) return;
            songGroups[g]->layout().addWidget(f);
        }
        songGroups[g]->setVisible(old);
        statusProgress.setValue(++sp);
        auto cur = std::chrono::steady_clock::now();
        if (cur - start >= 300ms) {
            start = cur;
            qApp->processEvents();
        }

    }
    done = true;
}

void MainWindow::filterList() {
    static bool done;
    done = false;

    //Filter
    statusProgress.setRange(0,songFrames.count());
    int sp = 0;

    auto start = std::chrono::steady_clock::now();
    for (QString group : songGroups.keys()) {
        QList<SongFrame*> sl = groupedFrames[group];
        int cnt = 0;
        for (SongFrame* sf : sl) {
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
        auto cur = std::chrono::steady_clock::now();
        if (cur - start >= 300ms) {
            start = cur;
            qApp->processEvents();
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
    SongFrame* sf = new SongFrame(song,this);
    sf->hide();
    songFrames.append(sf);
    sf->connect(sf,&SongFrame::clicked,this,&MainWindow::selectFrame);
    sf->connect(sf,&SongFrame::playSong,[this,sf] (Song* ) { if (!selectedFrames.contains(sf)) selectFrame(sf); ui->musicPlayer->play(); });
    //TODO: if (!song->isValid()) invalidCount++;
    if (!song->isWellFormed()) notWellFormedCount++;
    size_t total = std::accumulate(collections.begin(),collections.end(), size_t{},
                                [] (const int& s, const Collection* c) {
        return s + c->size();
    });

    statusBar()->showMessage(QString("Scanning ... %1 Songs found in %2 collections, %3 invalid, %4 not well formed").arg(total).arg(collections.size()).arg(invalidCount).arg(notWellFormedCount));
}

void MainWindow::on_actionSources_triggered()
{
    auto cols = deserializeCollectionList(config.value("collections").toStringList());
    CollectionEditor ssd(cols);
    ssd.exec();
    config.setValue("collections",serializeCollectionList(ssd.collections()));
    if (ssd.changed())
        rescanCollections();
}

void MainWindow::on_actionMidi_Output_triggered()
{
    QStringList items = ui->musicPlayer->midiPorts();
    int current = items.indexOf(config.value("midiPort","").toString());
    bool ok;
    QString item = QInputDialog::getItem(this, "Player subscription",
                                         "Output port:", items,
                                         current, false, &ok);
    if (ok && !item.isEmpty()) {
        config.setValue("midiPort",item);
        ui->musicPlayer->connectMidiPort(item);
    }
}

void MainWindow::on_actionImport_Directory_triggered() {
    qDebug() << "Import directory not implemented";
}

void MainWindow::on_actionImport_Song_triggered() {
    QString fileName = QFileDialog::getOpenFileName(this,"Open US Song",config.value("openSongPath").toString(),"USDX Songs (*.txt)");
    if (fileName.isEmpty()) return;
    QFileInfo fi(fileName);
    config.setValue("openSongPath",fi.absolutePath());
    try {
        Song* s = new Song(fi);
        SongImportDialog sid(collections,s,this);
        if (sid.exec() == QDialog::Rejected) {
            delete s;
            return;
        }
        addSong(s);
        collectionUpdated();
    } catch (std::exception& e) {
        QString msg("Importing the file '%1' failed with error:\n%2");
        QMessageBox::critical(this,"Import failed",msg.arg(fileName,QString(e.what())));
        return;
    }
}

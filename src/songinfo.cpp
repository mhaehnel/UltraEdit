#include "songinfo.h"
#include "ui_songinfo.h"
#include "ui_actionitem.h"
#include <cassert>
#include <QDebug>
#include <mediaplayer.h>
#include <QFileDialog>
#include <QMessageBox>

#include <actions/modifytag.h>
#include <actions/changefile.h>

SongInfo::SongInfo(QWidget *parent) :
    QWidget(parent), ui(new Ui::SongInfo())
{
    ui->setupUi(this);
    connect(ui->notes,&NoteWidget::play,this,&SongInfo::play);
    connect(ui->notes,&NoteWidget::pause,this,&SongInfo::pause);
    connect(ui->notes,&NoteWidget::seek,this,&SongInfo::seek);
    connect(ui->popoutVideo,&QPushButton::pressed,[this] {
        ui->popoutVideo->setChecked(!ui->popoutVideo->isChecked());
        ui->tabWidget->tabBar()->setVisible(!ui->tabWidget->tabBar()->isVisible());
        ui->songChanged->setVisible(!ui->popoutVideo->isChecked() && selection->first()->song->isModified());
        emit popOut();
    });
    connect(ui->undoButton,&QPushButton::clicked,[this] {
        if (song_ != nullptr) song_->undo(1);
    });
    connect(ui->redoButton,&QPushButton::clicked,[this] {
        if (song_ != nullptr) song_->redo(1);
    });
    connect(ui->revertSong,&QPushButton::clicked, [this] () {
           if (song_ != nullptr) song_->undo(ui->undoList->count());
    });
    connect(ui->saveSong,&QPushButton::clicked,[this] () {
        if (song_ != nullptr) song_->saveSong();
    });

    ui->videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    ui->actionItemsScrollArea->hide();
    ui->songMessages->hide();
    ui->songChanged->hide();
}

SongInfo::~SongInfo() {
    delete ui;
}

void SongInfo::updateVideoInfo(QSize resolution, QString codec) {
    ui->videoResolution->setText(QString("%1x%2").arg(resolution.width()).arg(resolution.height()));
    ui->videoCodec->setText(codec);
}


void SongInfo::setSelection(QList<SongFrame*>* selected) {
    if (MediaPlayer::instance != nullptr && selection == nullptr) {
        connect(MediaPlayer::instance,&MediaPlayer::haveVideoInfo,this,&SongInfo::updateVideoInfo);
    }
    selection = selected;
    selectionChanged();
}

QVideoWidget* SongInfo::videoWidget() {
    return ui->videoWidget;
}

void SongInfo::selectionChanged() {
    bool singleSelect = (selection->size() == 1);
    ui->filesGroup->setEnabled(singleSelect);
    ui->title->setEnabled(singleSelect);
    ui->titleLabel->setEnabled(singleSelect);
    ui->lyricsTab->setDisabled(!singleSelect);
    ui->tonesTab->setDisabled(!singleSelect);
    ui->mediaTab->setDisabled(!singleSelect);

    ui->artist->setEnabled(selection->size() > 0);
    ui->artistLabel->setEnabled(selection->size() > 0);
    ui->genre->setEnabled(selection->size() > 0);
    ui->genreLabel->setEnabled(selection->size() > 0);
    ui->edition->setEnabled(selection->size() >0);
    ui->editionLabel->setEnabled(selection->size() >0);

    for (auto i : {conSylText, conSylLine, conSyl, conUpdate})
        this->disconnect(i);
    if (selection->isEmpty()) {
        song_ = nullptr;
        return;
    }

    song_ = selection->first()->song;
    ui->notes->setSong(song_);
    conSylText = connect(song_,static_cast<void (Song::*)(int,int)>(&Song::playingSylabel),this,&SongInfo::highlightText);
    conSyl = connect(song_,static_cast<void (Song::*)(Sylabel*)>(&Song::playingSylabel),ui->notes,&NoteWidget::setCurrentNote);
    conSylLine = connect(song_,&Song::lineChanged,ui->notes, &NoteWidget::setLine);
    conUpdate = connect(song_,&Song::updated,[this] {
        ui->songChanged->setVisible(song_->isModified());
        selectionUpdated();
    });
    ui->songChanged->setVisible(song_->isModified());

    selectionUpdated();
}

void SongInfo::selectionUpdated() {
    //If we don't do this we will get update signals. Thats not usefull as we are changing it and not the user
    for (QWidget* w : this->findChildren<QWidget*>())
        w->blockSignals(true);

    ui->title->setText(song_->title());
    ui->artist->setText(song_->artist());
    ui->edition->setText(song_->tag("Edition"));
    ui->genre->setText(song_->tag("Genre"));
    ui->songLabel->setText(QString("<HTML><H1><CENTER>%1<BR/>%2</CENTER></H1></HTML>").arg(song_->title(),song_->artist()));

    ui->mp3file->setText(song_->tag("MP3"));
    ui->mp3file->setStyleSheet(!song_->mp3().exists()?"background-color: red":"");
    ui->vidFile->setText(song_->tag("VIDEO"));
    ui->vidFile->setStyleSheet((song_->hasVideo() && !song_->vid().exists())?"background-color: red":"");
    ui->bgFile->setText(song_->tag("BACKGROUND"));
    ui->bgFile->setStyleSheet((song_->hasBG() && !song_->bg().exists())?"background-color: red":"");
    ui->covFile->setText(song_->tag("COVER"));
    ui->covFile->setStyleSheet((song_->hasCover() && !song_->cov().exists())?"background-color: red":"");
    ui->rawLyrics->setText(song_->rawLyrics());
    ui->songMessages->clear();
    ui->undoList->clear();
    ui->redoList->clear();
//TODO:        for (QString m : s->fatals())
//            ui->songMessages->addItem(new QListWidgetItem(QIcon::fromTheme("dialog-error"),m));
    for (QString m : song_->errors())
        ui->songMessages->addItem(new QListWidgetItem(QIcon::fromTheme("dialog-warning"),m));
    for (QString m : song_->warnings())
        ui->songMessages->addItem(new QListWidgetItem(QIcon::fromTheme("dialog-information"),m));
    ui->rawData->setText(song_->rawData());
    for (QWidget* w : findChildren<QWidget*>())
        w->blockSignals(false);

    ui->songMessages->setVisible(ui->songMessages->count() != 0);


    auto aiLayout = dynamic_cast<QBoxLayout*>(ui->actionItems->layout());
    Q_ASSERT(aiLayout != nullptr);

    while (aiLayout->count() > 1)
        aiLayout->takeAt(0)->widget()->deleteLater();

    for (const auto& ai : song_->actionItems()) {
        QFrame* qw = new QFrame();
        std::unique_ptr<Ui::ActionItem> aiui(std::make_unique<Ui::ActionItem>());
        aiui->setupUi(qw);
        aiui->actionText->setText(ai->description());
        switch (ai->severity()) {
            case ActionItem::Severity::Error:
                aiui->actionIcon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(64));
                break;
            case ActionItem::Severity::Warning:
                aiui->actionIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(64));
                break;
            case ActionItem::Severity::Info:
                aiui->actionIcon->setPixmap(QIcon::fromTheme("dialog-information").pixmap(64));
                break;
        }

        aiLayout->insertWidget(0,qw);
    }
    ui->actionItemsScrollArea->setVisible(!song_->actionItems().empty());

    for (QString m : song_->performedActions()) {
        ui->undoList->addItem(new QListWidgetItem(QIcon::fromTheme("edit-undo"),m));
    }
    for (QString m : song_->undoneActions()) {
        ui->redoList->addItem(new QListWidgetItem(QIcon::fromTheme("edit-redo"),m));
    }
    ui->undoButton->setEnabled(song_->canUndo());
    ui->undoButton_2->setEnabled(song_->canUndo());
    ui->redoButton->setDisabled(ui->redoList->count() == 0);
    ui->redoButton_2->setDisabled(ui->redoList->count() == 0);
}

void SongInfo::on_title_textChanged(const QString &arg1)
{
    Song* s = selection->first()->song;
    if (!s->performAction(std::make_unique<Song::ModifyTag>("TITLE",Song::ModifyTag::Op::Modify,arg1)))
        qWarning() << "Updating title failed!";
}

void SongInfo::highlightText(int from, int to) {
    QTextCursor c = ui->rawLyrics->textCursor();
    if (c.anchor() == from && c.position() == to) return;
    c.setPosition(from);
    c.setPosition(to,QTextCursor::KeepAnchor);
    ui->rawLyrics->setTextCursor(c);
}

void SongInfo::on_artist_textChanged(const QString &arg1)
{
    for (SongFrame* sf : *selection) {
        if (!sf->song->performAction(std::make_unique<Song::ModifyTag>("ARTIST",Song::ModifyTag::Op::Modify,arg1)))
            qWarning() << "Updating artist failed!";
    }
}

void SongInfo::handleUpdate(QString title,QString filter, const QFileInfo& currentFile, Song::ChangeFile::FileType type) {
    QString fname = QFileDialog::getOpenFileName(this,title,song_->txt().absolutePath(),filter);
    if (!fname.isEmpty()) {
        if (fname == currentFile.absoluteFilePath()) {
            QMessageBox::information(this,"File not changed","The file you provided is the same that is currently used.\n\nNothing was changed");
            return;
        }
        song_->performAction(std::make_unique<Song::ChangeFile>(fname,type));
    }
}

void SongInfo::updateFile() {
   if (QObject::sender() == ui->setMP3File) {
        handleUpdate("Select MP3 File","MP3 Files (*.mp3)",
                     song_->mp3(),Song::ChangeFile::FileType::MP3);
        emit reloadMedia();
    } else if (QObject::sender() == ui->setCovFile) {
        handleUpdate("Select Cover","Images (*.png *.xpm *.jpg);;All Files (*.*)",
                     song_->cov(),Song::ChangeFile::FileType::COVER);
    } else if (QObject::sender() == ui->setBGFile) {
        handleUpdate("Select Background","Images (*.png *.xpm *.jpg);;All Files (*.*)",
                     song_->cov(),Song::ChangeFile::FileType::BACKGROUND);
    } else if (QObject::sender() == ui->setVidFile) {
        handleUpdate("Select Video","Videos (*.mkv *.mpg *.mpeg *.avi *.flv *.mov *.mp4);;All Files (*.*)",
                     song_->cov(),Song::ChangeFile::FileType::VIDEO);
        emit reloadMedia();
    } else if (QObject::sender() == ui->removeBackground) {
       song_->performAction(std::make_unique<Song::ChangeFile>(QString::null,Song::ChangeFile::FileType::BACKGROUND));
   } else if (QObject::sender() == ui->removeCover) {
      song_->performAction(std::make_unique<Song::ChangeFile>(QString::null,Song::ChangeFile::FileType::COVER));
   } else if (QObject::sender() == ui->removeVideo) {
      song_->performAction(std::make_unique<Song::ChangeFile>(QString::null,Song::ChangeFile::FileType::VIDEO));
      emit reloadMedia();
    } else {
        qDebug() << "Changing this file is not implemented";
    }
}

#include "songinfo.h"
#include "ui_songinfo.h"
#include "ui_actionitem.h"
#include <cassert>
#include <QDebug>

#include <actions/modifytag.h>

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

    ui->videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    ui->actionItemsScrollArea->hide();
    ui->songMessages->hide();
    ui->songChanged->hide();
}

SongInfo::~SongInfo() {
    delete ui;
}

void SongInfo::setSelection(QList<SongFrame*>* selected) {
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

    if (selection->size() == 1) { //TODO: Support mass edit
        Song* s = selection->first()->song;
        ui->notes->setSong(s);
        conSylText = connect(s,static_cast<void (Song::*)(int,int)>(&Song::playingSylabel),this,&SongInfo::highlightText);
        conSyl = connect(s,static_cast<void (Song::*)(Sylabel*)>(&Song::playingSylabel),ui->notes,&NoteWidget::setCurrentNote);
        conSylLine = connect(s,&Song::lineChanged,ui->notes, &NoteWidget::setLine);
        conUpdate = connect(s,&Song::updated,[this,s] {
            ui->songChanged->setVisible(s->isModified());
        });
        ui->songChanged->setVisible(s->isModified());
    }
    selectionUpdated();
}

void SongInfo::selectionUpdated() {
    //Title
    if (selection->size() == 1) {
        //If we don't do this we will get update signals. Thats not usefull as we are changing it and not the user
        for (QWidget* w : this->findChildren<QWidget*>())
            w->blockSignals(true);

        Song* s = selection->first()->song;
        ui->title->setText(s->title());
        ui->artist->setText(s->artist());
        ui->edition->setText(s->tag("Edition"));
        ui->genre->setText(s->tag("Genre"));
        ui->songLabel->setText(QString("<HTML><H1><CENTER>%1<BR/>%2</CENTER></H1></HTML>").arg(s->title(),s->artist()));

        ui->mp3file->setText(s->tag("MP3"));
        ui->mp3file->setStyleSheet(!s->mp3().exists()?"background-color: red":"");
        ui->vidFile->setText(s->tag("VIDEO"));
        ui->vidFile->setStyleSheet((s->hasVideo() && !s->vid().exists())?"background-color: red":"");
        ui->bgFile->setText(s->tag("BACKGROUND"));
        ui->bgFile->setStyleSheet((s->hasBG() && !s->bg().exists())?"background-color: red":"");
        ui->covFile->setText(s->tag("COVER"));
        ui->covFile->setStyleSheet((s->hasCover() && !s->cov().exists())?"background-color: red":"");
        ui->rawLyrics->setText(s->rawLyrics());
        ui->songMessages->clear();
        ui->undoList->clear();
        ui->redoList->clear();
//TODO:        for (QString m : s->fatals())
//            ui->songMessages->addItem(new QListWidgetItem(QIcon::fromTheme("dialog-error"),m));
        for (QString m : s->errors())
            ui->songMessages->addItem(new QListWidgetItem(QIcon::fromTheme("dialog-warning"),m));
        for (QString m : s->warnings())
            ui->songMessages->addItem(new QListWidgetItem(QIcon::fromTheme("dialog-information"),m));
        ui->rawData->setText(s->rawData());
        for (QWidget* w : findChildren<QWidget*>())
            w->blockSignals(false);

        ui->songMessages->setVisible(ui->songMessages->count() != 0);


        auto aiLayout = dynamic_cast<QBoxLayout*>(ui->actionItems->layout());
        Q_ASSERT(aiLayout != nullptr);

        while (aiLayout->count() > 1)
            aiLayout->takeAt(0)->widget()->deleteLater();

        for (const auto& ai : s->actionItems()) {
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
        ui->actionItemsScrollArea->setVisible(!s->actionItems().empty());

        for (QString m : s->performedActions()) {
            ui->undoList->addItem(new QListWidgetItem(QIcon::fromTheme("edit-undo"),m));
        }
        for (QString m : s->undoneActions()) {
            ui->redoList->addItem(new QListWidgetItem(QIcon::fromTheme("edit-redo"),m));
        }
        ui->undoButton->setDisabled(ui->undoList->count() == 0);
        ui->undoButton_2->setDisabled(ui->undoList->count() == 0);
        ui->redoButton->setDisabled(ui->redoList->count() == 0);
        ui->redoButton_2->setDisabled(ui->redoList->count() == 0);

        static QMetaObject::Connection undoCon, redoCon, resetCon;
        QObject::disconnect(undoCon);
        QObject::disconnect(redoCon);
        QObject::disconnect(resetCon);
        undoCon = connect(ui->undoButton,&QPushButton::clicked,[this] { selection->first()->song->undo(1); } );
        redoCon = connect(ui->redoButton,&QPushButton::clicked,[this] { selection->first()->song->redo(1); } );
        resetCon = connect(ui->revertSong,&QPushButton::clicked, [this] () {
               selection->first()->song->undo(ui->undoList->count());
        });
    }
    //TODO: Support mass edit!
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

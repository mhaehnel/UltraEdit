#include "songinfo.h"
#include "ui_songinfo.h"
#include <cassert>
#include <QDebug>

SongInfo::SongInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SongInfo)
{
    ui->setupUi(this);
}

SongInfo::~SongInfo()
{
    delete ui;
}

void SongInfo::setSelection(QList<SongFrame*>* selected) {
    selection = selected;
    selectionUpdated();
}

void SongInfo::selectionUpdated() {
    ui->filesGroup->setEnabled(selection->size() == 1);
    ui->settingsGroup->setEnabled(selection->size() == 1);
    ui->title->setEnabled(selection->size() == 1);
    ui->titleLabel->setEnabled(selection->size() == 1);
    ui->artist->setEnabled(selection->size() > 0);
    ui->artistLabel->setEnabled(selection->size() > 0);
    ui->genre->setEnabled(selection->size() > 0);
    ui->genreLabel->setEnabled(selection->size() > 0);
    ui->edition->setEnabled(selection->size() >0);
    ui->editionLabel->setEnabled(selection->size() >0);
    ui->lyricsTab->setDisabled(selection->size() != 1);
    ui->tonesTab->setDisabled(selection->size() != 1);
    ui->mediaTab->setDisabled(selection->size() != 1);
    //Title
    this->disconnect(con);
    if (selection->size() == 1) {
        //If we don't do this we will get update signals. Thats not usefull
        for (QWidget* w : this->findChildren<QWidget*>())
            w->blockSignals(true);

        Song* s = selection->first()->song();
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
        con = connect(s,static_cast<void (Song::*)(int,int)>(&Song::playingSylabel),this,&SongInfo::highlightText);
        for (QWidget* w : findChildren<QWidget*>())
            w->blockSignals(false);
    }

    //TODO: Support mass edit!
}

void SongInfo::on_title_textChanged(const QString &arg1)
{
    //This function only makes sense for single songs!
    assert(selection->size() == 1);
    Song* s = selection->first()->song();
    if (!s->updateTag("TITLE",arg1)) {
        //TODO
        qWarning() << "Updating title failed!";
    }
}

void SongInfo::highlightText(int from, int to) {
    QTextCursor c = ui->rawLyrics->textCursor();
    if (c.anchor() == from && c.position() == to) return;
//    qWarning()  << "Cursor -> " << from << "-" << to;
    c.setPosition(from);
    c.setPosition(to,QTextCursor::KeepAnchor);
    ui->rawLyrics->setTextCursor(c);
}

void SongInfo::on_artist_textChanged(const QString &arg1)
{
    for (SongFrame* sf : *selection) {
        Song* s = sf->song();
        if (!s->updateTag("ARTIST",arg1)) {
            //TODO
            qWarning() << "Updating title failed!";
        }
    }
}


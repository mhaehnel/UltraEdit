#include "songinfo.h"
#include "ui_songinfo.h"
#include <cassert>
#include <QDebug>

SongInfo::SongInfo(QWidget *parent) :
    QWidget(parent), ui(new Ui::SongInfo)
{
    ui->setupUi(this);
    connect(ui->notes,&NoteWidget::play,this,&SongInfo::play);
    connect(ui->notes,&NoteWidget::pause,this,&SongInfo::pause);
    connect(ui->notes,&NoteWidget::seek,&midiPlayer, &MidiPlayer::seek);
    connect(ui->notes,&NoteWidget::seek,this,&SongInfo::seek);

    connect(ui->nextLine,&QPushButton::clicked,[this] {
       ui->notes->goToLine(ui->notes->line()+1);
    });

    connect(ui->prevLine,&QPushButton::clicked,[this] {
       ui->notes->goToLine(ui->notes->line()-1);
    });

    connect(ui->firstLine,&QPushButton::clicked,[this] {
       ui->notes->goToLine(0);
    });

    connect(ui->lastLine,&QPushButton::clicked,[this] {
       ui->notes->goToLine(ui->notes->lines()-1);
    });

    connect(ui->notes,&NoteWidget::lineChanged,[this] (int line) {
        ui->curLine->setText(QString::number(line));
    });

    connect(ui->curLine,&QLineEdit::editingFinished, [this] {
        ui->notes->goToLine(ui->curLine->text().toInt());
    });

    connect(ui->notes,&NoteWidget::lineCount,[this] (int count) {
        ui->maxLine->setText(QString::number(count));
    });

    videoPlayer.setVideoOutput(ui->videoWidget);
    videoPlayer.setMuted(ui->muteVideo);
    //ui->videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
}

SongInfo::~SongInfo()
{
    delete ui;
}

void SongInfo::setMidiPort(QString port) {
    midiPlayer.connect(port);
}

QStringList SongInfo::getMidiPorts() {
    return midiPlayer.getPorts();
}

void SongInfo::setSelection(QList<SongFrame*>* selected) {
    selection = selected;
    selectionChanged();
}

void SongInfo::selectionChanged() {
    bool singleSelect = (selection->size() == 1);

    ui->filesGroup->setEnabled(singleSelect);
    ui->settingsGroup->setEnabled(singleSelect);
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

    for (auto i : {conSylText, conSylLine, conSyl})
        this->disconnect(i);

    if (selection->size() == 1) { //TODO: Support mass edit
        Song* s = selection->first()->song;
        ui->notes->setSong(s);
        midiPlayer.setSong(s);
        if (s->hasVideo())
            videoPlayer.setMedia(QUrl::fromLocalFile(s->vid().canonicalFilePath()));
        else
            videoPlayer.setMedia(QMediaContent());
        conSylText = connect(s,static_cast<void (Song::*)(int,int)>(&Song::playingSylabel),this,&SongInfo::highlightText);
        conSyl = connect(s,static_cast<void (Song::*)(Sylabel*)>(&Song::playingSylabel),ui->notes,&NoteWidget::setCurrentNote);
        conSylLine = connect(s,&Song::lineChanged,ui->notes, &NoteWidget::setLine);
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
        ui->rawData->setText(s->rawData());
        for (QWidget* w : findChildren<QWidget*>())
            w->blockSignals(false);
    }
    //TODO: Support mass edit!
}

void SongInfo::on_title_textChanged(const QString &arg1)
{
    Song* s = selection->first()->song;
    if (!s->updateTag("TITLE",arg1))
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
        if (!sf->song->updateTag("ARTIST",arg1))
            qWarning() << "Updating artist failed!";
    }
}

void SongInfo::seekTo(quint64 time) {
    midiPlayer.seek(time);
    videoPlayer.setPosition(time);
}

void SongInfo::pausePlayback() {
    if (ui->playNotes->isChecked())
        midiPlayer.stop();
    videoPlayer.stop();
}

void SongInfo::startPlayback() {
    if (ui->playNotes->isChecked())
        midiPlayer.play();
    videoPlayer.play();
}

void SongInfo::on_playNotes_toggled(bool checked)
{
    if (!checked) {
        midiPlayer.stop();
    } else {
        emit pause();
        emit play();
    }
}

void SongInfo::on_muteVideo_toggled(bool checked)
{
    videoPlayer.setMuted(checked);
}

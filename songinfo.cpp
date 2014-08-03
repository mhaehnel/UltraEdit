#include "songinfo.h"
#include "ui_songinfo.h"
#include <cassert>
#include <QDebug>

SongInfo::SongInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SongInfo)
{
    ui->setupUi(this);

    connect(ui->notes,&NoteWidget::play,[this] {
        if (ui->playMP3->isChecked()) emit play();
        if (ui->playNotes->isChecked()) midiPlayer.play();
    });

    connect(ui->notes,&NoteWidget::pause,[this] {
        if (ui->playMP3->isChecked()) emit pause();
        if (ui->playNotes->isChecked()) midiPlayer.stop();
    });
    //TODO: THis breaks if the notewidget edits another song than the one currently playing!
    connect(ui->notes,&NoteWidget::seek,[this] (quint64 pos) {
        if (ui->playMP3->isChecked()) emit seek(pos);
        if (ui->playNotes->isChecked()) midiPlayer.seek(pos);
    });


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
        conSylText = connect(s,static_cast<void (Song::*)(int,int)>(&Song::playingSylabel),this,&SongInfo::highlightText);
        conSyl = connect(s,static_cast<void (Song::*)(Sylabel*)>(&Song::playingSylabel),ui->notes,&NoteWidget::setCurrentNote);
        conSylLine = connect(s,&Song::lineChanged,ui->notes, &NoteWidget::setLine);
        connect(&midiPlayer,&MidiPlayer::positionChanged,[this,s] (quint64 pos) {
            if (!ui->playMP3->isChecked())
                s->playing(pos);
        });
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


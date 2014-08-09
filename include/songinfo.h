#ifndef SONGINFO_H
#define SONGINFO_H

#include "songframe.h"
#include "midiplayer.h"
#include <QWidget>
#include <metronome.h>
#include <QMediaPlayer>

namespace Ui {
class SongInfo;
}

class SongInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SongInfo(QWidget *parent = 0);
    QStringList getMidiPorts();
    ~SongInfo();

public slots:
    void setSelection(QList<SongFrame*> *selected);
    void selectionUpdated();
    void selectionChanged();
    void pausePlayback();
    void startPlayback();
    void seekTo(quint64 time);
    void setMidiPort(QString port);

private slots:
    void on_title_textChanged(const QString &arg1);
    void on_artist_textChanged(const QString &arg1);
    void highlightText(int from, int to);

    void on_playNotes_toggled(bool checked);
    void on_playMetronome_toggled(bool checked);

    void on_muteVideo_toggled(bool checked);

signals:
    void seek(quint64 pos);
    void play();
    void pause();

private:
    Ui::SongInfo *ui;
    MidiPlayer midiPlayer;
    Metronome metronome;
    QMediaPlayer videoPlayer;
    QMetaObject::Connection conSylText;
    QMetaObject::Connection conSylLine;
    QMetaObject::Connection conSyl;
    QList<SongFrame*> *selection;
};

#endif // SONGINFO_H

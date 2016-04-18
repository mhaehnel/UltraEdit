#pragma once
#include "songframe.h"
#include "midiplayer.h"
#include <QWidget>
#include <QMediaPlayer>
#include <memory>

namespace Ui {
class SongInfo;
}

class SongInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SongInfo(QWidget *parent = 0);
    QStringList getMidiPorts();
    virtual ~SongInfo();

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

    void on_muteVideo_toggled(bool checked);

signals:
    void seek(quint64 pos);
    void play();
    void pause();

private:
    std::unique_ptr<Ui::SongInfo> ui;
    MidiPlayer midiPlayer;
    QMediaPlayer videoPlayer;
    QMetaObject::Connection conSylText,conSylLine,conSyl,conUpdate;
    QList<SongFrame*> *selection;
};

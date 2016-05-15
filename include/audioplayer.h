#pragma once
#include <QFrame>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <song.h>
#include <drumstick.h>
#include <midiplayer.h>
#include <memory>

namespace Ui {
class AudioPlayer;
}

class AudioPlayer : public QFrame
{
    Q_OBJECT

public:
    explicit AudioPlayer(QWidget *parent = 0);
    virtual ~AudioPlayer();
    QSize sizeHint() const;
    MidiPlayer midi;

signals:
    void started();
    void stopped();
    void paused();
    void seeking(quint64 pos);

public slots:
    void setSong(Song* song);
    void seek(quint64 pos);
    void stop();
    void play();
    void pause();
    void on_playNotes_toggled(bool toggled);

private slots:
    void updateSongData();

private:
    std::unique_ptr<Ui::AudioPlayer> ui;

    Song* _song; //TODO: Ownership?

    //MP3 Player
    QMediaPlayer player;
    QMediaPlaylist pl;
};

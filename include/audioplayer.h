#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QFrame>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <song.h>

namespace Ui {
class AudioPlayer;
}

class AudioPlayer : public QFrame
{
    Q_OBJECT

public:
    explicit AudioPlayer(QWidget *parent = 0);
    ~AudioPlayer();

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

private slots:
    void updateSongData();

private:
    Song* _song;
    Ui::AudioPlayer *ui;
    QMediaPlayer player;
    QMediaPlaylist pl;
};

#endif // AUDIOPLAYER_H
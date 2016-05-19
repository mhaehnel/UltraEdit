#pragma once
#include <QFrame>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <song.h>
#include <midiplayer.h>
#include <memory>
#include <audiotrace.h>

namespace Ui {
class MediaPlayer;
}

class MediaPlayer : public QFrame
{
    Q_OBJECT

public:
    static const MediaPlayer *instance;
    explicit MediaPlayer(QWidget *parent = 0);
    virtual ~MediaPlayer();
    QSize sizeHint() const;
    void setVideoOutput(QVideoWidget* wv);
    void connectMidiPort(QString port);
    QStringList midiPorts();
    void renderWaveForm(QPixmap& target, quint64 start, quint64 end) const;

public slots:
    void setSong(Song* song);
    void seek(quint64 pos);
    void stop();
    void play();
    void pause();
    void on_playNotes_toggled(bool toggled);
    void on_playVidAudio_toggled(bool toggled);
    void on_repeatLine_toggled(bool toggled);
    void on_audioGap_valueChanged(int value);
    void on_videoGap_valueChanged(int value);

private slots:
    void updateSongData();
    void updateInfos(qint64 pos) const;

    void on_firstLine_clicked();
    void on_prevLine_clicked();
    void on_nextLine_clicked();
    void on_lastLine_clicked();

private:
    std::unique_ptr<Ui::MediaPlayer> ui;

    Song* _song; //TODO: Ownership?

    int repeatLine = 0;

    AudioTrace* MP3trace = nullptr;
    AudioTrace* VidTrace = nullptr;

    //Players
    MidiPlayer midi;
    QMediaPlayer videoPlayer;
    QMediaPlayer player;
    QMediaPlaylist pl;
};

#include "audioplayer.h"
#include "ui_audioplayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QWidget *parent) :
    QFrame(parent), _song(nullptr),
    ui(new Ui::AudioPlayer), pl(&player)
{
    ui->setupUi(this);
    ui->songTimePassed->display("00:00");
    ui->songTimeRemaining->display("00:00");
    connect(ui->songPos,&QSlider::valueChanged,&player,&QMediaPlayer::setPosition);
    connect(ui->volume,&QSlider::valueChanged,&player,&QMediaPlayer::setVolume);
    connect(ui->mute,&QPushButton::clicked,&player,&QMediaPlayer::setMuted);
    connect(ui->mute,&QPushButton::clicked,[this](bool checked) {
        if (checked) {
            ui->mute->setIcon(QIcon::fromTheme("audio-volume-muted"));
        } else {
            ui->mute->setIcon(QIcon::fromTheme("audio-volume-high"));
        }
    });
    connect(ui->songPause,&QPushButton::clicked,[this] {
        switch (player.state()) {
            case QMediaPlayer::PausedState:
                player.play();
                break;
            case QMediaPlayer::StoppedState:
            case QMediaPlayer::PlayingState:
                player.pause();
                break;
        }});
    connect(&player,&QMediaPlayer::positionChanged,[this] (qint64 pos) {
        bool o = ui->songPos->blockSignals(true); //Needed to avoid feedback
        ui->songPos->setValue(pos);
        ui->songPos->blockSignals(o);
        pos /= 1000; //to seconds
        qint64 rem = player.duration()/1000 - pos;
        ui->songTimePassed->display(QString("%1:%2").arg(pos/60,2,10,QLatin1Char('0')).arg(pos%60,2,10,QLatin1Char('0')));
        ui->songTimeRemaining->display(QString("%1:%2").arg(rem/60,2,10,QLatin1Char('0')).arg(rem%60,2,10,QLatin1Char('0')));
    });
    connect(&player,&QMediaPlayer::durationChanged,[this] (qint64 dur) { ui->songPos->setRange(0,dur); });
}

AudioPlayer::~AudioPlayer()
{
    delete ui;
}

void AudioPlayer::seek(quint64 pos) {
    player.setPosition(pos);
}

void AudioPlayer::updateSongData() {
    ui->cover->setPixmap(_song->cover());
    ui->playerTitle->setText(QString("<HTML><H1><CENTER>%1 - %2 </CENTER></H1></HTML>").arg(_song->artist(),_song->title()));
}

void AudioPlayer::playSong(Song *song) {
    if (_song != nullptr) {
        disconnect(_song,&Song::updated,this,&AudioPlayer::updateSongData);
        disconnect(&player,&QMediaPlayer::positionChanged,_song,&Song::playing);
    }
    _song = song;
    if (!song->mp3().exists()) return;
    updateSongData();
    connect(song,&Song::updated,this,&AudioPlayer::updateSongData);
    connect(&player,&QMediaPlayer::positionChanged,song,&Song::playing);
    player.stop();\
    player.setMedia(QUrl::fromLocalFile(song->mp3().canonicalFilePath()));
    player.setNotifyInterval(15/song->bpm()*1000);
    player.play();
    qWarning() << "Should play. Notify: " << 15/(song->bpm())*1000;
}

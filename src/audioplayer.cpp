#include "audioplayer.h"
#include "ui_audioplayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QWidget *parent) :
    QFrame(parent), ui(std::make_unique<Ui::AudioPlayer>()),
    _song(nullptr), pl(&player)
{
    ui->setupUi(this);
    ui->songTimePassed->display("00:00");
    ui->songTimeRemaining->display("00:00");

    connect(&player,&QMediaPlayer::stateChanged, [this] (QMediaPlayer::State state) {
       switch (state) {
           case QMediaPlayer::State::PlayingState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-pause"));
               emit seeking(player.position());
               emit started();
               break;
           case QMediaPlayer::State::StoppedState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-start"));
               emit stopped();
               break;
           case QMediaPlayer::State::PausedState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-start"));
               emit paused();
               break;
           default: break;
       }
    });
    connect(ui->songPos,&QSlider::valueChanged,&player,&QMediaPlayer::setPosition);
    connect(ui->songPos,&QSlider::valueChanged,this,&AudioPlayer::seeking);
    connect(ui->volume,&QSlider::valueChanged,&player,&QMediaPlayer::setVolume);
    connect(ui->mute,&QPushButton::clicked,&player,&QMediaPlayer::setMuted);
    connect(ui->mute,&QPushButton::clicked,[this](bool checked) {
        ui->mute->setIcon(QIcon::fromTheme(checked?"audio-volume-muted":"audio-volume-high"));
    });
    connect(ui->songPause,&QPushButton::clicked,[this] {
        switch (player.state()) {
            case QMediaPlayer::PausedState:
            case QMediaPlayer::StoppedState:
                play();
                break;
            case QMediaPlayer::PlayingState:
                pause();
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

void AudioPlayer::seek(quint64 pos) {
    player.setPosition(pos);
    midi.seek(pos);
}

AudioPlayer::~AudioPlayer() {}

void AudioPlayer::stop() {
    player.stop();
    if (ui->playNotes->isChecked()) midi.stop();
}

void AudioPlayer::play() {
    player.play();
    if (ui->playNotes->isChecked()) {
        midi.seek(player.position());
        midi.play();
    }
}

void AudioPlayer::pause() {
    player.pause();
    if (ui->playNotes->isChecked()) midi.stop();
}

QSize AudioPlayer::sizeHint() const {
    QSize sh = QWidget::sizeHint();
    sh.setHeight(106);
    return sh;
}

void AudioPlayer::updateSongData() {
    ui->cover->setPixmap(_song->cover());
    ui->playerTitle->setText(QString("<HTML><H1><CENTER>%1 - %2 </CENTER></H1></HTML>").arg(_song->artist(),_song->title()));
    midi.setTempo(_song->bpm());
}

void AudioPlayer::setSong(Song *song) {
    if (_song != nullptr) {
        disconnect(_song,&Song::updated,this,&AudioPlayer::updateSongData);
        disconnect(&player,&QMediaPlayer::positionChanged,_song,&Song::playing);
        player.stop();
    }
    _song = song;
    midi.setSong(_song);
    if (!song->mp3().exists()) return;
    updateSongData();
    connect(song,&Song::updated,this,&AudioPlayer::updateSongData);
    connect(&player,&QMediaPlayer::positionChanged,song,&Song::playing);

    player.setMedia(QUrl::fromLocalFile(song->mp3().canonicalFilePath()));
    player.setNotifyInterval(1/song->bpm()*1000); //60*bpm This should be tuneable for low powered systems
}

void AudioPlayer::on_playNotes_toggled(bool checked)
{
    if (checked) {
        midi.play();
        midi.seek(player.position());
    } else {
        midi.stop();
    }
}

#include "mediaplayer.h"
#include "ui_mediaplayer.h"
#include <QDebug>

MediaPlayer::MediaPlayer(QWidget *parent) :
    QFrame(parent), ui(std::make_unique<Ui::MediaPlayer>()),
    _song(nullptr), pl(&player)
{
    ui->setupUi(this);
    ui->songTimePassed->display("00:00");
    ui->songTimeRemaining->display("00:00");

    connect(&player,&QMediaPlayer::stateChanged, [this] (QMediaPlayer::State state) {
       switch (state) {
           case QMediaPlayer::State::PlayingState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-pause"));
               videoPlayer.setPosition(player.position());
               videoPlayer.play();
               break;
           case QMediaPlayer::State::StoppedState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-start"));
               videoPlayer.stop();
               if (ui->songRepeat->isChecked()) {
                   player.setPosition(0);
                   player.play();
               }
               break;
           case QMediaPlayer::State::PausedState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-start"));
               videoPlayer.pause();
               break;
           default: break;
       }
    });
    connect(ui->songPos,&QSlider::valueChanged,&player,&QMediaPlayer::setPosition);
    connect(ui->songPos,&QSlider::valueChanged,&videoPlayer,&QMediaPlayer::setPosition);
    connect(ui->volumeMP3,&QSlider::valueChanged,&player,&QMediaPlayer::setVolume);
    connect(ui->volumeMidi,&QSlider::valueChanged,&midi,&MidiPlayer::setVolume);
    connect(ui->playMP3,&QPushButton::clicked,[this](bool checked) {
        ui->volumeMP3->setEnabled(checked);
        player.setMuted(!checked);
    });
    connect(ui->volumeVideo,&QSlider::valueChanged,&videoPlayer,&QMediaPlayer::setVolume);
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
    connect(ui->linePos,&QSlider::valueChanged,[this] (int pos) {
       seek(_song->timeAtLine(pos));
    });
    videoPlayer.setMuted(!ui->playVidAudio->isChecked());
}

void MediaPlayer::seek(quint64 pos) {
    player.setPosition(pos);
    midi.seek(pos);
}

MediaPlayer::~MediaPlayer() {}

void MediaPlayer::stop() {
    player.stop();
    if (ui->playNotes->isChecked()) midi.stop();
}

void MediaPlayer::play() {
    player.play();
    if (ui->playNotes->isChecked()) {
        midi.seek(player.position());
        midi.play();
    }
}

void MediaPlayer::setVideoOutput(QVideoWidget* wv) {
    videoPlayer.setVideoOutput(wv);
}

void MediaPlayer::pause() {
    player.pause();
    if (ui->playNotes->isChecked()) midi.stop();
}

QSize MediaPlayer::sizeHint() const {
    QSize sh = QWidget::sizeHint();
    sh.setHeight(106);
    return sh;
}

void MediaPlayer::updateSongData() {
    ui->cover->setPixmap(_song->cover());
    ui->playerTitle->setText(QString("<HTML><H1><CENTER>%1 - %2 </CENTER></H1></HTML>").arg(_song->artist(),_song->title()));
    midi.setTempo(_song->bpm());
}

void MediaPlayer::setSong(Song *song) {
    if (_song != nullptr) {
        disconnect(_song,&Song::updated,this,&MediaPlayer::updateSongData);
        disconnect(&player,&QMediaPlayer::positionChanged,_song,&Song::playing);
        disconnect(song,&Song::lineChanged,ui->linePos,&QSlider::setValue);
        disconnect(&player,&QMediaPlayer::positionChanged,song,&Song::playing);
        player.stop();
    }
    _song = song;
    if (_song->hasVideo())
        videoPlayer.setMedia(QUrl::fromLocalFile(_song->vid().canonicalFilePath()));
    else
        videoPlayer.setMedia(QMediaContent());
    midi.setSong(_song);
    if (!song->mp3().exists()) return;
    updateSongData();
    connect(song,&Song::updated,this,&MediaPlayer::updateSongData);
    connect(song,&Song::lineChanged,ui->curLine,static_cast<void(QLCDNumber::*)(int)>(&QLCDNumber::display));
    static QMetaObject::Connection lineChange;
    QObject::disconnect(lineChange);
    lineChange = connect(song,&Song::lineChanged,[this] (int val) {
        ui->linePos->blockSignals(true);
        ui->linePos->setValue(val);
        ui->linePos->blockSignals(false);
    });
    connect(&player,&QMediaPlayer::positionChanged,song,&Song::playing);
    ui->curLine->display(1);
    ui->numLines->display(_song->lines());
    ui->linePos->setMaximum(_song->lines());
    ui->linePos->setValue(0);
    player.setMedia(QUrl::fromLocalFile(song->mp3().canonicalFilePath()));
    player.setNotifyInterval(1/song->bpm()*1000); //60*bpm This should be tuneable for low powered systems
}

void MediaPlayer::on_playNotes_toggled(bool checked)
{
    ui->volumeMidi->setEnabled(checked);
    if (checked) {
        midi.play();
        midi.seek(player.position());
    } else {
        midi.stop();
    }
}

void MediaPlayer::on_playVidAudio_toggled(bool checked)
{
    ui->volumeVideo->setEnabled(checked);
    videoPlayer.setMuted(!checked);
}

void MediaPlayer::connectMidiPort(QString port) {
    midi.connect(port);
}

QStringList MediaPlayer::midiPorts() {
    return midi.getPorts();
}

void MediaPlayer::on_firstLine_clicked() {
    seek(_song->timeAtLine(0));
}

void MediaPlayer::on_prevLine_clicked() {
    seek(_song->timeAtLine(ui->curLine->intValue()-1));
}

void MediaPlayer::on_nextLine_clicked() {
    seek(_song->timeAtLine(ui->curLine->intValue()+1));
}

void MediaPlayer::on_lastLine_clicked() {
    seek(_song->timeAtLine(_song->lines()));
}

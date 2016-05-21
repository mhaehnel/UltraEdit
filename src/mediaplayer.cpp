#include "mediaplayer.h"
#include "ui_mediaplayer.h"
#include <QDebug>
#include <actions/modifygap.h>

const MediaPlayer* MediaPlayer::instance = nullptr;

MediaPlayer::MediaPlayer(QWidget *parent) :
    QFrame(parent), ui(std::make_unique<Ui::MediaPlayer>()),
    _song(nullptr), pl(&player)
{
    ui->setupUi(this);
    ui->songTimePassed->display("00:00");
    ui->songTimeRemaining->display("00:00");

    Q_ASSERT(instance == nullptr);
    instance = this;

    connect(&player,&QMediaPlayer::stateChanged, [this] (QMediaPlayer::State state) {
       switch (state) {
           case QMediaPlayer::State::PlayingState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-pause"));
               break;
           case QMediaPlayer::State::StoppedState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-start"));
               break;
           case QMediaPlayer::State::PausedState:
               ui->songPause->setIcon(QIcon::fromTheme("media-playback-start"));
               break;
           default: break;
       }
    });
    connect(ui->songPos,&QSlider::valueChanged,&player,&QMediaPlayer::setPosition);
    connect(ui->songPos,&QSlider::valueChanged,[this] (int position){
        videoPlayer.setPosition(position+_song->videoGap());
        //TODO: midiPlayer?
    });
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
    connect(&player,&QMediaPlayer::positionChanged,this,&MediaPlayer::updateInfos);
    connect(&player,&QMediaPlayer::durationChanged,[this] (qint64 dur) { ui->songPos->setRange(0,dur); });
    connect(ui->linePos,&QSlider::valueChanged,[this] (int pos) {
       seek(_song->timeAtLine(pos));
    });
    videoPlayer.setMuted(!ui->playVidAudio->isChecked());
}

void MediaPlayer::seek(quint64 pos) {
    player.setPosition(pos);
    videoPlayer.setPosition(pos+_song->videoGap());
    midi.seek(pos);
}

MediaPlayer::~MediaPlayer() {}

void MediaPlayer::stop() {
    player.stop();
    videoPlayer.stop();
    if (ui->playNotes->isChecked()) midi.stop();
}

void MediaPlayer::play() {
    player.play();
    if (ui->playNotes->isChecked()) {
        midi.seek(player.position());
        midi.play();
    }
    if (videoPlayer.isVideoAvailable()) {
        videoPlayer.setPosition(player.position()+_song->videoGap());
        videoPlayer.play();
    }
}

void MediaPlayer::setVideoOutput(QVideoWidget* wv) {
    videoPlayer.setVideoOutput(wv);
}

void MediaPlayer::pause() {
    player.pause();
    videoPlayer.pause();
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
        stop();
    }
    _song = song;
    this->setEnabled(true);

    if (_song->hasVideo()) {
        videoPlayer.setMedia(QUrl::fromLocalFile(_song->vid().canonicalFilePath()));
        if (VidTrace != nullptr) delete VidTrace;
        ui->videoTrace->clear();
        VidTrace = new AudioTrace(_song->vid().canonicalFilePath());
        connect(VidTrace,&AudioTrace::finished,[this] {
            VidTrace->renderTrace(*(ui->videoTrace),0);
        });
    } else {
        if (VidTrace != nullptr) delete VidTrace;
        VidTrace = nullptr;
        ui->videoTrace->clear();
        videoPlayer.setMedia(QMediaContent());
    }

    if (!song->mp3().exists()) return;
    connect(song,&Song::updated,this,&MediaPlayer::updateSongData);
    connect(song,&Song::lineChanged,ui->curLine,static_cast<void(QLCDNumber::*)(int)>(&QLCDNumber::display));
    static QMetaObject::Connection lineChange;
    QObject::disconnect(lineChange);
    lineChange = connect(song,&Song::lineChanged,[this] (int val) {
        if (repeatLine != val && repeatLine != 0) {
            seek(_song->timeAtLine(repeatLine));
            return;
        }
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
    if (MP3trace != nullptr) delete MP3trace;
    ui->MP3Trace->clear();
    MP3trace = new AudioTrace(song->mp3().canonicalFilePath(),song->bpm()/song->bpmFactor(),song->gap());
    connect(MP3trace,&AudioTrace::finished,[this] {
        emit haveWaveform();
        MP3trace->renderTrace(*(ui->MP3Trace),0,true);
    });

    midi.setSong(_song);
    ui->audioGap->setValue(_song->gap());
    ui->videoGap->setValue(_song->videoGap());
    ui->bpm->setValue(_song->bpm());
    ui->videoGap->setEnabled(true); //FIXME: Why do we need those 3 lines?!
    ui->audioGap->setEnabled(true);
    ui->tuningBox->setEnabled(true);

    updateSongData();
    //This should be tuneable for low powered systems
    // we require 60000/bpm for notes but 100ms
    player.setNotifyInterval(std::min(100,static_cast<int>(60000/song->bpm())));
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
    if (repeatLine) repeatLine = 0;
}

void MediaPlayer::on_prevLine_clicked() {
    seek(_song->timeAtLine(ui->curLine->intValue()-1));
    if (repeatLine) repeatLine--;
}

void MediaPlayer::on_nextLine_clicked() {
    seek(_song->timeAtLine(ui->curLine->intValue()+1));
    if (repeatLine) repeatLine++;
}

void MediaPlayer::on_lastLine_clicked() {
    seek(_song->timeAtLine(_song->lines()));
    if (repeatLine) repeatLine = _song->lines();
}

void MediaPlayer::on_repeatLine_toggled(bool toggled) {
    repeatLine = (toggled)?ui->curLine->intValue():0;
}

void MediaPlayer::on_audioGap_valueChanged(int value) {
    if (_song->gap() != value) {
        _song->performAction(std::make_unique<Song::ModifyGap>(Song::ModifyGap::Type::Audio,value));
        midi.reschedule();
    }
    if (MP3trace != nullptr) MP3trace->updateGap(value);
    updateInfos(player.position());
}

void MediaPlayer::on_videoGap_valueChanged(int value) {
    if (_song->videoGap() != value) {
        _song->performAction(std::make_unique<Song::ModifyGap>(Song::ModifyGap::Type::Video,value));
        videoPlayer.setPosition(player.position()+_song->videoGap());
    }
    updateInfos(player.position());
}

void MediaPlayer::renderWaveForm(QPixmap &target, quint64 start, quint64 end) const {
    MP3trace->renderSection(target,start,end);
}

void MediaPlayer::updateInfos(qint64 pos) const {
    bool o = ui->songPos->blockSignals(true); //Needed to avoid feedback
    ui->songPos->setValue(pos);
    ui->songPos->blockSignals(o);
    if (MP3trace != nullptr && ui->waveForm->isChecked()) MP3trace->renderTrace(*(ui->MP3Trace),pos,true);
    if (VidTrace != nullptr && ui->waveForm->isChecked()) VidTrace->renderTrace(*(ui->videoTrace),videoPlayer.position());
    pos /= 1000; //to seconds
    qint64 rem = player.duration()/1000 - pos;
    ui->songTimePassed->display(QString("%1:%2").arg(pos/60,2,10,QLatin1Char('0')).arg(pos%60,2,10,QLatin1Char('0')));
    ui->songTimeRemaining->display(QString("%1:%2").arg(rem/60,2,10,QLatin1Char('0')).arg(rem%60,2,10,QLatin1Char('0')));
}

void MediaPlayer::on_bpm_valueChanged(double bpm) {
    if (MP3trace != nullptr) MP3trace->updateBpm(bpm);
    updateInfos(player.position());
}

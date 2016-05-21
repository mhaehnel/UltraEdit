#include "songframe.h"
#include "ui_songframe.h"
#include <QDebug>

//TODO: Credit multiplayer icon author:
//<div>Icon made by <a href="http://www.freepik.com" alt="Freepik.com" title="Freepik.com">Freepik</a> from <a href="http://www.flaticon.com/free-icon/two-persons-silhouettes_35120" title="Flaticon">www.flaticon.com</a></div>
//TODO: Credit Note icon author:
//<div>Icon made by <a href="http://www.freepik.com" alt="Freepik.com" title="Freepik.com">Freepik</a> from <a href="http://www.flaticon.com/free-icon/music-black-note-shape_26499" title="Flaticon">www.flaticon.com</a></div>
//TODO: Credit Freestyle icon author:
//<div>Icon made by <a href="http://www.freepik.com" alt="Freepik.com" title="Freepik.com">Freepik</a> from <a href="http://www.flaticon.com/free-icon/break-beat_10937" title="Flaticon">www.flaticon.com</a></div>
//TODO: Credit Movie icon author:
//<div>Icon made by <a href="http://www.freepik.com" alt="Freepik.com" title="Freepik.com">Freepik</a> from <a href="http://www.flaticon.com/free-icon/movie_31087" title="Flaticon">www.flaticon.com</a></div>
//TODO: Credit Background icon author:
//<div>Icon made by <a href="http://www.freepik.com" alt="Freepik.com" title="Freepik.com">Freepik</a> from <a href="http://www.flaticon.com/free-icon/movie_31087" title="Flaticon">www.flaticon.com</a></div>

SongFrame::SongFrame(Song *song, QWidget *parent) :
    QFrame(parent), song(song),
    ui(new Ui::songframe())
{
    ui->setupUi(this);
    connect(song,&Song::updated,this,&SongFrame::updateData);
    updateData();
}

void SongFrame::updateData() {
    ui->title->setText(QString("<HTML><H1><CENTER>%1</CENTER></H1></CENTER>").arg(song->title()));
    ui->artist->setText(QString("<HTML><H3><CENTER>%1</CENTER></H3></CENTER>").arg(song->artist()));
    ui->cover->setPixmap(song->cover());
    if (!song->hasBG()) {
        ui->background->setPixmap(QPixmap(":/images/landscape_haveNo"));
    } else if (song->missingBG()) {
        ui->background->setPixmap(QPixmap(":/images/landscape_notFound"));
    } else {
        ui->background->setPixmap(QPixmap(":/images/landscape"));
    }
    if (!song->hasVideo()) {
        ui->video->setPixmap(QPixmap(":/images/video_haveNo"));
    } else if (song->missingVideo()) {
        ui->video->setPixmap(QPixmap(":/images/video_notFound"));
    } else {
        ui->video->setPixmap(QPixmap(":/images/video"));
    }
}

void SongFrame::mousePressEvent(QMouseEvent *ev) {
    if (ev->button() == Qt::LeftButton) {
        emit clicked(this);
    }
    QFrame::mousePressEvent(ev);
}

void SongFrame::select() {
    setStyleSheet("background-color: palette(highlight);");
}

void SongFrame::deselect() {
    setStyleSheet("");
}

SongFrame::~SongFrame() {
    delete ui;
}

void SongFrame::on_playMedia_clicked()
{
    emit playSong(song);
}

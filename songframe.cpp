#include "songframe.h"
#include "ui_songframe.h"

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
    QFrame(parent), _song(song),
    ui(new Ui::songframe)
{
    ui->setupUi(this);
    ui->title->setText(ui->title->text().replace("#TITLE#",song->title()));
    ui->artist->setText(ui->artist->text().replace("#ARTIST#",song->artist()));
    ui->cover->setPixmap(song->cover());
//    setAttribute(Qt::WA_StyledBackground);
}

Song* SongFrame::song() {
    return _song;
}

const Song* SongFrame::song() const {
    return _song;
}

SongFrame::~SongFrame()
{
    delete ui;
}

#include "songwidget.h"
#include "ui_songwidget.h"

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

SongWidget::SongWidget(Song *song, QWidget *parent) :
    QWidget(parent), _song(song),
    ui(new Ui::SongWidget)
{
    ui->setupUi(this);
    ui->title->setText(ui->title->text().replace("#TITLE#",song->title()));
    ui->artist->setText(ui->artist->text().replace("#ARTIST#",song->artist()));
    ui->cover->setPixmap(song->cover());
}

Song* SongWidget::song() {
    return _song;
}

SongWidget::~SongWidget()
{
    delete ui;
}

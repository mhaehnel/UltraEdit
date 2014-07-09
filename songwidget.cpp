#include "songwidget.h"
#include "ui_songwidget.h"

SongWidget::SongWidget(const Song& song, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SongWidget)
{
    ui->setupUi(this);
    ui->title->setText(ui->title->text().replace("#TITLE#",song.title()));
    ui->artist->setText(ui->artist->text().replace("#ARTIST#",song.artist()));
}

SongWidget::~SongWidget()
{
    delete ui;
}

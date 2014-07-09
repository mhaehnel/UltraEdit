#include "songinfo.h"
#include "ui_songinfo.h"

SongInfo::SongInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SongInfo)
{
    ui->setupUi(this);
}

SongInfo::~SongInfo()
{
    delete ui;
}

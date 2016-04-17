#include "songgroup.h"
#include "ui_songgroup.h"

SongGroup::SongGroup(QString groupName, QWidget *parent) :
    QFrame(parent),
    ui(std::make_unique<Ui::SongGroup>())
{
    ui->setupUi(this);
    ui->groupName->setText(QString("<HTML><H1>%1</HTML></H1>").arg(groupName));
}

QLayout& SongGroup::layout() {
    return *ui->songs;
}

SongGroup::~SongGroup() {}

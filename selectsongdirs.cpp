#include <QDebug>
#include "selectsongdirs.h"
#include <QFileSystemModel>
#include "ui_selectsongdirs.h"

SelectSongDirs::SelectSongDirs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectSongDirs), _changed(false)
{
    ui->setupUi(this);
    QFileSystemModel* fm = new QFileSystemModel();
    fm->setFilter(QDir::AllDirs | QDir::Dirs | QDir::NoDotAndDotDot);
    fm->setRootPath(QDir::homePath());
    ui->dirTree->setModel(fm);
    ui->dirTree->hideColumn(1);
    ui->dirTree->hideColumn(2);
    ui->dirTree->hideColumn(3);
    ui->dirTree->hideColumn(4);
    ui->dirTree->setRootIndex(fm->index(QDir::rootPath()));
}

SelectSongDirs::~SelectSongDirs()
{
    delete ui;
}

void SelectSongDirs::on_pushButton_clicked()
{
    const QFileSystemModel *m = dynamic_cast<const QFileSystemModel*>(ui->dirTree->model());
    QDir dir(m->filePath(ui->dirTree->currentIndex()));
    if (ui->selectedDirs->findItems(dir.canonicalPath(), Qt::MatchExactly).empty()) {
        ui->selectedDirs->addItem(dir.canonicalPath());
        _changed = true;
    }
}

void SelectSongDirs::on_pushButton_2_clicked()
{
    if (ui->selectedDirs->count() != 0) _changed = true;
    for (auto item : ui->selectedDirs->selectedItems()) delete item;
}

QStringList SelectSongDirs::getPaths() const {
    QStringList res;
    for (int i = 0; i < ui->selectedDirs->count(); i++)
        res.append(ui->selectedDirs->item(i)->text());
    return res;
}

void SelectSongDirs::setPaths(QStringList paths) {
    for (QString p : paths) {
        QDir dir(p);
        if (!dir.exists()) continue;
        if (dir.canonicalPath() != p) continue;
        ui->selectedDirs->addItem(p);
    }
}

bool SelectSongDirs::changed() const {
    return _changed;
}

#include "songimportdialog.h"
#include "ui_songimportdialog.h"
#include <mainwindow.h>

SongImportDialog::SongImportDialog(Song* song, MainWindow *parent) :
    QDialog(parent), song_(song),
    ui(new Ui::SongImportDialog)
{
    ui->setupUi(this);
    importBtn = ui->buttonBox->addButton("Import",QDialogButtonBox::AcceptRole);
    ui->noMP3->setVisible(!song_->mp3().exists());
    ui->noVideo->setVisible(!song_->vid().fileName().isEmpty() && !song_->vid().exists());
    ui->noCover->setVisible(!song_->cov().fileName().isEmpty() && !song_->cov().exists());
    ui->noCover->setVisible(!song_->bg().fileName().isEmpty() && !song_->bg().exists());
    ui->cover->setPixmap(song->cover());
    ui->title->setText(QString("<HTML><H1><CENTER>%1 - %2 </CENTER></H1></HTML>").arg(song_->artist(),song_->title()));

}

SongImportDialog::~SongImportDialog()
{
    delete ui;
}

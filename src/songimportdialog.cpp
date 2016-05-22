#include "songimportdialog.h"
#include "ui_songimportdialog.h"
#include <actions/transfertocollection.h>
#include <mainwindow.h>

SongImportDialog::SongImportDialog(const QList<Collection>& cols, Song* song, MainWindow *parent) :
    QDialog(parent), song_(song), cols_(cols),
    ui(new Ui::SongImportDialog)
{
    ui->setupUi(this);
    importBtn = ui->buttonBox->addButton("Import",QDialogButtonBox::AcceptRole);
    connect(importBtn,&QPushButton::clicked,this,&SongImportDialog::import);
    ui->noMP3->setVisible(!song_->mp3().exists());
    ui->noVideo->setVisible(!song_->vid().fileName().isEmpty() && !song_->vid().exists());
    ui->noCover->setVisible(!song_->cov().fileName().isEmpty() && !song_->cov().exists());
    ui->noCover->setVisible(!song_->bg().fileName().isEmpty() && !song_->bg().exists());
    ui->cover->setPixmap(song->cover());
    ui->title->setText(QString("<HTML><H1><CENTER>%1 - %2 </CENTER></H1></HTML>").arg(song_->artist(),song_->title()));
    for (const Collection& c : cols_) {
        ui->collection->addItem(c.name());
    }
    ui->collection->setCurrentIndex(cols_.size()-1);
}

void SongImportDialog::import() {
    if (!song_->performAction(std::make_unique<Song::TransferToCollection>(
                             &cols_.at(ui->collection->currentIndex()),
                             (ui->moveImport->isChecked())?
                                 Song::TransferToCollection::Type::Move:
                                 Song::TransferToCollection::Type::Copy
                         )))
        reject();
}

SongImportDialog::~SongImportDialog()
{
    delete ui;
}

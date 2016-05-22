#include "songimportdialog.h"
#include "ui_songimportdialog.h"
#include <actions/transfertocollection.h>
#include <mainwindow.h>
#include <QFileDialog>
#include <QMenu>

//This suggests alternatives for not-found files. Can be extended to be
//more intelligent
static QStringList alternatives(const QFileInfo &orig) {
    QStringList res;
    for (const QString& e : orig.dir().entryList()) {
        if (orig.fileName().compare(e,Qt::CaseInsensitive) == 0)
            res << e;
    }
    return res;
}

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
    ui->noBackground->setVisible(!song_->bg().fileName().isEmpty() && !song_->bg().exists());

    if (!song_->mp3().exists())
        createAlt(song_->_mp3,ui->noMP3,"MP3 files (*.mp3)");
    if (!song->vid().fileName().isEmpty() && !song_->vid().exists())
        createAlt(song_->_vid,ui->noVideo,QString());
    if (!song->cov().fileName().isEmpty() && !song_->cov().exists())
        createAlt(song_->_cov,ui->noCover,QString());
    if (!song->bg().fileName().isEmpty() && !song_->bg().exists())
        createAlt(song_->_bg,ui->noBackground,QString());

    ui->cover->setPixmap(song->cover());
    ui->title->setText(QString("<HTML><H1><CENTER>%1 - %2 </CENTER></H1></HTML>").arg(song_->artist(),song_->title()));
    for (const Collection& c : cols_) {
        ui->collection->addItem(c.name());
    }
    ui->collection->setCurrentIndex(cols_.size()-1);
}

void SongImportDialog::createAlt(QFileInfo &fi,QPushButton* tb,QString filter) {
    QStringList alt = alternatives(fi);
    auto selectFile = [this,&fi,tb,filter] {
        QString file = QFileDialog::getOpenFileName(this,"Select File",fi.dir().absolutePath(),filter);
        qDebug() << "File is now: " << fi.absoluteFilePath();
        if (!file.isEmpty()) {
            fi.setFile(fi.dir(),file);
            tb->hide();
        }
    };
    qDebug() << "Alt: " << alt;
    if (!alt.empty()) {
        QMenu *menu = new QMenu(tb);
        for (const QString& s : alt) {
            menu->addAction(s,[this,&fi,tb,s] {
                fi.setFile(fi.dir(),s);
                qDebug() << "File is now: " << fi.absoluteFilePath();
                tb->hide();
            });
        }
        menu->addSeparator();
        menu->addAction("Select File ...",selectFile);
        tb->setMenu(menu);
    } else {
        connect(tb,&QPushButton::clicked,selectFile);
    }
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

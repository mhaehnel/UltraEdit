#include "songimportdialog.h"
#include "ui_songimportdialog.h"
#include <actions/transfertocollection.h>
#include <mainwindow.h>
#include <QFileDialog>
#include <QMenu>
#include <QLineEdit>

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

SongImportDialog::SongImportDialog(const QList<Collection*>& cols, Song* song, MainWindow *parent) :
    QDialog(parent), song_(song), cols_(cols),
    ui(new Ui::SongImportDialog)
{
    ui->setupUi(this);
    importBtn = ui->buttonBox->addButton("Import",QDialogButtonBox::AcceptRole);
    connect(importBtn,&QPushButton::clicked,this,&SongImportDialog::import);

    if (!song_->mp3().exists())
        createAlt(song_->_mp3,"MP3","MP3 files (*.mp3)");
    if (!song_->vid().fileName().isEmpty() && !song_->vid().exists())
        createAlt(song_->_vid,"Video",QString());
    if (!song_->cov().fileName().isEmpty() && !song_->cov().exists())
        createAlt(song_->_cov,"Cover",QString());
    if (!song_->bg().fileName().isEmpty() && !song_->bg().exists())
        createAlt(song_->_bg,"Background",QString());

    ui->cover->setPixmap(song->cover());
    ui->title->setText(QString("<HTML><H1><CENTER>%1 - %2 </CENTER></H1></HTML>").arg(song_->artist(),song_->title()));
    for (const Collection* c : cols_) {
        ui->collection->addItem(c->name());
    }
    connect(ui->collection,&QComboBox::currentTextChanged,[this] {
        checkDupes();
        checkTags();
    });
    checkDupes();
    checkTags();
    ui->collection->setCurrentIndex(cols_.size()-1);
}

//If the pattern tags of the collection are the same we bail, because
//then it suggests this is the same song
void SongImportDialog::checkDupes() {
    bool dupe = isDupe();
    importBtn->setDisabled(dupe);
    ui->dupe->setVisible(dupe);
}

void SongImportDialog::checkTags() {
    const Collection* c = cols_.at(ui->collection->currentIndex());
    for (const QString& tag : c->significantTags()) {
        if (song_->tag(tag).isEmpty()) {
            qDebug() << "Song is missing tag: "+tag;
            QBoxLayout* bl = new QBoxLayout(QBoxLayout::LeftToRight);
            bl->addWidget(new QLabel("Set empty required tag '"+tag+"' to",this));
            QLineEdit* le = new QLineEdit(this);
            bl->addWidget(le);
            connect(le,&QLineEdit::textChanged,[this,tag] (const QString& text) {
                song_->setTag(tag,text);
            });
            QMetaObject::Connection* conn = new QMetaObject::Connection();
            conn->operator =(connect(ui->collection,&QComboBox::currentTextChanged,[bl,this,conn] (const QString&){
                QObject::disconnect(*conn);
                delete conn;
                ui->infos->removeItem(bl);
                while (bl->count() != 0) {
                    auto item = bl->takeAt(0);
                    if (item->layout() != 0) bl->removeItem(item);
                    if (item->widget() != 0) delete item->widget();
                    delete item;
                }
                delete bl;
            }));
            ui->infos->addLayout(bl);
        }
    }
}

bool SongImportDialog::isDupe() {
    Collection* c = cols_.at(ui->collection->currentIndex());
    for (const Song* s : c->songs()) {
        qDebug() << "Checking" << s->title() << "by" << s->artist();
        const QStringList& tags = c->significantTags();
        if (std::accumulate(tags.begin(),tags.end(),true,
                            [this,s] (const bool& r, const QString& t) -> bool {
                                return r && (s->tag(t) == song_->tag(t));
                            }) == true) return true;
    }
    return false;
}

void SongImportDialog::createAlt(QFileInfo &fi,QString what,QString filter) {
    QPushButton *tb = new QPushButton(QIcon::fromTheme("dialog-warning"),what+" not found ...",this);
    QStringList alt = alternatives(fi);
    auto selectFile = [this,&fi,tb,filter] {
        QString file = QFileDialog::getOpenFileName(this,"Select File",fi.dir().absolutePath(),filter);
        if (!file.isEmpty()) {
            fi.setFile(fi.dir(),file);
            delete tb;
        }
    };
    if (!alt.empty()) {
        QMenu *menu = new QMenu(tb);
        for (const QString& s : alt) {
            menu->addAction(s,[this,&fi,tb,s] {
                fi.setFile(fi.dir(),s);
                tb->hide();
            });
        }
        menu->addSeparator();
        menu->addAction("Select File ...",selectFile);
        tb->setMenu(menu);
    } else {
        connect(tb,&QPushButton::clicked,selectFile);
    }
    ui->infos->addWidget(tb);
}


void SongImportDialog::import() {
    if (!song_->performAction(std::make_unique<Song::TransferToCollection>(
                             cols_.at(ui->collection->currentIndex()),
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

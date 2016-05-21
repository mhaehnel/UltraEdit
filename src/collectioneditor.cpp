#include "collectioneditor.h"
#include "ui_collectioneditor.h"
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

//TODO: Names, rules or paths may not contain the pipe character!
CollectionEditor::CollectionEditor(QList<Collection> cols, QWidget *parent) :
    QDialog(parent), cols_(cols),
    ui(new Ui::CollectionEditor()), _changed(false)
{
    ui->setupUi(this);
    connect(ui->name,&QLineEdit::textChanged,this,&CollectionEditor::checkButton);
    connect(ui->basePath,&QLineEdit::textChanged,this,&CollectionEditor::checkButton);
    connect(ui->pathRule,&QLineEdit::textChanged,this,&CollectionEditor::checkButton);
    updateCollections();
}

CollectionEditor::~CollectionEditor() {
    delete ui;
}

void CollectionEditor::updateCollections() {
    ui->collectionList->clear();
    for (const Collection& c : cols_) {
        ui->collectionList->addItem(QString("%1: [%2] %3").
                                    arg(c.name(),c.basePath(),c.pathRule()));
    }
}

void CollectionEditor::on_create_clicked()
{
    _changed = true;
    cols_.push_back(Collection(ui->name->text(),ui->basePath->text(),
                               ui->pathRule->text()));
    updateCollections();
}

QList<Collection> CollectionEditor::getPaths() const {
    return cols_;
}

bool CollectionEditor::changed() const {
    return _changed;
}

void CollectionEditor::on_toolButton_clicked()
{
    auto p = QFileDialog::getExistingDirectory(this,"Base directory for collection",
                                    QSettings().value("collectionBase").toString());
    if (!p.isEmpty()) QSettings().setValue("collectionBase",p);
    ui->basePath->setText(p);
}

void CollectionEditor::checkButton(const QString &)
{
    ui->create->setEnabled(!ui->name->text().contains('|') &&
                                 !ui->basePath->text().contains('|') &&
                                 !ui->pathRule->text().contains('|'));
}

void CollectionEditor::on_remove_clicked()
{
    _changed = true;
    QListWidgetItem* i = ui->collectionList->selectedItems().first();
    cols_.removeAt(ui->collectionList->row(i));
    updateCollections();
}

void CollectionEditor::on_collectionList_itemSelectionChanged()
{
    ui->remove->setEnabled(ui->collectionList->selectedItems().size() != 0);
}

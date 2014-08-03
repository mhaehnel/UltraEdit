#include "pathinstanceselector.h"
#include <QRadioButton>
#include <cassert>
#include "ui_pathinstanceselector.h"

PathInstanceSelector::PathInstanceSelector(QStringList& choice, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PathInstanceSelector)
{
    ui->setupUi(this);
    assert(choice.size() != 0);
    if (choice.size() > 5) {
        for (QString s : choice) {
            ui->manyChoices->addItem(s);
        }
        connect(ui->manyChoices,&QComboBox::currentTextChanged,[this] (const QString& text) {
            selectedText = text;
        });
        ui->groupBox->setVisible(false);
        ui->manyChoices->setCurrentIndex(0);
    } else {
        bool first = true;
        for (QString s : choice) {
            QRadioButton* rb = new QRadioButton(s);
            ui->fewChoices->addWidget(rb);
            if (first) {
                first = false;
                rb->setChecked(true);
            }
            connect(rb,&QRadioButton::toggled,[this,rb] (bool checked) {
                if (checked)
                    selectedText = rb->text();
            });
        }
        ui->manyChoices->setVisible(false);
    }
}

PathInstanceSelector::~PathInstanceSelector()
{
    QLayoutItem* li;
    while ( (li = ui->fewChoices->itemAt(0)) != nullptr) delete li->widget();
    delete ui;
}

void PathInstanceSelector::on_buttonBox_accepted()
{
    close();
}

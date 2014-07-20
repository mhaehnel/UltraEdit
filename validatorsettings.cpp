#include <QDebug>
#include "validatorsettings.h"
#include "validator.h"
#include "ui_validatorsettings.h"

ValidatorSettings::ValidatorSettings(QSettings& settings, QList<Song*> *songs, QWidget *parent) :
    QDialog(parent), s(settings), songs(songs),
    ui(new Ui::ValidatorSettings)
{
    ui->setupUi(this);
    sample = new SongFrame(songs->at(qrand()%songs->size()));
    ui->songCheck->layout()->addWidget(sample);
    ui->dirFormat->setText(s.value("dirFormat","${artist:start}/${artist} - ${title}/${artist} - ${title}!{COV:_COV}!{BG:_BG}.${suffix}").toString());
}

ValidatorSettings::~ValidatorSettings()
{
    qWarning() << "Closing!";
    s.setValue("dirFormat",ui->dirFormat->text());
    delete ui;
}

void ValidatorSettings::on_pushButton_clicked()
{
    delete sample;
    sample = new SongFrame(songs->at(qrand()%songs->size()));
    ui->songCheck->layout()->addWidget(sample);
    on_samplePath_textChanged(ui->samplePath->text());
}

void ValidatorSettings::on_samplePath_textChanged(const QString &arg1)
{
    Validator v(s,Validator::Mode::ReadOnly, sample->song()->basePath());
    Validator::Type vt;
    switch (ui->comboBox->currentIndex()) {
        case 0: vt = Validator::Type::MP3; break;
        case 1: vt = Validator::Type::TXT; break;
        case 2: vt = Validator::Type::COV; break;
        case 3: vt = Validator::Type::BG; break;
        case 4: vt = Validator::Type::VID; break;
        case 5: vt = Validator::Type::ALL; break;
    }
    if (ui->realFile->isChecked()) {
        if (!v.good() || !v.validate(sample->song(),vt)) {
            ui->samplePath->setStyleSheet("background-color: red");
        } else {
            ui->samplePath->setStyleSheet("background-color: green");
        }
    } else {
        if (vt == Validator::Type::ALL)
            qWarning() << "Checking against all does not make a lot of sense for a single filename!";
        if (!v.good() || !v.validatePath(vt,sample->song(),ui->samplePath->text())) {
            ui->samplePath->setStyleSheet("background-color: red");
        } else {
            ui->samplePath->setStyleSheet("background-color: green");
        }
    }
}

void ValidatorSettings::on_dirFormat_textChanged(const QString &arg1)
{
    //TODO: Save last valid string for abort case
    s.setValue("dirFormat",ui->dirFormat->text());
    Validator v(s,Validator::Mode::ReadOnly,sample->song()->basePath());
    if (!v.good()) {
        ui->dirFormat->setStyleSheet("background-color: red");
    } else {
        ui->dirFormat->setStyleSheet("");
    }
    //Force recheck!
    on_samplePath_textChanged(ui->samplePath->text());
}

void ValidatorSettings::on_realFile_toggled(bool checked)
{
    ui->samplePath->setDisabled(checked);
    if (checked) {
        on_samplePath_textChanged(ui->samplePath->text());
    }
}

void ValidatorSettings::on_comboBox_currentIndexChanged(int index)
{
    on_samplePath_textChanged(ui->samplePath->text());
}

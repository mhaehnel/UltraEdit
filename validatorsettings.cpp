#include "validatorsettings.h"
#include "ui_validatorsettings.h"

ValidatorSettings::ValidatorSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ValidatorSettings)
{
    ui->setupUi(this);
}

ValidatorSettings::~ValidatorSettings()
{
    delete ui;
}

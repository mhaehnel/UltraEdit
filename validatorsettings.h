#ifndef VALIDATORSETTINGS_H
#define VALIDATORSETTINGS_H

#include <QWidget>

namespace Ui {
class ValidatorSettings;
}

class ValidatorSettings : public QWidget
{
    Q_OBJECT

public:
    explicit ValidatorSettings(QWidget *parent = 0);
    ~ValidatorSettings();

private:
    Ui::ValidatorSettings *ui;
};

#endif // VALIDATORSETTINGS_H

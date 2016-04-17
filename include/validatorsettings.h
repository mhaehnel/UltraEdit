#ifndef VALIDATORSETTINGS_H
#define VALIDATORSETTINGS_H

#include <QDialog>
#include <QSettings>
#include "songframe.h"
#include <memory>

namespace Ui {
class ValidatorSettings;
}

class ValidatorSettings : public QDialog
{
    Q_OBJECT

public:
    explicit ValidatorSettings(QSettings& settings, QList<Song*> *songs, QWidget *parent = 0);
    ~ValidatorSettings();
    QSettings& s;

private slots:
    void on_pushButton_clicked();

    void on_samplePath_textChanged(const QString &arg1);

    void on_dirFormat_textChanged(const QString &arg1);

    void on_realFile_toggled(bool checked);

    void on_comboBox_currentIndexChanged(int);

private:
    std::unique_ptr<Ui::ValidatorSettings> ui;
    SongFrame *sample;
    QList<Song*> *songs;
};

#endif // VALIDATORSETTINGS_H

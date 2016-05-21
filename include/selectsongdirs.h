#pragma once
#include <QDialog>

namespace Ui {
class SelectSongDirs;
}

class SelectSongDirs : public QDialog
{
    Q_OBJECT

public:
    explicit SelectSongDirs(QWidget *parent = 0);
    virtual ~SelectSongDirs();

    void setPaths(QStringList paths);
    QStringList getPaths() const;
    bool changed() const;

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::SelectSongDirs* ui;
    bool _changed;
};

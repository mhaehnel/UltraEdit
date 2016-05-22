#ifndef SONGIMPORTDIALOG_H
#define SONGIMPORTDIALOG_H

#include <QDialog>
#include <song.h>

namespace Ui {
class SongImportDialog;
}

class MainWindow;

class SongImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SongImportDialog(const QList<Collection*>& cols, Song* song, MainWindow *parent);
    ~SongImportDialog();

private slots:
    void import();

private:
    Song* song_;
    QPushButton* importBtn;
    const QList<Collection*>& cols_;
    Ui::SongImportDialog *ui;

    void createAlt(QFileInfo& fi, QPushButton* button, QString filter);
};

#endif // SONGIMPORTDIALOG_H

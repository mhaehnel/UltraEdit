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
    explicit SongImportDialog(Song* song, MainWindow *parent);
    ~SongImportDialog();

private:
    Song* song_;
    QPushButton* importBtn;
    Ui::SongImportDialog *ui;
};

#endif // SONGIMPORTDIALOG_H

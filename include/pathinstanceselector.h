#ifndef PATHINSTANCESELECTOR_H
#define PATHINSTANCESELECTOR_H

#include <QDialog>

namespace Ui {
class PathInstanceSelector;
}

class PathInstanceSelector : public QDialog
{
    Q_OBJECT

public:
    explicit PathInstanceSelector(QStringList& choice, QWidget *parent = 0);
    const QString& getSelected() { return selectedText; }
    ~PathInstanceSelector();

private slots:
    void on_buttonBox_accepted();

private:
    QString selectedText;
    Ui::PathInstanceSelector *ui;
};

#endif // PATHINSTANCESELECTOR_H

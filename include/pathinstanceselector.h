#pragma once
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
    virtual ~PathInstanceSelector();

private slots:
    void on_buttonBox_accepted();

private:
    QString selectedText;
    Ui::PathInstanceSelector* ui;
};

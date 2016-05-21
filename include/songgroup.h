#pragma once
#include <QFrame>

namespace Ui {
class SongGroup;
}

class SongGroup : public QFrame
{
    Q_OBJECT

public:
    explicit SongGroup(QString groupName, QWidget *parent = 0);
    ~SongGroup();
    QLayout& layout();
private:
   Ui::SongGroup* ui;
};

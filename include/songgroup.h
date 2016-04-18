#pragma once
#include <memory>
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
    std::unique_ptr<Ui::SongGroup> ui;
};

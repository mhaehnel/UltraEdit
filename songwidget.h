#ifndef SONGWIDGET_H
#define SONGWIDGET_H

#include <QWidget>
#include "song.h"

namespace Ui {
class SongWidget;
}

class SongWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SongWidget(const Song &song, QWidget *parent = 0);
    ~SongWidget();
private:
    Ui::SongWidget *ui;
};

#endif // SONGWIDGET_H

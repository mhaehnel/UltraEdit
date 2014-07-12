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
    Song* _song;
public:
    explicit SongWidget(Song *song, QWidget *parent = 0);
    Song* song();
    ~SongWidget();
private:
    Ui::SongWidget *ui;
};

#endif // SONGWIDGET_H

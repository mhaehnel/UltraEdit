#ifndef SONGFRAME_H
#define SONGFRAME_H

#include "song.h"
#include <QFrame>
#include <QMouseEvent>

namespace Ui {
class songframe;
}

class SongFrame : public QFrame
{
    Q_OBJECT
    Song* _song;

public:
    explicit SongFrame(Song* song, QWidget *parent = 0);
    Song* song();
    const Song* song() const;

    ~SongFrame();

protected:
    void mousePressEvent(QMouseEvent *ev);

signals:
    void clicked(SongFrame* sf);

public slots:
    void select();
    void deselect();

private:
    Ui::songframe *ui;
    void markMissing(QPixmap &pm);
    void markNo(QPixmap &pm);
};

#endif // SONGFRAME_H

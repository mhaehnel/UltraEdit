#ifndef SONGFRAME_H
#define SONGFRAME_H

#include "song.h"
#include <QFrame>

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

private:
    Ui::songframe *ui;
};

#endif // SONGFRAME_H

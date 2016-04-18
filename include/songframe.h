#pragma once
#include <memory>
#include "song.h"
#include <QFrame>
#include <QMouseEvent>

namespace Ui {
class songframe;
}

class SongFrame : public QFrame
{
    Q_OBJECT

public:
    Song *const song;
    explicit SongFrame(Song* song, QWidget *parent = 0);

    virtual ~SongFrame();

protected:
    void mousePressEvent(QMouseEvent *ev);

signals:
    void clicked(SongFrame* sf);
    void playSong(Song* song);
public slots:
    void select();
    void deselect();
    void updateData();

private slots:
    void on_playMedia_clicked();

private:
    std::unique_ptr<Ui::songframe> ui;
    void markMissing(QPixmap &pm);
    void markNo(QPixmap &pm);
};

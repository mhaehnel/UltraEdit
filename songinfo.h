#ifndef SONGINFO_H
#define SONGINFO_H

#include <QWidget>

namespace Ui {
class SongInfo;
}

class SongInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SongInfo(QWidget *parent = 0);
    ~SongInfo();

private:
    Ui::SongInfo *ui;
};

#endif // SONGINFO_H

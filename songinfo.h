#ifndef SONGINFO_H
#define SONGINFO_H

#include "songframe.h"
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

public slots:
    void setSelection(QList<SongFrame*> *selected);
    void selectionUpdated();

private slots:
    void on_title_textChanged(const QString &arg1);
    void on_artist_textChanged(const QString &arg1);
    void highlightText(int from, int to);

private:
    Ui::SongInfo *ui;
    QMetaObject::Connection conSylText;
    QMetaObject::Connection conSylLine;
    QMetaObject::Connection conSyl;
    QList<SongFrame*> *selection;
};

#endif // SONGINFO_H

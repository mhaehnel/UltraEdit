#ifndef SONGINFO_H
#define SONGINFO_H

#include "songframe.h"
#include "midiplayer.h"
#include <QWidget>

namespace Ui {
class SongInfo;
}

class SongInfo : public QWidget
{
    Q_OBJECT

public:
    explicit SongInfo(QWidget *parent = 0);
    QStringList getMidiPorts();
    ~SongInfo();

public slots:
    void setSelection(QList<SongFrame*> *selected);
    void selectionUpdated();
    void selectionChanged();
    void setMidiPort(QString port);

private slots:
    void on_title_textChanged(const QString &arg1);
    void on_artist_textChanged(const QString &arg1);
    void highlightText(int from, int to);

signals:
    void seek(quint64 pos);
    void play();
    void pause();

private:
    Ui::SongInfo *ui;
    MidiPlayer midiPlayer;
    QMetaObject::Connection conSylText;
    QMetaObject::Connection conSylLine;
    QMetaObject::Connection conSyl;
    QList<SongFrame*> *selection;
};

#endif // SONGINFO_H

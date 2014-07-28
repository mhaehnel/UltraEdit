#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QWidget>
#include "sylabel.h"
#include <QList>
#include <QFile>

namespace Ui {
class NoteWidget;
}

class NoteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteWidget(QWidget *parent = 0);
    void setNotes(QList<Sylabel> notes);
    ~NoteWidget();

public slots:
    void setCurrentNote(Sylabel s);
private:
    void paintEvent(QPaintEvent *);
    Ui::NoteWidget *ui;
    int maxKey, minKey, startBeat, totalBeats;
    QByteArray gclef;
    QByteArray fclef;
    const Sylabel* currentNote;
    QList<Sylabel> _notes;
};

#endif // NOTEWIDGET_H

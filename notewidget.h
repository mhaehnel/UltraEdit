#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QWidget>
#include "sylabel.h"
#include <QList>
#include <QSet>
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

signals:
    void seek(quint64 pos);
    void pause();
    void play();

public slots:
    void setCurrentNote(Sylabel s);

protected:
    void mousePressEvent(QMouseEvent *event);
private:
    void paintEvent(QPaintEvent *);
    Ui::NoteWidget *ui;
    int maxKey, minKey, startBeat, totalBeats;
    QByteArray gclef, fclef, sharp, natural;
    const Sylabel* currentNote;
    QList<Sylabel> _notes;
    Sylabel::Clef currentClef;
    double notesStart;
    double lengthPerBeat;
    QSet<Sylabel::Note> sharpies;
    QSet<Sylabel::Note> nonSharpies;
    QSet<Sylabel::Note> someSharpies;
};

#endif // NOTEWIDGET_H

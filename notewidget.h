#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <QWidget>
#include "sylabel.h"
#include <QList>
#include <QSet>
#include <QFile>
#include <QMap>

namespace Ui {
class NoteWidget;
}

class NoteWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NoteWidget(QWidget *parent = 0);
    int line() const;
    int lines() const;
    ~NoteWidget();

signals:
    void lineCount(int count);
    void lineChanged(int line);
    void seek(quint64 pos);
    void pause();
    void play();

public slots:
    void setCurrentNote(Sylabel s);
    void setLine(int line);
    void setNotes(QList<Sylabel> const& note);
    void goToLine(int line);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void paintEvent(QPaintEvent *);
    Ui::NoteWidget *ui;
    int maxKey, minKey, startBeat, totalBeats;
    QByteArray gclef, fclef, sharp, natural;
    const Sylabel* currentNote;
    QMap<int,QList<Sylabel*>> _notes;
    Sylabel::Clef currentClef;
    double notesStart;
    double lengthPerBeat;
    int currentLine;
    QSet<Sylabel::Note> sharpies;
    QSet<Sylabel::Note> nonSharpies;
    QSet<Sylabel::Note> someSharpies;
};

#endif // NOTEWIDGET_H

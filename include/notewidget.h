#ifndef NOTEWIDGET_H
#define NOTEWIDGET_H

#include <memory>
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
    inline int line() const { return currentLine; }
    inline int lines() const { return _notes.size(); }
    virtual ~NoteWidget();

signals:
    void lineCount(int count);
    void lineChanged(int line);
    void seek(quint64 pos);
    void pause();
    void play();

public slots:
    void setCurrentNote(Sylabel* s);
    void setLine(int line);
    void setSong(Song* song);
    void goToLine(int line);
    void refreshData();

protected:
    void mousePressEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);

private:
    static void loadSVG(QString file, QByteArray& target);
    static QByteArray gclef, fclef, sharp, natural;
    void paintEvent(QPaintEvent *);
    void calculate();
    void transpose(int steps);
    bool keepLine = false;
    bool keepSylabel = false;
    std::unique_ptr<Ui::NoteWidget> ui;
    unsigned char maxKey, minKey;
    int startBeat, totalBeats;
    Sylabel* currentNote;
    int currentNoteIdx;
    QMap<int,QList<Sylabel*>> _notes;
    QHash<QRectF,Sylabel*> noteGraphs;
    QHash<QRectF,Sylabel*> texts;
    int currentLine;
    QTimer breaker;
    QSet<Sylabel::Note> sharpies;
    QSet<Sylabel::Note> someSharpies;
};

#endif // NOTEWIDGET_H

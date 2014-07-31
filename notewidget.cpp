#include "notewidget.h"
#include "ui_notewidget.h"
#include <QtSvg/QSvgRenderer>
#include <cassert>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <sylabel.h>
#include <song.h>

NoteWidget::NoteWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::NoteWidget), maxKey(0), minKey(0), currentLine(0)
{
    QFile gcf(":/images/gclef.svg");
    gcf.open(QIODevice::ReadOnly);
    gclef = gcf.readAll();

    QFile fcf(":/images/fclef.svg");
    fcf.open(QIODevice::ReadOnly);
    fclef = fcf.readAll();

    QFile shf(":/images/sharp.svg");
    shf.open(QIODevice::ReadOnly);
    sharp = shf.readAll();

    QFile naf(":/images/natural.svg");
    naf.open(QIODevice::ReadOnly);
    natural = naf.readAll();

    ui->setupUi(this);
}

NoteWidget::~NoteWidget()
{
    delete ui;
}

int NoteWidget::line() const {
    qWarning() << currentLine;
    return currentLine;
}

int NoteWidget::lines() const {
    return _notes.size();
}

void NoteWidget::goToLine(int line) {
    if (line >= _notes.size() || line < 0) return;
    setLine(line);
    emit seek(_notes[line].first()->time()*1000+_notes.first().first()->song->gap());
}

void NoteWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int lineHeight = fontMetrics().height();
    int ycenter = height()/2;
    for (int i : {-2,-1,0,1,2})
        painter.drawLine(0,ycenter-lineHeight*i,width()-lineHeight,ycenter-lineHeight*i);
    painter.drawLine(width()-lineHeight,ycenter+lineHeight*2,width()-lineHeight,ycenter-lineHeight*2);
    painter.drawLine(0,ycenter+lineHeight*2,0,ycenter-lineHeight*2);
    //painter.drawLine(width()-lineHeight*1.2,ycenter+lineHeight*2,width()-lineHeight*1.2,ycenter-lineHeight*2);

    QSvgRenderer renderer(this);
    double height, top;
    currentClef = Sylabel::Clef::G;
    if (minKey < -3 && maxKey < 4) currentClef = Sylabel::Clef::F;
    if (currentClef == Sylabel::Clef::G) {
        renderer.load(gclef);
        height = lineHeight*6.5;
        top = ycenter-3.25*lineHeight;
    } else {
        renderer.load(fclef);
        height=7.5*lineHeight;
        top = ycenter-4*lineHeight;
    }
    double ratio = 1.0*renderer.defaultSize().width()/renderer.defaultSize().height();
    double width = ratio*height;

    renderer.render(&painter,QRectF(lineHeight,top,width,height));
    lengthPerBeat = (this->width()-6.5*lineHeight-width)/totalBeats;
    double pos = 5*lineHeight+width;
    notesStart = pos;
    QSet<Sylabel::Note> sharpified;
    renderer.load(sharp);
    QSvgRenderer rnatural(natural,this);
    for (const Sylabel* s : _notes[currentLine]) {
        double y = ycenter-lineHeight*(s->getLine(currentClef)-6)*0.5;
        double x = pos+(s->beat()-startBeat)*lengthPerBeat;
        double length = s->beats()*lengthPerBeat;
        QLinearGradient noteGrad(x,y,x+length,y);
        if (s == currentNote) {
            noteGrad.setColorAt(0,Qt::green);
        } else {
            noteGrad.setColorAt(0,Qt::blue);
        }
        noteGrad.setColorAt(1,Qt::white);
        painter.setBrush(QBrush(noteGrad));
        if (s->getLine(currentClef)-3 < -1) {
            for (int i = -6; i >= s->getLine(currentClef)-6; i-=2)
                painter.drawLine(x-0.25*lengthPerBeat,ycenter-lineHeight*i*0.5,x+length+0.25*lengthPerBeat,ycenter-lineHeight*i*0.5);
        }
        if (s->getLine(currentClef)-3 > 6) {
            for (int i = 6; i <= s->getLine(currentClef)-6; i+=2)
                painter.drawLine(x-0.25*lengthPerBeat,ycenter-lineHeight*i*0.5,x+length+0.25*lengthPerBeat,ycenter-lineHeight*i*0.5);
        }
        if (someSharpies.contains(s->note())) {
            if (s->isSharp() && !sharpified.contains(s->note())) {
                sharpified.insert(s->note());
                renderer.render(&painter,QRectF(x-0.9*lineHeight,y-1.5*lineHeight,0.8*lineHeight,2.5*lineHeight));
            } else if (!s->isSharp() && sharpified.contains(s->note())) {
                sharpified.remove(s->note());
                rnatural.render(&painter,QRectF(x-0.9*lineHeight,y-1.5*lineHeight,0.8*lineHeight,2.5*lineHeight));
            }
        }
        painter.drawRoundedRect(QRectF(x,y-0.4*lineHeight,length,0.8*lineHeight),1.2,1.2);
    }
    pos = 1.5*lineHeight+width;
    double yfix = (currentClef == Sylabel::Clef::F)?lineHeight:0;
    if (sharpies.contains(Sylabel::Note::F))
        renderer.render(&painter,QRectF(pos,ycenter-3.5*lineHeight+yfix,lineHeight,3*lineHeight));
    if (sharpies.contains(Sylabel::Note::C))
        renderer.render(&painter,QRectF(pos+0.75*lineHeight,ycenter-2*lineHeight+yfix,lineHeight,2.5*lineHeight));
    if (sharpies.contains(Sylabel::Note::G))
        renderer.render(&painter,QRectF(pos+1.5*lineHeight,ycenter-4*lineHeight+yfix,lineHeight,2.5*lineHeight));
    if (sharpies.contains(Sylabel::Note::D))
        renderer.render(&painter,QRectF(pos+2.25*lineHeight,ycenter-2.5*lineHeight+yfix,lineHeight,2.5*lineHeight));
    if (sharpies.contains(Sylabel::Note::A))
        renderer.render(&painter,QRectF(pos+3*lineHeight,ycenter-1*lineHeight+yfix,lineHeight,2.5*lineHeight));
    double lengthSoFar = 4*lineHeight+width;

    painter.save();
    QFont textFont(painter.font());
    textFont.setPointSize(21);
    painter.setFont(textFont);

    for (const Sylabel* s : _notes[currentLine]) {
        double w = painter.fontMetrics().width(s->text());
        painter.save();
        if (s == currentNote) {
            QPen colPen(painter.pen());
            colPen.setColor(Qt::green);
            painter.setPen(colPen);
        }
        painter.drawText(QRectF(lengthSoFar,ycenter+lineHeight*10,w,painter.fontMetrics().height()),s->text());
        lengthSoFar+=w;
        painter.restore();
    }
    painter.restore();
    //painter.drawRect(QRectF(lineHeight,top,width,height));
}

void NoteWidget::setSong(Song* song) {
    int line = 0;
    _notes.clear();
    for (Sylabel& s : song->sylabels()) {
        if (s.type() == Sylabel::Type::LineBreak) line++;
        else _notes[line].append(&s);
        //TODO: This does not handle linebreak timings!
    }
    currentLine = -1;
    setLine(0); //We need this to paint the first line.
    currentLine = 0;
    currentNote = _notes.first().first();
    emit lineCount(line);
    emit lineChanged(0);
}

void NoteWidget::setLine(int line) {
    if (currentLine == line) return;
    const QList<Sylabel*>& notes = _notes[line];
    assert(notes.size() > 0);
    sharpies.clear();
    nonSharpies.clear();
    someSharpies.clear();
    minKey = std::numeric_limits<int>::max();
    maxKey = std::numeric_limits<int>::min();
    startBeat = notes.first()->beat();
    totalBeats = notes.last()->beat()+notes.last()->beats()-startBeat;
    qWarning() << "Line:" << line << "Beats: " << startBeat << "-" << startBeat+totalBeats;
    for (const Sylabel* s : notes) {
        Sylabel::Note n = s->note();
        if (!someSharpies.contains(n)) {
            if (s->isSharp()) {
                if (nonSharpies.contains(n)) {
                    nonSharpies.remove(n);
                    someSharpies.insert(n);
                } else {
                    sharpies.insert(n);
                }
            } else {
                if (sharpies.contains(n)) {
                    sharpies.remove(n);
                    someSharpies.insert(n);
                } else {
                    nonSharpies.insert(n);
                }
            }
        }
        if (s->key() < minKey) minKey = s->key();
        if (s->key() > maxKey) maxKey = s->key();
    }

    currentNote = notes.first();
    currentLine = line;
    emit lineChanged(line);
    update();
}

void NoteWidget::setCurrentNote(Sylabel s) {
    for (const Sylabel* _s : _notes[currentLine]) {
        if (*_s == s) {
            currentNote = _s;
            break;
        }
    }
    update();
}


void NoteWidget::mousePressEvent(QMouseEvent *event) {
//    int ycenter = height()/2;
//    int lineHeight = fontMetrics().height();
    //Pos2line

    int beat = std::floor(startBeat+(event->localPos().x() - notesStart)/lengthPerBeat);
    qWarning() << "Clicked beat" << beat;
    for (const Sylabel* _s : _notes[currentLine]) {
        const Song* song = _s->song;
        if (_s->beat() <= beat && _s->beat()+_s->beats() > beat) {
            qWarning() << "Sylabel: " << _s->text();
            qWarning() << "Emiting seek to: " << _s->time()+song->gap()/1000.0;
            emit seek(_s->time()*1000.0+song->gap());
            return;
        }
    }
    qWarning() << "You missed!";
}

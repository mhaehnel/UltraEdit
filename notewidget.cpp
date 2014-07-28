#include "notewidget.h"
#include "ui_notewidget.h"
#include <QtSvg/QSvgRenderer>
#include <cassert>
#include <QPainter>
#include <QDebug>

NoteWidget::NoteWidget(QWidget *parent) :
    QWidget(parent), ui(new Ui::NoteWidget), maxKey(0), minKey(0)
{
    QFile gcf(":/images/gclef.svg");
    gcf.open(QIODevice::ReadOnly);
    gclef = gcf.readAll();

    QFile fcf(":/images/fclef.svg");
    fcf.open(QIODevice::ReadOnly);
    fclef = fcf.readAll();

    ui->setupUi(this);
}

NoteWidget::~NoteWidget()
{
    delete ui;
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
    Sylabel::Clef c = Sylabel::Clef::G;
    if (minKey < -3 && maxKey < 4) c = Sylabel::Clef::F;
    if (c == Sylabel::Clef::G) {
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
    double lengthPerBeat = (this->width()-4*lineHeight-width)/totalBeats;
    double pos = 2*lineHeight+width;
    for (const Sylabel& s : _notes) {
        double y = ycenter-lineHeight*(s.getLine(c)-6)*0.5;
        double x = pos+(s.beat()-startBeat)*lengthPerBeat;
        double length = s.beats()*lengthPerBeat;
        QLinearGradient noteGrad(x,y,x+length,y);
        if (&s == currentNote) {
            noteGrad.setColorAt(0,Qt::green);
        } else {
            noteGrad.setColorAt(0,Qt::blue);
        }
        noteGrad.setColorAt(1,Qt::white);
        painter.setBrush(QBrush(noteGrad));
        if (s.getLine(c)-3 < -1) {
            for (int i = -6; i >= s.getLine(c)-6; i-=2)
                painter.drawLine(x-0.25*lengthPerBeat,ycenter-lineHeight*i*0.5,x+length+0.25*lengthPerBeat,ycenter-lineHeight*i*0.5);
        }

        painter.drawRoundedRect(QRectF(x,y-0.4*lineHeight,length,0.8*lineHeight),1.2,1.2);
    }
    double lengthSoFar = pos;

    painter.save();
    QFont textFont(painter.font());
    textFont.setPointSize(21);
    painter.setFont(textFont);

    for (const Sylabel& s : _notes) {
        double w = painter.fontMetrics().width(s.text());
        painter.save();
        if (&s == currentNote) {
            QPen colPen(painter.pen());
            colPen.setColor(Qt::green);
            painter.setPen(colPen);
        }
        painter.drawText(QRectF(lengthSoFar,ycenter+lineHeight*10,w,painter.fontMetrics().height()),s.text());
        lengthSoFar+=w;
        painter.restore();
    }
    painter.restore();
    //painter.drawRect(QRectF(lineHeight,top,width,height));
}

void NoteWidget::setNotes(QList<Sylabel> notes) {
    assert(notes.size() > 0);
    minKey = std::numeric_limits<int>::max();
    maxKey = std::numeric_limits<int>::min();
    startBeat = notes.first().beat();
    totalBeats = notes.last().beat()+notes.last().beats()-startBeat;
    for (const Sylabel& s : notes) {
        if (s.key() < minKey) minKey = s.key();
        if (s.key() > maxKey) maxKey = s.key();
    }
    _notes = notes;
    currentNote = &_notes.first();
    update();
}

void NoteWidget::setCurrentNote(Sylabel s) {
    for (const Sylabel& _s : _notes) {
        if (_s == s) currentNote = &_s;
    }
    update();
}

#include "notewidget.h"
#include "ui_notewidget.h"
#include <QtSvg/QSvgRenderer>
#include <cassert>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <sylabel.h>
#include <song.h>
#include <QTimer>
#include <QMutex>
#include <QInputDialog>

QByteArray NoteWidget::gclef, NoteWidget::fclef, NoteWidget::sharp, NoteWidget::natural;

NoteWidget::NoteWidget(QWidget *parent) :
    QWidget(parent), ui(std::make_unique<Ui::NoteWidget>()), maxKey(0), minKey(0), currentLine(0)
{
    loadSVG(":/images/gclef.svg",gclef);
    loadSVG(":/images/fclef.svg",fclef);
    loadSVG(":/images/sharp.svg",sharp);
    loadSVG(":/images/natural.svg",natural);

    ui->setupUi(this);
    setFocusPolicy(Qt::ClickFocus);
    breaker.setSingleShot(true);
    connect(&breaker,&QTimer::timeout,[this] {
        emit pause();
        qApp->processEvents();
        keepLine = keepSylabel = false;
    });
}

NoteWidget::~NoteWidget() {}

void NoteWidget::loadSVG(QString file, QByteArray& target) {
    if (!target.isEmpty()) return;
    QFile f(file);
    f.open(QIODevice::ReadOnly);
    target = f.readAll();
}

void NoteWidget::goToLine(int line) {
    if (line >= _notes.size() || line < 0) return;
    setLine(line);
    emit seek(_notes[line].first()->time()*1000+_notes.first().first()->song->gap());
}

void NoteWidget::keyPressEvent(QKeyEvent *event) {
    double timeout;
    static QMutex mutex;
    const QList<Sylabel*>& ln = _notes[currentLine];
    bool shift = event->modifiers()&Qt::ShiftModifier;
    switch (event->key()) {
        case Qt::Key_Plus:     currentNote->transpose(shift?12:1); return;
        case Qt::Key_Minus:    currentNote->transpose(shift?-12:-1); return;
        case Qt::Key_PageDown: transpose(shift?-12:-1); return;
        case Qt::Key_PageUp:   transpose(shift?12:1); return;
        case Qt::Key_Down:     goToLine(currentLine+1); return;
        case Qt::Key_Up:       goToLine(currentLine-1); return;
        case Qt::Key_P:        emit play(); return;
        case Qt::Key_Space: //Play only this line
            timeout = ln.last()->time()*1000+ln.last()->duration()*1000-
                      ln.first()->time()*1000;
            breaker.start(timeout);
            keepLine = true;
        case Qt::Key_Return: //Play this line and then continue
            goToLine(currentLine);
            emit play();
            return;
        case Qt::Key_Period:
            emit seek(currentNote->time()*1000+currentNote->song->gap());
            breaker.start(currentNote->duration()*1000);
            keepSylabel = true;
            emit play();
            return;
        case Qt::Key_Right:
            mutex.lock();
            if (currentNote == ln.last())
                goToLine(currentLine+1);
            else
                setCurrentNote( _notes[currentLine][ _notes[currentLine].indexOf(currentNote)+1]);
            emit seek(currentNote->time()*1000+currentNote->song->gap());
            mutex.unlock();
            return;
        case Qt::Key_Left:
            mutex.lock();
            if (currentNote == ln.first()) {
                goToLine(currentLine-1);
                setCurrentNote(_notes[currentLine].last());
            } else {
                setCurrentNote( _notes[currentLine][ _notes[currentLine].indexOf(currentNote)-1]);
            }
            emit seek(currentNote->time()*1000+currentNote->song->gap());
            mutex.unlock();
            return;
        default: QWidget::keyPressEvent(event);
    }
}

void NoteWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    int lineHeight = fontMetrics().height();
    int ycenter = height()/2;

    //Draw note lines
    for (int i : {-1,0,1})
        painter.drawLine(0,ycenter-lineHeight*i,width(),ycenter-lineHeight*i);
    painter.drawRect(0,ycenter-lineHeight*2,width(),lineHeight*4);

    QSvgRenderer renderer(this); //Render clef
    double height, top;
    Sylabel::Clef currentClef = Sylabel::Clef::G;
    if (minKey < 55 && maxKey < 64) currentClef = Sylabel::Clef::F;
    if (currentClef == Sylabel::Clef::G) {
        renderer.load(gclef);
        height = lineHeight*6.5;
        top = ycenter-3.25*lineHeight;
    } else {
        renderer.load(fclef);
        height=7.5*lineHeight;
        top = ycenter-4*lineHeight;
    }
    double width = height*renderer.defaultSize().width()/renderer.defaultSize().height();
    renderer.render(&painter,QRectF(lineHeight,top,width,height));

    double lengthPerBeat = (this->width()-6.5*lineHeight-width)/totalBeats;
    double pos = 5*lineHeight+width;
    QSet<Sylabel::Note> sharpified;
    renderer.load(sharp);
    QSvgRenderer rnatural(natural,this);
    for (Sylabel* s : _notes[currentLine]) {
        double y = ycenter-lineHeight*(s->line(currentClef)-6)*0.5;
        double x = pos+(s->beat()-startBeat)*lengthPerBeat;
        double length = s->beats()*lengthPerBeat;

        QLinearGradient noteGrad(x,y,x+length,y); //Set color ...
        if (s == currentNote) {
            if (s->type() == Sylabel::Type::Golden)
                noteGrad.setColorAt(0,Qt::red);
            else
                noteGrad.setColorAt(0,Qt::green);
        } else {
            if (s->type() == Sylabel::Type::Golden)
                noteGrad.setColorAt(0,QColor(255,215,0));
            else
                noteGrad.setColorAt(0,Qt::blue);
        }
        noteGrad.setColorAt(1,Qt::white);
        painter.setBrush(QBrush(noteGrad));

        if (s->line(currentClef)-3 < -1) { //Lines below std. grid
            for (int i = -6; i >= s->line(currentClef)-6; i-=2)
                painter.drawLine(x-0.25*lengthPerBeat,ycenter-lineHeight*i*0.5,x+length+0.25*lengthPerBeat,ycenter-lineHeight*i*0.5);
        }
        if (s->line(currentClef)-3 > 6) { //Lines above std. grid
            for (int i = 6; i <= s->line(currentClef)-6; i+=2)
                painter.drawLine(x-0.25*lengthPerBeat,ycenter-lineHeight*i*0.5,x+length+0.25*lengthPerBeat,ycenter-lineHeight*i*0.5);
        }

        if (someSharpies.contains(s->note())) { //Do we need to worry about sharp/flat tones?
            if (s->isSharp() && !sharpified.contains(s->note())) {
                sharpified.insert(s->note());
                renderer.render(&painter,QRectF(x-0.9*lineHeight,y-1.5*lineHeight,0.8*lineHeight,2.5*lineHeight));
            } else if (!s->isSharp() && sharpified.contains(s->note())) {
                sharpified.remove(s->note());
                rnatural.render(&painter,QRectF(x-0.9*lineHeight,y-1.5*lineHeight,0.8*lineHeight,2.5*lineHeight));
            }
        }

        //Finally Draw note
        QRectF r(x,y-0.4*lineHeight,length,0.8*lineHeight);
        painter.drawRoundedRect(r,1.2,1.2);
        noteGraphs.insert(r,s);
    }

    //Draw sharps
    pos = 1.5*lineHeight+width;
    double yfix = (currentClef == Sylabel::Clef::F)?lineHeight:0;
    if (sharpies.contains(Sylabel::Note::F))
        renderer.render(&painter,QRectF(pos,ycenter-3.5*lineHeight+yfix,lineHeight,2.5*lineHeight));
    if (sharpies.contains(Sylabel::Note::C))
        renderer.render(&painter,QRectF(pos+0.75*lineHeight,ycenter-2*lineHeight+yfix,lineHeight,2.5*lineHeight));
    if (sharpies.contains(Sylabel::Note::G))
        renderer.render(&painter,QRectF(pos+1.5*lineHeight,ycenter-4*lineHeight+yfix,lineHeight,2.5*lineHeight));
    if (sharpies.contains(Sylabel::Note::D))
        renderer.render(&painter,QRectF(pos+2.25*lineHeight,ycenter-2.5*lineHeight+yfix,lineHeight,2.5*lineHeight));
    if (sharpies.contains(Sylabel::Note::A))
        renderer.render(&painter,QRectF(pos+3*lineHeight,ycenter-1*lineHeight+yfix,lineHeight,2.5*lineHeight));
    double lengthSoFar = 4*lineHeight+width;

    painter.save(); //Todo: This needs a major makeover! We might want to write them under the tones (?)
    QFont textFont(painter.font());
    textFont.setPointSize(21);
    painter.setFont(textFont);

    for (Sylabel* s : _notes[currentLine]) {
        double w = painter.fontMetrics().width(s->text());
        painter.save();
        if (s == currentNote) {
            QPen colPen(painter.pen());
            colPen.setColor(Qt::green);
            painter.setPen(colPen);
        }
        QRectF r(lengthSoFar,ycenter+lineHeight*10,w,painter.fontMetrics().height());
        painter.drawText(r,s->text());
        texts.insert(r,s);
        lengthSoFar+=w;
        painter.restore();
    }
    painter.restore();
}

void NoteWidget::refreshData() {
    calculate();
    update();
}

void NoteWidget::setSong(Song* song) {
    static QMetaObject::Connection c;
    disconnect(c);
    c = connect(song,SIGNAL(updated()),this,SLOT(refreshData()));
    int line = 0;
    _notes.clear();
    for (Sylabel* s : song->sylabels()) { //TODO: This does not handle linebreak timings!
        if (s->isLineBreak())
            line++;
        else
            _notes[line].append(s);
    }
    currentLine = -1; //We need this to paint the first line.
    setLine(0);
    currentLine = 0;
    currentNote = _notes.first().first();
    emit lineCount(line);
    emit lineChanged(0);
}

void NoteWidget::calculate() {
    const QList<Sylabel*>& notes = _notes[currentLine];
    QSet<Sylabel::Note> nonSharpies;

    sharpies.clear();
    someSharpies.clear();
    minKey = std::numeric_limits<unsigned char>::max();
    maxKey = std::numeric_limits<unsigned char>::min();
    startBeat = notes.first()->beat();
    totalBeats = notes.last()->beat()+notes.last()->beats()-startBeat;

    for (const Sylabel* s : notes) {
        Sylabel::Note n = s->note();
        if (!someSharpies.contains(n)) {
            QSet<Sylabel::Note>& belongs = s->isSharp()?sharpies:nonSharpies;
            QSet<Sylabel::Note>& mustNot = s->isSharp()?nonSharpies:sharpies;
            if (mustNot.contains(n)) {
                mustNot.remove(n);
                someSharpies.insert(n);
            } else {
                belongs.insert(n);
            }
        }
        if (s->key() < minKey) minKey = s->key();
        if (s->key() > maxKey) maxKey = s->key();
    }
}

void NoteWidget::setLine(int line) {
    line--;
    if (currentLine == line) return;
    if (keepLine) return;
    assert(_notes[line].size() > 0);
    currentNote = _notes[line].first();
    currentLine = line;
    refreshData();
    emit lineChanged(line);
}

void NoteWidget::setCurrentNote(Sylabel* s) {
    if (s == currentNote || keepSylabel) return;
    if (!_notes[currentLine].contains(s)) return;
    currentNote = s;
    update();
}

void NoteWidget::transpose(int steps) {
    for (Sylabel* s : currentNote->song->sylabels())
            s->transpose(steps);
}

void NoteWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    for (QRectF &r : texts.keys()) {
        if (r.contains(event->pos())) {
            bool ok;

            QString newText = QInputDialog::getText(this,"Change Sylabel","Update sylabel text",QLineEdit::Normal,texts[r]->text(),&ok);
            if (ok) texts[r]->setText(newText);
            return;
        }
    }
}

void NoteWidget::wheelEvent(QWheelEvent *event) {
    int steps = -event->delta()/8/15;

    if (event->orientation() ==Qt::Vertical)
        goToLine(std::min(std::max(0,currentLine+steps),_notes.size()-1));
}

void NoteWidget::mousePressEvent(QMouseEvent *event) {
    for (QRectF &r : texts.keys()) {
        if (r.contains(event->pos())) {
            setCurrentNote(texts[r]);
            emit seek(texts[r]->time()*1000+texts[r]->song->gap());
            return;
        }
    }
    for (QRectF &r : noteGraphs.keys()) {
        if (r.contains(event->pos())) {
            setCurrentNote(noteGraphs[r]);
            emit seek(noteGraphs[r]->time()*1000+noteGraphs[r]->song->gap());
            return;
        }
    }
}

uint qHash(const QRectF &r)
{
    return qHash(QString("%1,%2,%3,%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height()));
}

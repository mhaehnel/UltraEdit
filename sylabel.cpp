#include "sylabel.h"
#include <QStringList>
#include <QDebug>
#include <cassert>
#include <song.h>

int Sylabel::ppq = 96;

Sylabel::Sylabel(QString source, int players, Song* song) : QObject(),  song(song), _event(nullptr), _players(players)
{
    _t = Type::Bad;
    Type t = Type::Bad; //Internal (only set if error!) TODO: Add error messages for diagnosis!
    if (source.isEmpty()) return;

    switch (source.at(0).toUpper().toLatin1()) {
        case ':': t = Type::Regular; break;
        case '*': t = Type::Golden; break;
        case 'F': t = Type::Freestyle; break;
        case '-': t = Type::LineBreak; break;
        default: return;
    }

    source.remove(0,1);
    QStringList data = source.split(" ",QString::SkipEmptyParts);
    bool ok;

    if (data.size() < 1) return;
    _beat = data[0].toInt(&ok);
    if (!ok) return;
    data.removeFirst();
    if (t == Type::LineBreak) {
        if (data.size() == 1) { //TODO: Check that this is represented in the editor somehow!
//            qWarning() << "Edge case found! Please check manually! Not tested";
            //_beats = data[0].toInt(&ok);
            if (!ok) return;
            data.removeFirst();
        }
        if (data.size() != 0) return;
        _t = t;
        return;
    }
    if (data.size() < 3) return; //Invalid line! (BEtter error handling needed? TODO!)
    int beats = data[0].toInt(&ok);
    if (!ok) return;
    int pitch = data[1].toInt(&ok);
    if (!ok) return;

    _event = new drumstick::NoteEvent(0,pitch,100,beats*ppq/4);
    _t = t;
    _text = data[2]; //Pure text
    //Get text with trailing spaces (but not with leading seperators!)
    QRegExp re("\\s(\\s*"+QRegExp::escape(data[2])+"\\s*)$");
    re.indexIn(source);
    assert(re.captureCount() == 1 || "This can not fail!");
    _text = re.cap(1);
}

Sylabel::~Sylabel() {
    delete _event;
}

bool Sylabel::operator ==(const Sylabel& other) const {
    return other._text == _text && other.key() == key() &&
           other._t == _t && other._beat == _beat && other.beats() == beats();

}

void Sylabel::setText(QString text) {
     _text = text;
     emit updated();
}

bool Sylabel::isSharp() const {
    int idx = key()%12;
    return (idx == 1 || idx == 3 || idx == 6 || idx == 8 || idx == 10);
}

double Sylabel::time() const {
    return _beat/song->bpm()/4*60.0;
}

int Sylabel::beats() const {
 return _event->getDuration()*4/ppq;
}

double Sylabel::duration() const {
     return beats()/song->bpm()/4*60.0;
}

unsigned char Sylabel::key() const {
    if (_event == nullptr) return 0;
    return _event->getKey();
}

void Sylabel::transpose(char lvl) {
    _event->setKey(std::max(0,_event->getKey()+lvl));
    emit updated();
}

Sylabel::Note Sylabel::note() const {
    static Note tbl[] = {Note::C,Note::C,Note::D,Note::D,Note::E,Note::F,
                         Note::F,Note::G,Note::G,Note::A,Note::A,Note::B};
    return tbl[key()%12];
}

int Sylabel::line(Clef c) const {
    static int tbl[] = {0,0,1,1,2,3,3,4,4,5,5,6};
    int fixup = (key()/12-5)*7; //we are at C4 level
    return (tbl[key()%12] + fixup + ((c==Clef::G)?0:12));
}

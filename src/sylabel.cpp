#include "sylabel.h"
#include <QStringList>
#include <QDebug>
#include <cassert>
#include <song.h>

#include <exceptions/sylabelformatexception.h>
#include <QTextStream>

int Sylabel::ppq = 96;

Sylabel::Sylabel(QString source, int players, Song* song)
    : QObject(),  song(song), _event(nullptr), _players(players)
{
    using Reason = SylabelFormatException::Reason;
    if (source.isEmpty()) throw new SylabelFormatException(source,Reason::EmptyLine);

    switch (source.at(0).toUpper().toLatin1()) {
        case ':': _t = Type::Regular; break;
        case '*': _t = Type::Golden; break;
        case 'F': _t = Type::Freestyle; break;
        case '-': _t = Type::SimpleLineBreak; break;
        default: throw new SylabelFormatException(source,Reason::InvalidType);
    }

    source.remove(0,1);
    QTextStream data(&source);
    data.skipWhiteSpace();
    data >> _beat;
    switch (data.status()) {
        case QTextStream::ReadPastEnd:
            throw SylabelFormatException(source,Reason::NotEnoughData);
        case QTextStream::ReadCorruptData:
            throw SylabelFormatException(source,Reason::InvalidNumber);;
        default: ;
    }

    int beats = 0, pitch = 0;
    if (_t == Type::SimpleLineBreak) {
        data >> beats;
        switch (data.status()) {
            case QTextStream::ReadCorruptData:
                throw SylabelFormatException(source,Reason::InvalidNumber);
            case QTextStream::Ok:
                _t = Type::LineBreak; //Not simple
            default: ;
        }
        if (!data.readAll().trimmed().isEmpty())
            throw SylabelFormatException::Reason::ExtraData;
        _event = new drumstick::NoteEvent(0,0,100,beats*ppq/4);
        return;
    }
    for (auto d : {&beats,&pitch}) {
        data >> *d;
        switch (data.status()) {
            case QTextStream::ReadPastEnd:
                throw SylabelFormatException(source,Reason::NotEnoughData);
            case QTextStream::ReadCorruptData:
                throw SylabelFormatException(source,Reason::InvalidNumber);;
            default: ;
        }
    }
    _text = data.readAll().remove(0,1); //Pure text
    if (_text.trimmed().isEmpty())
        throw SylabelFormatException(source,Reason::NotEnoughData);
    _event = new drumstick::NoteEvent(0,pitch,100,beats*ppq/4);
}

Sylabel::~Sylabel() {
    delete _event;
}

void Sylabel::move(int beats) {
    _beat += beats;
    emit updated();
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
    if (isLineBreak()) return;
    _event->setKey(std::max(0,_event->getKey()+lvl));
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

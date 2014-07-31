#include "sylabel.h"
#include <QStringList>
#include <QDebug>
#include <cassert>
#include <song.h>

//TODO: Put error outputs on abort

Sylabel::Sylabel(QString source, int players, Song* song) : _beats(-1), _players(players), song(song)
{
    _t = Type::Bad;
    Type t = Type::Bad; //Internal (only set if all well!)
    if (source.isEmpty()) return;
    switch (source.at(0).toUpper().toLatin1()) {
        case ':': t = Type::Regular; break;
        case '*': t = Type::Golden; break;
        case 'F': t = Type::Freestyle; break;
        case '-': t = Type::LineBreak; break;
        default: return;
    }
    source = source.remove(0,1);
    QStringList data = source.split(" ",QString::SkipEmptyParts);
    bool ok;

    if (data.size() < 1) return;
    _beat = data[0].toInt(&ok);
    if (!ok) return;
    data.removeFirst();
    if (t == Type::LineBreak) {
        if (data.size() == 1) {
//            qWarning() << "Edge case found! Please check manually! Not tested";
            _beats = data[0].toInt(&ok);
            if (!ok) return;
            data.removeFirst();
        }
        if (data.size() != 0) return;
        _t = t;
        return;
    }
    if (data.size() < 3) return;
    _beats = data[0].toInt(&ok);
    if (!ok) return;
    _pitch = data[1].toInt(&ok);
    if (!ok) return;
    //Get pure text ...
    _text = data[2];
    QRegExp re("\\s(\\s*"+QRegExp::escape(data[2])+"\\s*)$");
    re.indexIn(source);
    assert(re.captureCount() == 1);
    _text = re.cap(1);
}

bool Sylabel::operator ==(const Sylabel& other) const {
    return other.key() == _pitch && other.text() == _text && other.type() == _t
            && other.beat() == _beat && other.beats() == _beats;

}

bool Sylabel::isSharp() const {
    int idx = (_pitch < 0)?_pitch%12+12:_pitch%12;
    return (idx == 1 || idx == 3 || idx == 6 || idx == 8 || idx == 10);
}

double Sylabel::time() const {
    return _beat/song->bpm()/4*60.0;
}

double Sylabel::duration() const {
     return _beats/song->bpm()/4*60.0;
}

Sylabel::Note Sylabel::note() const {
    Note tbl[] = {Note::C,Note::C,Note::D,Note::D,Note::E,Note::F,
                  Note::F,Note::G,Note::G,Note::A,Note::A,Note::B};
    return tbl[(_pitch < 0)?_pitch%12+12:_pitch%12];
}

int Sylabel::getLine(Clef c) const {
    static int tbl[] = {0,0,1,1,2,3,3,4,4,5,5,6};
    int base = (_pitch < 0)?_pitch%12+12:_pitch%12;
    int fixup = ((_pitch < 0)?_pitch/12-1:_pitch/12)*7;
//    qWarning() << "Base:" << base << "Fixup:" << fixup << "[]" << tbl[base];
    int result = (tbl[base] + fixup + ((c==Clef::G)?0:12));
//    qWarning() << "=>" << result;
    return result;
}

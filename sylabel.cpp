#include "sylabel.h"
#include <QStringList>
#include <QDebug>
#include <cassert>

//TODO: Put error outputs on abort

Sylabel::Sylabel(QString source, int players) : _beats(-1), _players(players)
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

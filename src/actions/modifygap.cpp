#include "actions/modifygap.h"
#include <actions/action.h>

Song::ModifyGap::ModifyGap(Type t, qint64 value)
    : newVal(value), t_(t)
{}

bool Song::ModifyGap::perform(Song &song) {
    switch (t_) {
        case Type::Video:
            oldVal = song._videoGap;
            song._videoGap = newVal;
            break;
        case Type::Audio:
            oldVal = song._gap;
            song._gap = newVal;
    }
    return true;
}

bool Song::ModifyGap::undo(Song &song) {
    switch (t_) {
        case Type::Video:
            song._videoGap = oldVal;
            break;
        case Type::Audio:
            song._gap = oldVal;
    }
    return true;
}

QString Song::ModifyGap::shortName() const {
    switch (t_) {
        case Type::Video: return "Modify VIDEOGAP";
        case Type::Audio: return "Modify GAP";
    }
    __builtin_unreachable();
}

QString Song::ModifyGap::description() const {
    switch (t_) {
        case Type::Video:
            return QString("Set VIDEOGAP from %1 to %2").arg(oldVal).arg(newVal);
        case Type::Audio:
            return QString("Set GAP from %1 to %2").arg(oldVal).arg(newVal);
    }
    __builtin_unreachable();
}


bool Song::ModifyGap::merge(const Action &other) {
    try {
        const Song::ModifyGap& o = dynamic_cast<const Song::ModifyGap&>(other);
        if (o.t_ != t_) return false;
        this->newVal = o.newVal;
        return true;
    } catch (std::bad_cast& e) {
        qWarning() << "Failed merge due to cast error!";
        return false;
    }
}

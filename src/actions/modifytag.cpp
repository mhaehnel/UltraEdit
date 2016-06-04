#include "actions/modifytag.h"
#include <actions/action.h>

Song::ModifyTag::ModifyTag(QString tag, Op op, QString val)
    : tag_(tag.toUpper()), newVal_(val), operation_(op)
{}

bool Song::ModifyTag::perform(Song& song) {
    switch (operation_) {
        case Op::Create:
            return song.addTag(tag_,newVal_);
        case Op::Modify:
            if (!song.hasTag(tag_)) return false;
            oldVal_ = song.tag(tag_);
            return song.setTag(tag_,newVal_);
        case Op::Delete:
            if (!song.hasTag(tag_)) return false;
            oldVal_ = song.tag(tag_);
            song.tags.remove(tag_);
            return true;
    }
    return false; //Stupid GCC
}

bool Song::ModifyTag::undo(Song& song) {
    //No checking needed. If this fails something is broken anyways. Maybe asserts?
    switch (operation_) {
        case Op::Create:
            song.tags.remove(tag_);
            return true;
        case Op::Modify:
            return song.setTag(tag_,oldVal_);
        case Op::Delete:
            return song.setTag(tag_,oldVal_);
    }
    return false; //Stupid GCC
}

bool Song::ModifyTag::isIdentity() const {
    return newVal_ == oldVal_;
}

QString Song::ModifyTag::shortName() const {
    switch (operation_) {
        case Op::Create: return "Create tag";
        case Op::Delete: return "Delete tag";
        case Op::Modify: return "Change tag";
    }
    return ""; //Stupid GCC
}

QString Song::ModifyTag::description() const {
    switch (operation_) {
        case Op::Create:
            return QString("Create tag '%1' with value '%2'").arg(tag_,newVal_);
        case Op::Delete:
            return QString("Delete tag '%1'").arg(tag_);
        case Op::Modify:
            return QString("Set tag '%1' from '%2' to '%3").arg(tag_,oldVal_,newVal_);
    }
    return ""; //Stupid GCC
}

bool Song::ModifyTag::merge(const Action& other) {
    try {
        const Song::ModifyTag& o = dynamic_cast<const Song::ModifyTag&>(other);
        //Only modifys can be merged
        if (o.operation_ != Op::Modify && operation_ != Op::Modify)
            return false;
        //Only same tag changes can be merged
        if (o.tag_ != tag_)
            return false;
        this->newVal_ = o.newVal_;
        return true;
    } catch (std::bad_cast& e) {
        qWarning() << "Failed merging due to cast error!";
        return false;
    }
}

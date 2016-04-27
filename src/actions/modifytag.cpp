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

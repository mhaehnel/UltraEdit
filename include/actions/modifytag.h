#pragma once

#include <song.h>

class Song::ModifyTag : public Action
{
public:
    enum class Op {
        Create, Modify, Delete
    };
    ModifyTag(QString tag, Op op, QString value = {});
    bool perform(Song &song) override;
    bool undo(Song &song) override;
    QString shortName() const override;
    QString description() const override;
    bool merge(const Action &other) override;
private:
    QString tag_, oldVal_, newVal_;
    Op operation_;
};

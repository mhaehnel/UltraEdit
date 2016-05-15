#pragma once

#include <song.h>

class Song::ModifyGap : public Action
{
public:
    enum class Type {
        Audio, Video
    };

    ModifyGap(Type t, quint64 value);
    bool perform(Song &song) override;
    bool undo(Song &song) override;
    QString shortName() const override;
    QString description() const override;
    bool merge(const Action &other) override;

private:
    quint64 oldVal, newVal;
    Type t_;
};

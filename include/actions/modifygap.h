#pragma once

#include <song.h>

class Song::ModifyGap : public Action
{
public:
    enum class Type {
        Audio, Video
    };

    ModifyGap(Type t, qint64 value);
    bool perform(Song &song) override;
    bool undo(Song &song) override;
    QString shortName() const override;
    QString description() const override;
    bool merge(const Action &other) override;
    bool canUndo() const override { return true; }
    bool isIdentity() const override;

private:
    qint64 oldVal, newVal;
    Type t_;
};

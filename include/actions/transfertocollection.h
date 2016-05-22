#pragma once

#include <song.h>

class Song::TransferToCollection : public Action
{
public:
    enum class Type {
        Move, Copy
    };

    TransferToCollection(const Collection* target, Type t);
    bool perform(Song &song) override;
    bool undo(Song &song) override;
    QString shortName() const override;
    QString description() const override;
private:
    const Collection *oldCollection, *newCollection;
    Type t_;
};

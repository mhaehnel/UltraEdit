#pragma once

#include <song.h>

class Song::TransferToCollection : public Action
{
public:
    enum class Type {
        Move, Copy
    };

    TransferToCollection(Collection* target, Type t);
    bool perform(Song &song) override;
    bool undo(Song &song) override;
    QString shortName() const override;
    QString description() const override;
private:
    Collection *oldCollection, *newCollection;
    Type t_;
};

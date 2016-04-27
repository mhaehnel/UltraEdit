#pragma once

#include <song.h>

class Song::TransposeSong : public Action
{
    int level;
public:
    TransposeSong(int lvl) : level(lvl) {}
    bool perform(Song& song) override;
    bool undo(Song& song) override;
    QString shortName() const override {
        return "Transpose";
    }

    QString description() const override {
        return "Transpose the song to a normal key";
    }
};

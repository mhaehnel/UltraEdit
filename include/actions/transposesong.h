#pragma once

#include <song.h>

class Song::TransposeSong : public Action
{
    int level;
public:
    TransposeSong(int lvl) : level(lvl) {}
    bool perform(Song& song) override;
    bool undo(Song& song) override;
    bool canUndo() const override { return true; }
    bool isIdentity() const override { return false; } //as long as we don't support merge
    QString shortName() const override {
        return "Transpose";
    }

    QString description() const override {
        return "Transpose the song to a normal key";
    }
};

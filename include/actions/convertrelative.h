#pragma once

#include <song.h>

class Song::ConvertRelative : public Action {
public:
    bool perform(Song& song) override;
    bool undo(Song& song) override;
    QString shortName() const override {
        return "Convert";
    }

    QString description() const override {
        return "Conversion of the song from relative to absolute notation";
    }
};

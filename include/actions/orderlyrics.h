#pragma once

#include <song.h>

class Song::OrderLyrics : public Action {
    QList<Sylabel*> back;
public:
    bool perform(Song &song) override;
    bool undo(Song &song) override;
    bool canUndo() const override { return true; }
    bool isIdentity() const override { return false; }

    QString shortName() const override {
        return "Order Notes";
    }

    QString description() const override {
        return "Orders lyrics and notes by timestamp";
    }
};


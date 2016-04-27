#pragma once

#include <QString>
#include <list>
#include <memory>

//ActionItems and thus Actions live strictly as long as their Song but never
//longer. As such they may have a pointer to 'their' song

//action _must_ be undoable
//action and undoAction _must_ either completely fail or completely succeed.
//  they have to return true or false depending on which it is.

class Song;

class Action  {
public:
    virtual ~Action() = default;

    virtual bool perform(Song& song) = 0;
    virtual bool undo(Song& song) = 0;
    virtual QString shortName() const = 0;
    virtual QString description() const  = 0;
};

class ActionItem {
public:
    virtual ~ActionItem() = default;

    enum class Severity {
        Error, Warning, Info
    };

    virtual QString description() const = 0;
    virtual const std::list<std::shared_ptr<Action>>& actions() const = 0;
};

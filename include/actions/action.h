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

    //The default implementation will *not* allow merging. Beware, that merge
    //will only be called with instances of the same class (incl. polymorphic base)
    //this should return true if a merge succeeded
    //merge will always be called on the *older* action with the *newer* action
    //as parameter. See ModifyTag action for an example
    virtual bool merge(const Action&) { return false; }

    //This should return true if undo is possible at all. Else undo will be
    //grayed out
    virtual bool canUndo() const = 0;

    //This returns if the action is the identity (e.g. if it did nothing)
    //If this is the case then the action is automatically deleted
    //Usually an action becomes identity through merging
    virtual bool isIdentity() const = 0;

};

class ActionItem {
public:
    virtual ~ActionItem() = default;

    enum class Severity {
        Error, Warning, Info
    };

    virtual QString description() const = 0;
    virtual Severity severity() const = 0;
    virtual const std::list<std::unique_ptr<Action>>& actions() const = 0;
};

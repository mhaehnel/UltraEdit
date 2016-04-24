#pragma once

#include <QString>
#include <list>

//action _must_ be undoable
//action and undoAction _must_ either completely fail or completely succeed.
//  they have to return true or false depending on which it is.
struct Action  {
    virtual bool action(Song&) = 0;
    virtual bool undoAction(T&) = 0;
    virtual QString shortName() const = 0;
    virtual QString description() const  = 0;
};

struct ActionItem {
    virtual QString description() const ;
    virtual std::list<std::shared_ptr<Action>> actions() const;
};

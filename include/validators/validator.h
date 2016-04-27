#pragma once

#include <actions/action.h>
#include <list>

class Song;

//A validator analyzes a song object based on user visible state
//and may create action items. Action items will get deleted and the song
//re-validated on any change that is triggered
class Validator {
public:
    virtual std::list<std::shared_ptr<ActionItem>> validate(const Song& song) = 0;
};

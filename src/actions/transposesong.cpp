#include "actions/transposesong.h"
#include <song.h>
//TODO: This does not cope if we can not transpose that deeply. There _is_
//a cuttoff on drumstick events that should be handled!

bool Song::TransposeSong::perform(Song *song) {
    for (auto syl : song->sylabels()) {
        syl->transpose(level);
    }
    return true;
}


bool Song::TransposeSong::undo(Song *song) {
    for (auto syl : song->sylabels()) {
        syl->transpose(-level);
    }
    return true;
}

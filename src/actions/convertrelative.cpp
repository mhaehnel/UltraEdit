#include "actions/convertrelative.h"

bool Song::ConvertRelative::perform(Song& song) {
    //First check for uncorrectable problems ...
    if (!song.hasTag("RELATIVE")) return false; //Actually it is already done ...
    for (auto& syl : song.sylabels()) {
       if (syl->type() == Sylabel::Type::SimpleLineBreak) {
           //TODO: deal with this!
           //_fatals << "Can not convert relative file with single number linebreaks!";
           return false;
       }
    }
    int base = 0;
    for (auto& syl : song.sylabels()) {
       syl->move(base);
       base += syl->beats();
    }
    song.tags.remove("RELATIVE");
    return true;
}

//No need to check. If we undo we already checked when doing :)
bool Song::ConvertRelative::undo(Song& song) {
    int sub = 0;
    for (auto& syl : song.musicAndLyrics) {
       syl->move(-sub);
       sub += syl->beats();
    }
    song.addTag("RELATIVE","YES");
    return true;
}

#include <actions/orderlyrics.h>

//TODO: Undo feature relies on naked ptrs in musicAndLyrics ...

bool Song::OrderLyrics::perform(Song &song) {
    back = song.musicAndLyrics;
    std::sort(song.musicAndLyrics.begin(),song.musicAndLyrics.end(),[this] (const Sylabel* s1, const Sylabel* s2) {
        if (s1->beat() == s2->beat()) {
            if (s1->isLineBreak()) return true;
            if (s2->isLineBreak()) return false;
            throw std::runtime_error("can not sort lyrics!");
        }
        return s1->beat() < s2->beat();
    });
    return true;
}

bool Song::OrderLyrics::undo(Song &song) {
    song.musicAndLyrics = back;
    return true;
}

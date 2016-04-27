#include "validators/lyricsinorder.h"
#include <actions/orderlyrics.h>

LyricsInOrder::OrderLyricsAI::OrderLyricsAI(int pos) : pos_(pos) {
    act.push_back(std::make_unique<Song::OrderLyrics>());
}

const std::list<std::unique_ptr<Action>>& LyricsInOrder::OrderLyricsAI::actions() const {
    return act;
}


QString LyricsInOrder::OrderLyricsAI::description() const {
    return QString("The notes in this song are not ordered. While this is "
                   "usually not a problem it is better for program stability "
                   "and clarity to order them by their timestamp. The "
                   "problematic timestamp is after beat %1").arg(pos_);
}

std::list<std::shared_ptr<ActionItem>> LyricsInOrder::validate(const Song &song) {
    bool inOrder = true;
    int prev = 0;
    for (const Sylabel* s : song.sylabels()) {
        if (s->beat() < prev) {
            inOrder = false;
            break;
        }
        prev = s->beat();
    }
    if (!inOrder)
        return {std::make_shared<LyricsInOrder::OrderLyricsAI>(prev)};
    return {};
}

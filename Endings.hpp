#pragma once
#include <string>

enum class EndingType {
    NONE,

    ESCAPE_WHOLE,       // all alive, morale >= 70
    ESCAPE_SCARRED,     // all alive, morale 40–69
    ESCAPE_HOLLOW,      // all alive, morale < 40 (but still > 0)

    ESCAPE_BROKEN,      // partner dead, child alive, morale >= 40
    ESCAPE_LAST_THING,  // child dead, partner alive
    ESCAPE_ALONE,       // both partner and child dead, player survives

    DEAD_COLLAPSE,      // player health hits 0
    DEAD_GAVE_UP,       // player morale hits 0 — psychological death
};

struct EndingCard {
    std::string title;
    std::string body;   
};

inline EndingCard getEndingCard(EndingType e) {
    switch (e) {

    case EndingType::ESCAPE_WHOLE:
        return {
            "YOU MADE IT",
            "The light at the end wasn't a metaphor.\n"
            "You come out blinking, all three of you.\n"
            "Your companion says nothing. Just holds on.\n"
            "The child looks up at the sky for a long time.\n"
            "You can feel the sun on your face. It feels like a new beginning."
        };

    case EndingType::ESCAPE_SCARRED:
        return {
            "YOU MADE IT OUT",
            "You reach the surface. Everyone is alive.\n"
            "Nobody says anything for a while.\n"
            "The tunnels did something to all of you.\n"
            "You're out. That has to be enough.\n"
            "The sun is shining, but it doesn't feel like a new beginning."
            "THE END"
        };

    case EndingType::ESCAPE_HOLLOW:
        return {
            "You Barely Made It",
            "You emerge into grey light. All three of you.\n"
            "You should feel something. You don't.\n"
            "Your companion stares at the horizon.\n"
            "The child doesn't smile anymore. Neither do you.\n"
            "A lot was taken from you in the tunnels.\n"
            "You don't know if it can be repaired.\n"
             "THE END"
        };

    case EndingType::ESCAPE_BROKEN:
        return {
            "YOU CAME OUT. NOT ALL OF YOU.",
            "The child holds your hand on the way up.\n"
            "You let them. You keep walking.\n"
            "You don't talk about what was left behind.\n"
            "You don't know if you ever will.\n"
            "You want to start over, but you don't know how to be whole again.\n"
             "THE END"
        };

    case EndingType::ESCAPE_LAST_THING:
        return {
            "YOU CAME OUT. NOT ALL OF YOU.",
             "Your companion holds your hand on the way up.\n"
            "You and your companion reach the surface.\n"
            "You don't speak. There's nothing to say.\n"
            "You keep moving because stopping is worse.\n"
            "Somewhere in the tunnels, something is still yours.\n"
             "THE END"
        };

    case EndingType::ESCAPE_ALONE:
        return {
            "YOU MADE IT ALONE",
            "You walk out into silence.\n"
            "Nobody is waiting for you.\n"
            "Nobody made it this far with you.\n"
            "You keep walking because that is all there is."
             "THE END"
        };

    case EndingType::DEAD_COLLAPSE:
        return {
            "AND THEN SLOWLY THERE WAS DARKNESS",
            "You don't get back up.\n"
            "The dark takes everything.\n"
            "Nobody finds you for a long time.\n"
            "Maybe nobody finds you at all."
             "THE END"
        };

    case EndingType::DEAD_GAVE_UP:
        return {
            "YOU STOPPED",
            "You were still breathing when it ended.\n"
            "But something had already gone out.\n"
            "You sat down and didn't get up again.\n"
            "The tunnels don't need to kill you. They just need to wait."
             "THE END"
        };

    default:
        return { "THE END", ""};
    }
}
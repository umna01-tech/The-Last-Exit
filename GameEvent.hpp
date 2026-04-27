#pragma once
#include <string>
#include <vector>
#include <functional> // so we can store a function inside a struct

enum class EventType {
    COMBAT,
    RESOURCE,
    MORAL,
    HAZARD,
    ENCOUNTER,
    REST
};

struct Choice {
    std::string text;              // words the player sees on the button
    std::function<void()> action; // without you cant store lambda in the struct
};

struct RoomEvent {
    EventType type;             //What type of event?
    std::string locationName;   //Where does it happen?
    std::string imageFile;      //What image to show in the event?
    std::string narrative;      //What text describes the event?    
    std::vector<Choice> choices;//What options does the player have to choose from?
};

enum class RoomTier { EASY, MEDIUM, HARD };
//DONE
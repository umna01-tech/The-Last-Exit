#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class Radio {
public:
    Radio();

    void update(float dt);
    void draw(sf::RenderWindow& window, const sf::Font& font,
              sf::Vector2f pos, sf::Vector2f size) const;

    bool isBroken() const { return broken; }
    void repair()         { broken = false; staticTimer = 0.f; }
    void nextBroadcast();
    std::string currentBroadcast() const;

private:
    bool   broken;
    float  staticTimer;
    int    broadcastIndex;
    static constexpr float CHECK_INTERVAL = 30.f;

    std::vector<std::string> broadcasts = {
        "ALERT: Bomb detonated downtown. Multiple craters confirmed.",
        "Survivors urged to stay underground. Surface radiation rising.",
        "Emergency services overwhelmed. Hospitals at full capacity.",
        "Looters reported in eastern tunnels. Stay armed and alert.",
        "Government declares martial law. Curfew imposed on all districts.",
        "Rescue teams dispatched to sector 4. ETA unknown.",
        "Water supply contaminated above ground. Do NOT drink it.",
        "Power grid failure citywide. Backup generators failing fast.",
        "Military convoy spotted heading north. Destination unknown.",
        "This is an emergency broadcast. Stay calm. Stay underground.",
        "Radiation levels dropping in western zones. Still dangerous.",
        "Unconfirmed: mutant sightings near old subway lines.",
        "Food drops scheduled for Zone 3. Priority given to families.",
        "All civilians: do NOT approach strangers. Trust no one.",
        "...static... ...signal lost...",
    };
};
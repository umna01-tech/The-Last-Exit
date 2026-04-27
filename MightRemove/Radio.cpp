#include "Radio.hpp"
#include <cstdlib>

Radio::Radio()
    : broken(false), staticTimer(0.f), broadcastIndex(0)
{}

void Radio::update(float dt) {
    if (broken) return;
    staticTimer += dt;
    if (staticTimer >= CHECK_INTERVAL) {
        staticTimer = 0.f;
        // 15% chance to break on each check
        if ((rand() % 100) < 15)
            broken = true;
    }
}

void Radio::nextBroadcast() {
    if (broken) return;
    broadcastIndex = (broadcastIndex + 1) % static_cast<int>(broadcasts.size());
}

std::string Radio::currentBroadcast() const {
    if (broken) return "[RADIO BROKEN - needs repair kit]";
    return broadcasts[broadcastIndex];
}

void Radio::draw(sf::RenderWindow& window, const sf::Font& font,
                 sf::Vector2f pos, sf::Vector2f size) const {
    // Background panel
    sf::RectangleShape bg(size);
    bg.setPosition(pos);
    bg.setFillColor(broken ? sf::Color(40, 20, 20) : sf::Color(20, 30, 20));
    bg.setOutlineColor(broken ? sf::Color(120, 40, 40) : sf::Color(40, 120, 40));
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    // Icon + label
    sf::Text label(font, broken ? "[X] RADIO" : ">>> RADIO", 11);
    label.setFillColor(broken ? sf::Color(180, 60, 60) : sf::Color(60, 200, 80));
    label.setPosition({pos.x + 4.f, pos.y + 4.f});
    window.draw(label);

    // Broadcast text - word wrap manually at ~30 chars
    std::string msg = currentBroadcast();
    // Split into two lines if long
    std::string line1 = msg.substr(0, std::min((int)msg.size(), 32));
    std::string line2 = msg.size() > 32 ? msg.substr(32, 32) : "";

    sf::Text t1(font, line1, 10);
    t1.setFillColor(sf::Color(180, 220, 180));
    t1.setPosition({pos.x + 4.f, pos.y + 20.f});
    window.draw(t1);

    if (!line2.empty()) {
        sf::Text t2(font, line2, 10);
        t2.setFillColor(sf::Color(180, 220, 180));
        t2.setPosition({pos.x + 4.f, pos.y + 34.f});
        window.draw(t2);
    }
}
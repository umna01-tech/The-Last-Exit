#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Inventory.hpp"

// ─────────────────────────────────────────────
//  A single item row in the store
// ─────────────────────────────────────────────
struct StoreItem {
    std::string name;
    int         priceEach;   // cost per unit
    int         maxPerBuy;   // max quantity slider allows
    int         quantity;    // currently selected quantity (1–maxPerBuy)

    // which inventory field to add to
    enum class Type { FOOD, WATER, MEDICINE, AMMO } type;
};

// ─────────────────────────────────────────────
//  Store
// ─────────────────────────────────────────────
class Store {
public:
    Store(sf::Font& font);

    // Call every frame — draws the full store UI
    void draw(sf::RenderWindow& window, const Inventory& inventory);

    // Call from the event loop
    void handleEvent(const sf::Event& event,
                     sf::RenderWindow& window,
                     Inventory& inventory);

    // True once the player clicks "Leave Store"
    bool isDone() const { return done; }
    void reset()        { done = false; }

private:
    sf::Font& font;
    bool      done = false;

    std::vector<StoreItem> items;

    // UI rects built once in the constructor
    struct Row {
        sf::RectangleShape minusBtn;   // "–"
        sf::RectangleShape plusBtn;    // "+"
        sf::RectangleShape buyBtn;     // "BUY"
    };
    std::vector<Row> rows;

    sf::RectangleShape leaveBtn;

    // layout constants
    static constexpr float START_Y    = 160.f;
    static constexpr float ROW_H      = 90.f;
    static constexpr float LEFT_X     = 100.f;
    static constexpr float BTN_W      = 40.f;
    static constexpr float BTN_H      = 40.f;
    static constexpr float BUY_BTN_W  = 100.f;

    void buildRows();
    void tryBuy(int index, Inventory& inventory);

    // helpers
    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color colour = sf::Color::White);
};
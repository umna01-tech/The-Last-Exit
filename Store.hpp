#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Inventory.hpp"

struct StoreItem {
    std::string name;
    int         priceEach;
    int         maxPerBuy;
    int         quantity;   // units currently held in inventory (starts at 0)
    enum class Type { FOOD, WATER, MEDICINE, AMMO } type;
};

class Store {
public:
    explicit Store(sf::Font& font);

    void draw(sf::RenderWindow& window, const Inventory& inventory);
    void handleEvent(const sf::Event& event, sf::RenderWindow& window, Inventory& inventory);

    bool isDone() const { return done; }
    void reset()        { done = false; }

private:
    sf::Font& font;
    bool done = false;

    std::vector<StoreItem> items;

    struct Row {
        sf::RectangleShape minusBtn;
        sf::RectangleShape plusBtn;
    };
    std::vector<Row> rows;

    sf::RectangleShape leaveBtn;

    static constexpr float START_Y = 160.f;
    static constexpr float ROW_H   =  90.f;
    static constexpr float LEFT_X  = 100.f;
    static constexpr float BTN_W   =  40.f;
    static constexpr float BTN_H   =  40.f;

    void buildRows();
    void addOne(int index, Inventory& inventory);
    void removeOne(int index, Inventory& inventory);

    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color colour = sf::Color::White);
};
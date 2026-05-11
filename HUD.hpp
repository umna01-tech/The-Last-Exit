#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Player.hpp"
#include "Partner.hpp"
#include "Child.hpp"
#include "Inventory.hpp"
#include "GameWorld.hpp"

class HUD {
public:
    HUD(sf::Font& font, Player& player, Partner& partner, Child& child,
        Inventory& inventory, GameWorld& world);

    void draw(sf::RenderWindow& window);

    void onMousePressed (sf::Vector2f mp);
    void onMouseMoved   (sf::Vector2f mp);
    void onMouseReleased(sf::Vector2f mp);
    void onScroll       (float delta);
    bool handleCombatClick(sf::Vector2f mp);

private:
    sf::Font&   font;
    Player&     player;
    Partner&    partner;
    Child&      child;
    Inventory&  inventory;
    GameWorld&  world;

    static constexpr float W        = 1280.f;
    static constexpr float H        = 720.f;
    static constexpr float INV_W    = 210.f;
    static constexpr float STATS_W  = 230.f;
    static constexpr float TOP_H    = 44.f;
    static constexpr float CENTER_W = W - INV_W - STATS_W;
    static constexpr float CENTER_H = H - TOP_H;

    struct DragItem {
        bool         active = false;
        std::string  itemName;
        sf::Vector2f pos;
        sf::Vector2f origin;
    };
    DragItem drag;

    struct DropZone {
        sf::FloatRect rect;
        std::string   target;   // "player" | "partner" | "child"
    };
    mutable std::vector<DropZone> dropZones;

    struct InvIcon {
        sf::FloatRect rect;
        std::string   itemName;
    };
    mutable std::vector<InvIcon> invIcons;

    sf::RectangleShape attackAmmoBtn;
    sf::RectangleShape attackHandBtn;
    sf::RectangleShape restBtn;

    void drawTopBar        (sf::RenderWindow& window);
    void drawInventoryPanel(sf::RenderWindow& window);
    void drawCenterPanel   (sf::RenderWindow& window);
    void drawStatsPanel    (sf::RenderWindow& window);
    void drawCombatOverlay (sf::RenderWindow& window);
    void drawEndingScreen  (sf::RenderWindow& window);
    void drawDraggedItem   (sf::RenderWindow& window);

    void drawItemIcon(sf::RenderWindow& window, const std::string& name,
                      sf::Vector2f center, float radius, float alpha = 255.f);

    void drawStatBar(sf::RenderWindow& window, const std::string& label,
                     float value, float maxVal, sf::Vector2f pos, float width,
                     sf::Color color) const;

    void applyDrop(const std::string& item, const std::string& target);

    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col = sf::Color(210, 205, 190)) const;
};
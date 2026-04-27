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
    HUD(sf::Font& font, Player& player, Partner& partner, Child& child, Inventory& inventory, GameWorld& world);

    void draw(sf::RenderWindow& window);

    void onMousePressed (sf::Vector2f mp);  //When clicked
    void onMouseMoved   (sf::Vector2f mp);  //When mouse moves (for dragging)
    void onMouseReleased(sf::Vector2f mp);  //When released (for dropping)
    void onScroll       (float delta);      //for scroll wheel
    bool handleCombatClick(sf::Vector2f mp);//fight buttom clicked

private:
    sf::Font& font;
    Player& player;
    Partner& partner;
    Child& child;
    Inventory& inventory;
    GameWorld& world;

    // ── Layout ────────────────────────────────────────────────────────────────
    static constexpr float W        = 1280.f;
    static constexpr float H        = 720.f;
    static constexpr float INV_W    = 210.f;
    static constexpr float STATS_W  = 230.f;
    static constexpr float TOP_H    = 44.f;
    static constexpr float CENTER_W = W - INV_W - STATS_W;
    static constexpr float CENTER_H = H - TOP_H;

    // ── Drag state ────────────────────────────────────────────────────────────
    struct DragItem {
        bool        active   = false;
        std::string itemName;          // "Food","Water","Medicine"
        sf::Vector2f pos;              // current drag position
        sf::Vector2f origin;           // where drag started
    };
    DragItem drag;

    // Drop zones per character — built each draw
    struct DropZone {
        sf::FloatRect rect;
        std::string   target; // "player","partner","child"
    };
    mutable std::vector<DropZone> dropZones;

    // Inventory icon rects — built each draw (for drag start detection)
    struct InvIcon {
        sf::FloatRect rect;
        std::string   itemName;
    };
    mutable std::vector<InvIcon> invIcons;

    // ── Combat buttons ────────────────────────────────────────────────────────
    sf::RectangleShape attackAmmoBtn;
    sf::RectangleShape attackHandBtn;
    // ── REST button ───────────────────────────────────────────────────────────
    sf::RectangleShape restBtn;

    // ── Draw sections ─────────────────────────────────────────────────────────
    void drawTopBar       (sf::RenderWindow& window);
    void drawInventoryPanel(sf::RenderWindow& window);
    void drawCenterPanel  (sf::RenderWindow& window);
    void drawStatsPanel   (sf::RenderWindow& window);
    void drawCombatOverlay(sf::RenderWindow& window);
    void drawEndingScreen (sf::RenderWindow& window);
    void drawDraggedItem  (sf::RenderWindow& window);

    // Draw a single inventory icon (food/water/med/ammo picture)
    void drawItemIcon(sf::RenderWindow& window, const std::string& name,
                      sf::Vector2f center, float radius, float alpha = 255.f);

    void drawStatBar(sf::RenderWindow& window, const std::string& label,
                     float value, float maxVal, sf::Vector2f pos, float width,
                     sf::Color color) const;

    void applyDrop(const std::string& item, const std::string& target);

    sf::Text makeText(const std::string& str, unsigned size,
                      sf::Color col = sf::Color(210, 205, 190)) const;
};
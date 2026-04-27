#include "HUD.hpp"
#include "Player.hpp"
#include "Partner.hpp"
#include "Child.hpp"
#include "Inventory.hpp"
#include "GameWorld.hpp"
#include "Enemy.hpp"
#include "Entity.hpp"
#include <cmath>

HUD::HUD(sf::Font& font, Player& player, Partner& partner, Child& child, Inventory& inventory, GameWorld& world)
    : font(font), player(player), partner(partner), child(child) , inventory(inventory), world(world)
{
    // Combat 
    float cbx = INV_W + CENTER_W * 0.5f - 110.f;
    float cby = H - 68.f;
    attackAmmoBtn.setSize({100.f, 38.f});
    attackAmmoBtn.setPosition({cbx, cby});
    attackAmmoBtn.setFillColor(sf::Color(120, 40, 40));

    attackHandBtn.setSize({100.f, 38.f});
    attackHandBtn.setPosition({cbx + 118.f, cby});
    attackHandBtn.setFillColor(sf::Color(80, 60, 20));

    // Rest button — bottom of inventory panel
    restBtn.setSize({INV_W - 16.f, 36.f});
    restBtn.setPosition({8.f, H - 44.f});
    restBtn.setFillColor(sf::Color(28, 50, 28));
    restBtn.setOutlineColor(sf::Color(50, 120, 50));
    restBtn.setOutlineThickness(1.f);
}

void HUD::draw(sf::RenderWindow& window) {
    //If gameOver just draw ending screen and return
    if (world.isFinished()) { 
        drawEndingScreen(window); 
        return; 
    }

    dropZones.clear();
    invIcons.clear();

    drawTopBar(window);
    drawInventoryPanel(window);
    drawCenterPanel(window);
    drawStatsPanel(window);

    if (world.inCombat()) 
    {
        drawCombatOverlay(window);
    }
    if (drag.active) 
    {
        drawDraggedItem(window);
    }
}

void HUD::drawTopBar(sf::RenderWindow& window) {
    sf::RectangleShape bar({W, TOP_H});
    bar.setPosition({0.f, 0.f});
    bar.setFillColor(sf::Color(10, 10, 18));
    bar.setOutlineColor(sf::Color(40, 40, 70));
    bar.setOutlineThickness(1.f);
    window.draw(bar);
}

void HUD::drawInventoryPanel(sf::RenderWindow& window) {
    sf::RectangleShape bg({INV_W, CENTER_H});
    bg.setPosition({0.f, TOP_H});
    bg.setFillColor(sf::Color(10, 12, 18));
    bg.setOutlineColor(sf::Color(50, 50, 80));
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    auto title = makeText("INVENTORY", 20, sf::Color(150, 150, 200));
    title.setPosition({8.f, TOP_H + 6.f});
    window.draw(title);

    auto hint = makeText("drag to give", 10, sf::Color(80, 80, 110));
    hint.setPosition({100.f, TOP_H + 8.f});
    window.draw(hint);

    sf::RectangleShape div({INV_W - 4.f, 1.f});
    div.setPosition({2.f, TOP_H + 26.f});
    div.setFillColor(sf::Color(50, 50, 80));
    window.draw(div);

    struct ResRow { std::string name; int count; sf::Color col; };
    std::vector<ResRow> rows = {
        {"Food",     inventory.getFood(),     sf::Color(220, 170, 60)},
        {"Water",    inventory.getWater(),    sf::Color(60,  150, 230)},
        {"Medicine", inventory.getMedicine(), sf::Color(60,  210, 120)},
        {"Ammo",     inventory.getAmmo(),     sf::Color(210, 80,  80)},
    };

    float iy = TOP_H + 36.f;
    float iconR = 24.f; // icon radius

    for (auto& row : rows) {
        // Section background
        sf::RectangleShape rowBg({INV_W - 8.f, 70.f});
        rowBg.setPosition({4.f, iy});
        rowBg.setFillColor(sf::Color(16, 18, 28));
        rowBg.setOutlineColor(sf::Color(40, 40, 65));
        rowBg.setOutlineThickness(1.f);
        window.draw(rowBg);

        // Icon (drawn as colored shape — acts as drag handle)
        sf::Vector2f iconCenter = {4.f + iconR + 4.f, iy + 35.f};
        drawItemIcon(window, row.name, iconCenter, iconR);

        // Register as draggable
        if (row.name != "Ammo") {
            InvIcon ic;
            ic.rect = sf::FloatRect({iconCenter.x - iconR, iconCenter.y - iconR}, {iconR*2.f, iconR*2.f});
            ic.itemName = row.name;
            invIcons.push_back(ic);
        }

        auto countT = makeText("x" + std::to_string(row.count), 15, row.col);
        countT.setPosition({4.f + iconR*2 + 12.f, iy + 10.f});
        window.draw(countT);

        // Item name
        auto nameT = makeText(row.name, 12, sf::Color(160, 160, 180));
        nameT.setPosition({4.f + iconR*2 + 12.f, iy + 30.f});
        window.draw(nameT);

        // Ammo note
        if (row.name == "Ammo") {
            auto ammoNote = makeText("player only\nNot draggable", 10, sf::Color(100, 100, 120));
            ammoNote.setPosition({4.f + iconR*2 + 12.f, iy + 48.f});
            window.draw(ammoNote);
        }

        iy += 76.f;
    }

    // ── REST button ───────────────────────────────────────────────────────────
    bool canRest = (inventory.getFood() >= 9 && inventory.getWater() >= 9);
    restBtn.setFillColor(canRest ? sf::Color(22, 48, 22) : sf::Color(22, 28, 22));
    restBtn.setOutlineColor(canRest ? sf::Color(50, 130, 50) : sf::Color(40, 55, 40));
    window.draw(restBtn);

    auto restL = makeText(canRest ? "REST (9 food + 9 water)" : "REST (need 9 food+water)",
                          11, canRest ? sf::Color(100, 220, 100) : sf::Color(80, 100, 80));
    restL.setPosition({12.f, H - 37.f});
    window.draw(restL);
}

// ─── Center panel ─────────────────────────────────────────────────────────────

void HUD::drawCenterPanel(sf::RenderWindow& window) {
    sf::RectangleShape bg({CENTER_W, CENTER_H});
    bg.setPosition({INV_W, TOP_H});
    bg.setFillColor(sf::Color(8, 8, 14));
    bg.setOutlineColor(sf::Color(40, 40, 70));
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    world.draw(window, font, {INV_W + 4.f, TOP_H + 4.f},
               {CENTER_W - 8.f, CENTER_H - 8.f});
}

// ─── Stats panel ──────────────────────────────────────────────────────────────

void HUD::drawStatsPanel(sf::RenderWindow& window) {
    float px = INV_W + CENTER_W;
    float py = TOP_H;
    float barW = STATS_W - 22.f;

    sf::RectangleShape bg({STATS_W, CENTER_H});
    bg.setPosition({px, py});
    bg.setFillColor(sf::Color(10, 10, 18));
    bg.setOutlineColor(sf::Color(50, 50, 80));
    bg.setOutlineThickness(1.f);
    window.draw(bg);

    float y = py + 8.f;

    // Helper: draw one character's stats block + drop zone
    auto drawCharBlock = [&](const std::string& label, int hp, int food, int water,
                              bool alive, sf::Color nameCol,
                              const std::string& targetKey, float blockH,
                              const std::string& status) {
        sf::RectangleShape block({STATS_W - 6.f, blockH});
        block.setPosition({px + 3.f, y});
        block.setFillColor(sf::Color(14, 14, 24));
        block.setOutlineColor(alive ? sf::Color(55,55,90) : sf::Color(100,30,30));
        block.setOutlineThickness(1.f);
        window.draw(block);

        // Register as drop zone
        DropZone dz;
        dz.rect   = block.getGlobalBounds();
        dz.target = targetKey;
        dropZones.push_back(dz);

        // Highlight if dragging over
        if (drag.active && block.getGlobalBounds().contains(drag.pos)) {
            sf::RectangleShape hl({STATS_W - 6.f, blockH});
            hl.setPosition({px + 3.f, y});
            hl.setFillColor(sf::Color(60, 100, 60, 80));
            hl.setOutlineColor(sf::Color(80, 200, 80));
            hl.setOutlineThickness(2.f);
            window.draw(hl);
        }

        auto nm = makeText(label + (alive ? "" : "  [DEAD]"), 13, alive ? nameCol : sf::Color(150,50,50));
        nm.setStyle(sf::Text::Bold);
        nm.setPosition({px + 8.f, y + 5.f});
        window.draw(nm);
        y += 22.f;

        drawStatBar(window, "HP", (float)hp, (float)Entity::MAX_HEALTH, {px+8.f,y}, barW, sf::Color(200,60,60)); y+=17.f;

        sf::Color statusCol = (status == "Critical") ? sf::Color(220,60,60) :
                              (status == "Tired")    ? sf::Color(220,180,60) :
                                                       sf::Color(80,180,80);
        auto statusT = makeText(status, 9, statusCol);
        statusT.setPosition({px + 8.f, y});
        window.draw(statusT);
        y += 12.f;

        drawStatBar(window, "Food",  (float)food,  (float)Entity::MAX_FOOD,   {px+8.f,y}, barW, sf::Color(200,160,60)); y+=17.f;
        drawStatBar(window, "Water", (float)water, (float)Entity::MAX_WATER,  {px+8.f,y}, barW, sf::Color(60,140,220)); y+=17.f;
        y += 6.f;
    };

    // Child
    drawCharBlock("COMPANION (young)",
                  child.getHealth(), child.getFood(), child.getWater(),
                  child.isAlive(), sf::Color(180, 220, 255), "child", 100.f,
                  child.getStatus());

    // Partner
    drawCharBlock("COMPANION",
                  partner.getHealth(), partner.getFood(), partner.getWater(),
                  partner.isAlive(), sf::Color(220, 180, 255), "partner", 100.f,
                  partner.getStatus());

    // Player block
    float playerBlockH = 124.f;
    sf::RectangleShape playerBlock({STATS_W - 6.f, playerBlockH});
    playerBlock.setPosition({px + 3.f, y});
    playerBlock.setFillColor(sf::Color(12, 14, 26));
    playerBlock.setOutlineColor(sf::Color(80, 80, 150));
    playerBlock.setOutlineThickness(2.f);
    window.draw(playerBlock);

    DropZone pdz; pdz.rect = playerBlock.getGlobalBounds(); pdz.target = "player";
    dropZones.push_back(pdz);

    if (drag.active && playerBlock.getGlobalBounds().contains(drag.pos)) {
        sf::RectangleShape hl({STATS_W - 6.f, playerBlockH});
        hl.setPosition({px + 3.f, y});
        hl.setFillColor(sf::Color(60, 100, 60, 80));
        hl.setOutlineColor(sf::Color(80, 200, 80));
        hl.setOutlineThickness(2.f);
        window.draw(hl);
    }

    auto pnm = makeText("YOU", 14, sf::Color(255, 220, 100));
    pnm.setStyle(sf::Text::Bold);
    pnm.setPosition({px + 8.f, y + 5.f});
    window.draw(pnm);
    y += 22.f;

    drawStatBar(window, "HP",    (float)player.getHealth(), (float)Entity::MAX_HEALTH, {px+8.f,y}, barW, sf::Color(200,60,60));  y+=17.f;

    sf::Color psCol = (player.getStatus() == "Critical") ? sf::Color(220,60,60) :
                      (player.getStatus() == "Tired")    ? sf::Color(220,180,60) :
                                                            sf::Color(80,180,80);
    auto psT = makeText(player.getStatus(), 9, psCol);
    psT.setPosition({px + 8.f, y});
    window.draw(psT);
    y += 12.f;

    drawStatBar(window, "Food",   (float)player.getFood(),   (float)Entity::MAX_FOOD,   {px+8.f,y}, barW, sf::Color(200,160,60)); y+=17.f;
    drawStatBar(window, "Water",  (float)player.getWater(),  (float)Entity::MAX_WATER,  {px+8.f,y}, barW, sf::Color(60,140,220)); y+=17.f;
    drawStatBar(window, "Morale", (float)player.getMorale(), (float)Player::MAX_MORALE, {px+8.f,y}, barW, sf::Color(160,80,200)); y+=17.f;

    auto ammoT = makeText("Ammo: " + std::to_string(inventory.getAmmo()), 12, sf::Color(200,100,100));
    ammoT.setPosition({px + 8.f, y + 2.f});
    window.draw(ammoT);
}

// ─── Combat overlay ───────────────────────────────────────────────────────────

void HUD::drawCombatOverlay(sf::RenderWindow& window) {
    if (!world.currentEnemy()) return;
    Enemy* enemy = world.currentEnemy();

    sf::RectangleShape tint({CENTER_W, 80.f});
    tint.setPosition({INV_W, H - 80.f});
    tint.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(tint);

    attackAmmoBtn.setFillColor(inventory.hasAmmo() ? sf::Color(140,40,40) : sf::Color(50,28,28));
    window.draw(attackAmmoBtn);
    auto aL = makeText(inventory.hasAmmo() ? "SHOOT (10)" : "NO AMMO", 12);
    aL.setPosition(attackAmmoBtn.getPosition() + sf::Vector2f(6.f, 11.f));
    window.draw(aL);

    window.draw(attackHandBtn);
    auto hL = makeText("PUNCH (5)", 12);
    hL.setPosition(attackHandBtn.getPosition() + sf::Vector2f(6.f, 11.f));
    window.draw(hL);

    auto eS = makeText("ENEMY: " + enemy->getStatus(), 14, sf::Color(255,100,100));
    eS.setPosition({INV_W + CENTER_W * 0.5f - 55.f, H - 82.f});
    window.draw(eS);
}

// ─── Draw item icon (colored SVG-style shape) ─────────────────────────────────

void HUD::drawItemIcon(sf::RenderWindow& window, const std::string& name,
                        sf::Vector2f center, float r, float alpha) {
    uint8_t a = static_cast<uint8_t>(alpha);

    if (name == "Food") {
        // Orange circle (bread/can)
        sf::CircleShape outer(r);
        outer.setOrigin({r, r});
        outer.setPosition(center);
        outer.setFillColor(sf::Color(200, 140, 40, a));
        window.draw(outer);
        sf::CircleShape inner(r * 0.5f);
        inner.setOrigin({r*0.5f, r*0.5f});
        inner.setPosition(center);
        inner.setFillColor(sf::Color(240, 190, 80, a));
        window.draw(inner);
        sf::Text t(font, "F", (unsigned)(r * 0.9f));
        t.setFillColor(sf::Color(80, 40, 0, a));
        sf::FloatRect tb = t.getLocalBounds();
        t.setOrigin({tb.size.x/2.f, tb.size.y/2.f});
        t.setPosition(center + sf::Vector2f(0.f, -2.f));
        window.draw(t);
    }
    else if (name == "Water") {
        // Blue teardrop-ish (circle)
        sf::CircleShape outer(r);
        outer.setOrigin({r, r});
        outer.setPosition(center);
        outer.setFillColor(sf::Color(40, 120, 210, a));
        window.draw(outer);
        sf::CircleShape inner(r * 0.5f);
        inner.setOrigin({r*0.5f, r*0.5f});
        inner.setPosition(center);
        inner.setFillColor(sf::Color(100, 180, 255, a));
        window.draw(inner);
        sf::Text t(font, "W", (unsigned)(r * 0.9f));
        t.setFillColor(sf::Color(0, 40, 100, a));
        sf::FloatRect tb = t.getLocalBounds();
        t.setOrigin({tb.size.x/2.f, tb.size.y/2.f});
        t.setPosition(center + sf::Vector2f(0.f, -2.f));
        window.draw(t);
    }
    else if (name == "Medicine") {
        // Green cross shape (two rectangles)
        sf::RectangleShape bg({r*2.f, r*2.f});
        bg.setOrigin({r, r});
        bg.setPosition(center);
        bg.setFillColor(sf::Color(30, 160, 80, a));
        window.draw(bg);
        sf::RectangleShape hBar({r*1.6f, r*0.6f});
        hBar.setOrigin({r*0.8f, r*0.3f});
        hBar.setPosition(center);
        hBar.setFillColor(sf::Color(220, 255, 220, a));
        window.draw(hBar);
        sf::RectangleShape vBar({r*0.6f, r*1.6f});
        vBar.setOrigin({r*0.3f, r*0.8f});
        vBar.setPosition(center);
        vBar.setFillColor(sf::Color(220, 255, 220, a));
        window.draw(vBar);
    }
    else if (name == "Ammo") {
        // Dark red bullet-ish shape
        sf::RectangleShape bg({r*1.4f, r*2.f});
        bg.setOrigin({r*0.7f, r});
        bg.setPosition(center);
        bg.setFillColor(sf::Color(160, 50, 50, a));
        window.draw(bg);
        sf::CircleShape tip(r * 0.7f);
        tip.setOrigin({r*0.7f, r*0.7f});
        tip.setPosition(center + sf::Vector2f(0.f, -r*0.7f));
        tip.setFillColor(sf::Color(220, 180, 60, a));
        window.draw(tip);
        sf::Text t(font, "A", (unsigned)(r * 0.8f));
        t.setFillColor(sf::Color(255, 200, 200, a));
        sf::FloatRect tb = t.getLocalBounds();
        t.setOrigin({tb.size.x/2.f, tb.size.y/2.f});
        t.setPosition(center + sf::Vector2f(0.f, 4.f));
        window.draw(t);
    }
}

// ─── Dragged item rendering ───────────────────────────────────────────────────

void HUD::drawDraggedItem(sf::RenderWindow& window) {
    if (!drag.active) return;
    drawItemIcon(window, drag.itemName, drag.pos, 28.f, 200.f);

    // Hint text under icon
    auto hint = makeText("drop on character", 10, sf::Color(180, 180, 180));
    hint.setPosition({drag.pos.x - 45.f, drag.pos.y + 32.f});
    window.draw(hint);
}

// ─── Input handlers ───────────────────────────────────────────────────────────

void HUD::onMousePressed(sf::Vector2f mp) {
    // Top bar
    if (restBtn.getGlobalBounds().contains(mp)) {
        world.doRest();
        return;
    }

    if (world.inCombat()) {
        handleCombatClick(mp);
        return;
    }

    int choice = world.handleClick(mp);
    if (choice >= 0) {
        world.fireChoice(choice);
        return;
    }

    if (world.isChoiceMade() && !world.inCombat()) {
        sf::FloatRect centerArea({INV_W, TOP_H}, {CENTER_W, CENTER_H});
        if (centerArea.contains(mp)) {
            world.enterNextRoom();
            return;
        }
    }

    // Start drag from inventory icons
    for (auto& ic : invIcons) {
        if (ic.rect.contains(mp)) {
            // Check we actually have the item
            bool hasIt = false;
            if      (ic.itemName == "Food")     hasIt = inventory.hasFood();
            else if (ic.itemName == "Water")    hasIt = inventory.hasWater();
            else if (ic.itemName == "Medicine") hasIt = inventory.hasMed();
            else if (ic.itemName == "Ammo")     hasIt = inventory.hasAmmo();

            if (hasIt) {
                drag.active   = true;
                drag.itemName = ic.itemName;
                drag.pos      = mp;
                drag.origin   = mp;
            }
            return;
        }
    }
}

void HUD::onMouseMoved(sf::Vector2f mp) {
    if (drag.active) drag.pos = mp;
}

void HUD::onMouseReleased(sf::Vector2f mp) {
    if (!drag.active) return;

    // Check drop zones
    for (auto& dz : dropZones) {
        if (dz.rect.contains(mp)) {
            applyDrop(drag.itemName, dz.target);
            break;
        }
    }

    drag.active = false;
}

// ─── Apply a drag-drop give ───────────────────────────────────────────────────

void HUD::applyDrop(const std::string& item, const std::string& target) {
    // Ammo only goes to player
    if (item == "Ammo" && target != "player") return;

    Entity* ent = nullptr;
    if      (target == "player")  ent = &player;
    else if (target == "partner") ent = &partner;
    else if (target == "child")   ent = &child;
    if (!ent) return;

    if      (item == "Food")     inventory.useFood(*ent);
    else if (item == "Water")    inventory.useWater(*ent);
    else if (item == "Medicine") inventory.useMedicine(*ent);
    else if (item == "Ammo")     inventory.useAmmo();
}

// ─── Combat click ─────────────────────────────────────────────────────────────

bool HUD::handleCombatClick(sf::Vector2f mp) {
    if (!world.inCombat() || !world.currentEnemy()) return false;
    Enemy* enemy = world.currentEnemy();

    if (attackAmmoBtn.getGlobalBounds().contains(mp)) {
        if (inventory.hasAmmo()) { inventory.removeAmmo(1); enemy->takeDamage(10); }
        else                     { enemy->takeDamage(5); player.takeDamage(2); }
        if (!enemy->isDead()) enemy->attack(player);
        else                  world.endCombat();
        return true;
    }
    if (attackHandBtn.getGlobalBounds().contains(mp)) {
        enemy->takeDamage(5); player.takeDamage(2);
        if (!enemy->isDead()) enemy->attack(player);
        else                  world.endCombat();
        return true;
    }
    return false;
}

// ─── Toggles ──────────────────────────────────────────────────────────────────
void HUD::onScroll(float delta) { (void)delta; }

// ─── Stat bar ─────────────────────────────────────────────────────────────────

void HUD::drawStatBar(sf::RenderWindow& window, const std::string& label,
                       float value, float maxVal, sf::Vector2f pos, float width,
                       sf::Color color) const {
    float barH = 11.f;
    sf::RectangleShape bg({width, barH});
    bg.setPosition(pos); bg.setFillColor(sf::Color(28,28,40)); window.draw(bg);
    float ratio = maxVal > 0.f ? value / maxVal : 0.f;
    sf::RectangleShape fill({width * ratio, barH});
    fill.setPosition(pos); fill.setFillColor(color); window.draw(fill);
    sf::Text lbl(font, label, 9);
    lbl.setFillColor(sf::Color(170,170,170));
    lbl.setPosition({pos.x + 2.f, pos.y});
    window.draw(lbl);
}

// ─── Ending screen ────────────────────────────────────────────────────────────

void HUD::drawEndingScreen(sf::RenderWindow& window) {
    window.clear(sf::Color(4, 4, 8));

    std::string heading, body;
    sf::Color   headCol;

    GameWorld::EndingType ending = world.getEnding();

    if (ending == GameWorld::EndingType::GOOD) {
        heading = "ENDING I — YOU MADE IT";
        body    = "You stepped into the light.\n"
                  "Battered, hungry — but here.\n\n"
                  "The city above was gone. What came next\n"
                  "was anyone's guess. But you got out.";
        headCol = sf::Color(100, 220, 100);
    } 
    else if (ending == GameWorld::EndingType::MORALE) {
        heading = "ENDING II — BROKEN";
        body    = "You reached the surface. Barely.\n"
                  "But the weight of every choice had cost something.\n\n"
                  "Nobody ever found out what happened to them after that.";
        headCol = sf::Color(150, 100, 200);
    } 
    else if (ending == GameWorld::EndingType::DEAD) {
        heading = "GAME OVER";
        body    = "The tunnels took you.\n"
                  "Like they've taken everyone else who came down here.\n\n"
                  "Nobody made it to the surface.";
        headCol = sf::Color(200, 40, 40);
    }
    else {
        return;
    }

    sf::Text h(font, heading, 46); h.setFillColor(headCol); h.setStyle(sf::Text::Bold);
    sf::FloatRect hb = h.getLocalBounds(); h.setOrigin({hb.size.x/2.f,0.f}); h.setPosition({640.f,190.f});
    window.draw(h);

    sf::Text b(font, body, 20); b.setFillColor(sf::Color(200,195,180));
    sf::FloatRect bb = b.getLocalBounds(); b.setOrigin({bb.size.x/2.f,0.f}); b.setPosition({640.f,310.f});
    window.draw(b);

    sf::Text p(font, "Survival Underground", 14); p.setFillColor(sf::Color(80,80,100));
    sf::FloatRect pb = p.getLocalBounds(); p.setOrigin({pb.size.x/2.f,0.f}); p.setPosition({640.f,590.f});
    window.draw(p);
}

// ─── makeText ─────────────────────────────────────────────────────────────────

sf::Text HUD::makeText(const std::string& str, unsigned size, sf::Color col) const {
    sf::Text t(font, str, size);
    t.setFillColor(col);
    return t;
}
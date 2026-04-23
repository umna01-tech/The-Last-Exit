#include "Store.hpp"
#include <stdexcept>

// ─────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────
Store::Store(sf::Font& font) : font(font) {
    // Define the four items
    items = {
        { "Food",     20,  10, 1, StoreItem::Type::FOOD     },
        { "Water",    15,  10, 1, StoreItem::Type::WATER    },
        { "Medicine", 50,   5, 1, StoreItem::Type::MEDICINE },
        { "Ammo",     10,  20, 1, StoreItem::Type::AMMO     },
    };

    buildRows();

    // Leave button
    leaveBtn.setSize({200.f, 50.f});
    leaveBtn.setPosition({540.f, 620.f});
    leaveBtn.setFillColor(sf::Color(160, 40, 40));
}

// ─────────────────────────────────────────────
//  Build row UI rects
// ─────────────────────────────────────────────
void Store::buildRows() {
    rows.clear();
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        float y = START_Y + i * ROW_H;

        Row r;

        // "–" button
        r.minusBtn.setSize({BTN_W, BTN_H});
        r.minusBtn.setPosition({LEFT_X + 420.f, y + 10.f});
        r.minusBtn.setFillColor(sf::Color(80, 80, 80));

        // "+" button
        r.plusBtn.setSize({BTN_W, BTN_H});
        r.plusBtn.setPosition({LEFT_X + 520.f, y + 10.f});
        r.plusBtn.setFillColor(sf::Color(80, 80, 80));

        // "BUY" button
        r.buyBtn.setSize({BUY_BTN_W, BTN_H});
        r.buyBtn.setPosition({LEFT_X + 600.f, y + 10.f});
        r.buyBtn.setFillColor(sf::Color(40, 140, 40));

        rows.push_back(r);
    }
}

// ─────────────────────────────────────────────
//  Handle input
// ─────────────────────────────────────────────
void Store::handleEvent(const sf::Event& event,
                        sf::RenderWindow& window,
                        Inventory& inventory)
{
    const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>();
    if (!mouse || mouse->button != sf::Mouse::Button::Left)
        return;

    sf::Vector2f mpos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // Leave button
    if (leaveBtn.getGlobalBounds().contains(mpos)) {
        done = true;
        return;
    }

    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        // "–": decrease quantity, min 1
        if (rows[i].minusBtn.getGlobalBounds().contains(mpos)) {
            if (items[i].quantity > 1)
                items[i].quantity--;
        }
        // "+": increase quantity, max = maxPerBuy
        else if (rows[i].plusBtn.getGlobalBounds().contains(mpos)) {
            if (items[i].quantity < items[i].maxPerBuy)
                items[i].quantity++;
        }
        // "BUY"
        else if (rows[i].buyBtn.getGlobalBounds().contains(mpos)) {
            tryBuy(i, inventory);
        }
    }
}

// ─────────────────────────────────────────────
//  Try to buy selected quantity of item i
// ─────────────────────────────────────────────
void Store::tryBuy(int i, Inventory& inventory) {
    int totalCost = items[i].priceEach * items[i].quantity;

    if (!inventory.canAfford(totalCost))
        return;   // not enough money — could flash a warning here

    inventory.spendMoney(totalCost);

    switch (items[i].type) {
        case StoreItem::Type::FOOD:     inventory.addFood    (items[i].quantity); break;
        case StoreItem::Type::WATER:    inventory.addWater   (items[i].quantity); break;
        case StoreItem::Type::MEDICINE: inventory.addMedicine(items[i].quantity); break;
        case StoreItem::Type::AMMO:     inventory.addAmmo    (items[i].quantity); break;
    }
}

// ─────────────────────────────────────────────
//  Draw
// ─────────────────────────────────────────────
void Store::draw(sf::RenderWindow& window, const Inventory& inventory) {
    // ── Background ────────────────────────────
    window.clear(sf::Color(15, 15, 25));

    // ── Title ─────────────────────────────────
    auto title = makeText("SUPPLY STORE", 42, sf::Color(220, 180, 80));
    title.setPosition({430.f, 60.f});
    window.draw(title);

    // ── Money display ─────────────────────────
    auto moneyTxt = makeText("Money: $" + std::to_string(inventory.getMoney()), 24);
    moneyTxt.setPosition({LEFT_X, 110.f});
    window.draw(moneyTxt);

    // ── Column headers ────────────────────────
    float headerY = START_Y - 30.f;
    for (const std::string& col : {"ITEM", "PRICE / UNIT", "MAX", "QTY", "COST", "", "", "BUY"}) {
        (void)col; // drawn manually below to control x positions
    }
    auto hItem  = makeText("ITEM",        18, sf::Color(150,150,150)); hItem .setPosition({LEFT_X,          headerY}); window.draw(hItem);
    auto hPrice = makeText("$/UNIT",      18, sf::Color(150,150,150)); hPrice.setPosition({LEFT_X + 220.f,  headerY}); window.draw(hPrice);
    auto hMax   = makeText("MAX",         18, sf::Color(150,150,150)); hMax  .setPosition({LEFT_X + 320.f,  headerY}); window.draw(hMax);
    auto hQty   = makeText("QTY",         18, sf::Color(150,150,150)); hQty  .setPosition({LEFT_X + 460.f,  headerY}); window.draw(hQty);
    auto hCost  = makeText("TOTAL COST",  18, sf::Color(150,150,150)); hCost .setPosition({LEFT_X + 720.f,  headerY}); window.draw(hCost);

    // ── Divider ───────────────────────────────
    sf::RectangleShape divider({1080.f, 2.f});
    divider.setPosition({LEFT_X, START_Y - 8.f});
    divider.setFillColor(sf::Color(60, 60, 80));
    window.draw(divider);

    // ── Item rows ─────────────────────────────
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        const StoreItem& item = items[i];
        float y = START_Y + i * ROW_H;

        // Alternating row background
        if (i % 2 == 0) {
            sf::RectangleShape rowBg({1080.f, ROW_H - 4.f});
            rowBg.setPosition({LEFT_X, y});
            rowBg.setFillColor(sf::Color(25, 25, 40));
            window.draw(rowBg);
        }

        // Name
        auto nameTxt = makeText(item.name, 26);
        nameTxt.setPosition({LEFT_X + 10.f, y + 22.f});
        window.draw(nameTxt);

        // Price per unit
        auto priceTxt = makeText("$" + std::to_string(item.priceEach), 22, sf::Color(100, 220, 100));
        priceTxt.setPosition({LEFT_X + 220.f, y + 24.f});
        window.draw(priceTxt);

        // Max per purchase
        auto maxTxt = makeText("x" + std::to_string(item.maxPerBuy), 22, sf::Color(180,180,180));
        maxTxt.setPosition({LEFT_X + 320.f, y + 24.f});
        window.draw(maxTxt);

        // "–" button
        window.draw(rows[i].minusBtn);
        auto minusLbl = makeText("-", 26); minusLbl.setPosition({LEFT_X + 427.f, y + 8.f}); window.draw(minusLbl);

        // Quantity
        auto qtyTxt = makeText(std::to_string(item.quantity), 24);
        qtyTxt.setPosition({LEFT_X + 476.f, y + 22.f});
        window.draw(qtyTxt);

        // "+" button
        window.draw(rows[i].plusBtn);
        auto plusLbl = makeText("+", 26); plusLbl.setPosition({LEFT_X + 527.f, y + 8.f}); window.draw(plusLbl);

        // BUY button — grey out if can't afford
        int totalCost = item.priceEach * item.quantity;
        bool canAfford = inventory.canAfford(totalCost);
        rows[i].buyBtn.setFillColor(canAfford ? sf::Color(40, 140, 40) : sf::Color(80, 80, 80));
        window.draw(rows[i].buyBtn);
        auto buyLbl = makeText("BUY", 20, canAfford ? sf::Color::White : sf::Color(130,130,130));
        buyLbl.setPosition({LEFT_X + 618.f, y + 18.f});
        window.draw(buyLbl);

        // Total cost
        auto costTxt = makeText("$" + std::to_string(totalCost), 22,
                                canAfford ? sf::Color(220, 180, 80) : sf::Color(180, 80, 80));
        costTxt.setPosition({LEFT_X + 720.f, y + 24.f});
        window.draw(costTxt);

        // Row divider
        sf::RectangleShape rowDiv({1080.f, 1.f});
        rowDiv.setPosition({LEFT_X, y + ROW_H - 2.f});
        rowDiv.setFillColor(sf::Color(40, 40, 60));
        window.draw(rowDiv);
    }

    // ── Current inventory summary ─────────────
    float summaryY = START_Y + items.size() * ROW_H + 20.f;
    auto summaryTitle = makeText("YOUR INVENTORY", 20, sf::Color(150,150,150));
    summaryTitle.setPosition({LEFT_X, summaryY});
    window.draw(summaryTitle);

    auto foodTxt = makeText("Food: "     + std::to_string(inventory.getFood()),     18, sf::Color(200,200,200));
    auto waterTxt= makeText("Water: "    + std::to_string(inventory.getWater()),    18, sf::Color(200,200,200));
    auto medTxt  = makeText("Medicine: " + std::to_string(inventory.getMedicine()), 18, sf::Color(200,200,200));
    auto ammoTxt = makeText("Ammo: "     + std::to_string(inventory.getAmmo()),     18, sf::Color(200,200,200));

    foodTxt .setPosition({LEFT_X,          summaryY + 28.f}); window.draw(foodTxt);
    waterTxt.setPosition({LEFT_X + 160.f,  summaryY + 28.f}); window.draw(waterTxt);
    medTxt  .setPosition({LEFT_X + 320.f,  summaryY + 28.f}); window.draw(medTxt);
    ammoTxt .setPosition({LEFT_X + 520.f,  summaryY + 28.f}); window.draw(ammoTxt);

    // ── Leave button ──────────────────────────
    window.draw(leaveBtn);
    auto leaveLbl = makeText("Leave Store", 22);
    leaveLbl.setPosition({563.f, 630.f});
    window.draw(leaveLbl);
}

// ─────────────────────────────────────────────
//  Helper — make a text object quickly
// ─────────────────────────────────────────────
sf::Text Store::makeText(const std::string& str, unsigned size, sf::Color colour) {
    sf::Text t(font, str, size);
    t.setFillColor(colour);
    return t;
}
#include "Store.hpp"

//name, priceEach, maxPerBuy, quantity, type
Store::Store(sf::Font& font) : font(font)
{
    items = {
        { "Food",    25, 25, 0, StoreItem::Type::FOOD    },
        { "Water",   20, 25, 0, StoreItem::Type::WATER   },
        { "Medicine",50, 10,  0, StoreItem::Type::MEDICINE},
        { "Ammo",    30, 30, 0, StoreItem::Type::AMMO    },
    };
    buildRows();

    leaveBtn.setSize({200.f, 50.f});
    leaveBtn.setPosition({540.f, 620.f});
    leaveBtn.setFillColor(sf::Color(160, 40, 40));
}

// buildRows

void Store::buildRows()
{
    rows.clear();
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        float y = START_Y + i * ROW_H;
        Row r;

        r.minusBtn.setSize({BTN_W, BTN_H});
        r.minusBtn.setPosition({LEFT_X + 420.f, y + 10.f});
        r.minusBtn.setFillColor(sf::Color(80, 80, 80));

        r.plusBtn.setSize({BTN_W, BTN_H});
        r.plusBtn.setPosition({LEFT_X + 480.f, y + 10.f});
        r.plusBtn.setFillColor(sf::Color(80, 80, 80));

        rows.push_back(r);
    }
}

// ─── handleEvent ──────────────────────────────────────────────────────────────

void Store::handleEvent(const sf::Event& event, sf::RenderWindow& window, Inventory& inventory)
{
    const auto* mouse = event.getIf<sf::Event::MouseButtonPressed>();
    if (!mouse || mouse->button != sf::Mouse::Button::Left)
        return;

    sf::Vector2f mpos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (leaveBtn.getGlobalBounds().contains(mpos)) {
        done = true;
        return;
    }

    for (int i = 0; i < static_cast<int>(rows.size()); ++i) {
        if (rows[i].minusBtn.getGlobalBounds().contains(mpos))
            removeOne(i, inventory);
        else if (rows[i].plusBtn.getGlobalBounds().contains(mpos))
            addOne(i, inventory);
    }
}

// ─── addOne / removeOne ───────────────────────────────────────────────────────

void Store::addOne(int i, Inventory& inventory)
{
    if (items[i].quantity >= items[i].maxPerBuy)       
        return;
    if (!inventory.canAfford(items[i].priceEach))      
        return; 

    inventory.spendMoney(items[i].priceEach);
    items[i].quantity++;

    switch (items[i].type) {
        case StoreItem::Type::FOOD:    inventory.addFood(1);     
            break;
        case StoreItem::Type::WATER:   inventory.addWater(1);    
            break;
        case StoreItem::Type::MEDICINE:inventory.addMedicine(1); 
            break;
        case StoreItem::Type::AMMO:    inventory.addAmmo(1);       
            break;
    }
}

void Store::removeOne(int i, Inventory& inventory)
{
    if (items[i].quantity <= 0) return;

    inventory.addMoney(items[i].priceEach);
    items[i].quantity--;

    switch (items[i].type) {
        case StoreItem::Type::FOOD:     inventory.removeFood(1);        
            break;
        case StoreItem::Type::WATER:    inventory.removeWater(1);      
            break;
        case StoreItem::Type::MEDICINE: inventory.removeMedicine(1); 
            break;
        case StoreItem::Type::AMMO:     inventory.removeAmmo(1);     
            break;
    }
}

// ─── draw ─────────────────────────────────────────────────────────────────────

void Store::draw(sf::RenderWindow& window, const Inventory& inventory)
{
    window.clear(sf::Color(15, 15, 25));

    // Title
    auto title = makeText("WELCOME TO THE STORE", 30, sf::Color(220, 180, 80));
    title.setPosition({430.f, 60.f});
    window.draw(title);

    // Money
    auto moneyTxt = makeText("Money: $" + std::to_string(inventory.getMoney()), 24);
    moneyTxt.setPosition({50.f, 50.f});
    window.draw(moneyTxt);

    // Column headers
    float headerY = START_Y - 30.f;
    auto hItem  = makeText("ITEM",     18, sf::Color(150, 150, 150));
    auto hPrice = makeText("$/UNIT",   18, sf::Color(150, 150, 150));
    auto hMax   = makeText("MAX",      18, sf::Color(150, 150, 150));
    auto hQty   = makeText("IN CART",  18, sf::Color(150, 150, 150));
    auto hCost  = makeText("SPENT",    18, sf::Color(150, 150, 150));
    hItem .setPosition({LEFT_X,           headerY});
    hPrice.setPosition({LEFT_X + 220.f,   headerY});
    hMax  .setPosition({LEFT_X + 320.f,   headerY});
    hQty  .setPosition({LEFT_X + 440.f,   headerY});
    hCost .setPosition({LEFT_X + 600.f,   headerY});
    window.draw(hItem);  window.draw(hPrice);
    window.draw(hMax);   window.draw(hQty);
    window.draw(hCost);

    // Header divider
    sf::RectangleShape divider({900.f, 2.f});
    divider.setPosition({LEFT_X, START_Y - 8.f});
    divider.setFillColor(sf::Color(60, 60, 80));
    window.draw(divider);

    // ── Rows ──────────────────────────────────
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        const StoreItem& item = items[i];
        float y = START_Y + i * ROW_H;

        // Alternating row background
        if (i % 2 == 0) {
            sf::RectangleShape rowBg({900.f, ROW_H - 4.f});
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

        // Max per visit
        auto maxTxt = makeText("x" + std::to_string(item.maxPerBuy), 22, sf::Color(180, 180, 180));
        maxTxt.setPosition({LEFT_X + 320.f, y + 24.f});
        window.draw(maxTxt);

        // "–" button
        bool canRemove = item.quantity > 0;
        rows[i].minusBtn.setFillColor(canRemove ? sf::Color(140, 60, 60) : sf::Color(60, 60, 60));
        window.draw(rows[i].minusBtn);
        auto minusLbl = makeText("-", 26, canRemove ? sf::Color::White : sf::Color(100, 100, 100));
        minusLbl.setPosition({LEFT_X + 427.f, y + 8.f});
        window.draw(minusLbl);

        // Quantity in cart
        auto qtyTxt = makeText(std::to_string(item.quantity), 24);
        qtyTxt.setPosition({LEFT_X + 458.f, y + 22.f});
        window.draw(qtyTxt);

        // "+" button
        bool canAdd = item.quantity < item.maxPerBuy && inventory.canAfford(item.priceEach);
        rows[i].plusBtn.setFillColor(canAdd ? sf::Color(40, 140, 40) : sf::Color(60, 60, 60));
        window.draw(rows[i].plusBtn);
        auto plusLbl = makeText("+", 26, canAdd ? sf::Color::White : sf::Color(100, 100, 100));
        plusLbl.setPosition({LEFT_X + 487.f, y + 8.f});
        window.draw(plusLbl);

        // Total spent on this item
        int spent = item.priceEach * item.quantity;
        auto costTxt = makeText("$" + std::to_string(spent), 22,
                                spent > 0 ? sf::Color(220, 180, 80) : sf::Color(120, 120, 120));
        costTxt.setPosition({LEFT_X + 600.f, y + 24.f});
        window.draw(costTxt);

        // Row divider
        sf::RectangleShape rowDiv({900.f, 1.f});
        rowDiv.setPosition({LEFT_X, y + ROW_H - 2.f});
        rowDiv.setFillColor(sf::Color(40, 40, 60));
        window.draw(rowDiv);
    }

    // ── Inventory summary ─────────────────────
    float summaryY = START_Y + items.size() * ROW_H + 20.f;

    auto summaryTitle = makeText("YOUR INVENTORY", 30, sf::Color(150, 150, 150));
    summaryTitle.setPosition({LEFT_X, summaryY});
    window.draw(summaryTitle);

    auto foodTxt  = makeText("Food: "     + std::to_string(inventory.getFood()),     26, sf::Color(200, 200, 200));
    auto waterTxt = makeText("Water: "    + std::to_string(inventory.getWater()),    26, sf::Color(200, 200, 200));
    auto medTxt   = makeText("Medicine: " + std::to_string(inventory.getMedicine()), 26, sf::Color(200, 200, 200));
    auto ammoTxt  = makeText("Ammo: "     + std::to_string(inventory.getAmmo()),     26, sf::Color(200, 200, 200));

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

// ─── makeText ─────────────────────────────────────────────────────────────────

sf::Text Store::makeText(const std::string& str, unsigned size, sf::Color colour)
{
    sf::Text t(font, str, size);
    t.setFillColor(colour);
    return t;
}
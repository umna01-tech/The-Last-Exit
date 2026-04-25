#include "Inventory.hpp"
#include <algorithm>
#include <iostream>

Inventory::Inventory()
    : food(0), water(0), medicine(0), ammo(0), money(0)
{}

// ─── Helper ───────────────────────────────────────────────────────────────────

int Inventory::clamp(int value, int lo, int hi)
{
    return std::max(lo, std::min(value, hi));
}

// ─── Queries ──────────────────────────────────────────────────────────────────

bool Inventory::hasFood()            const { return food     > 0; }
bool Inventory::hasWater()           const { return water    > 0; }
bool Inventory::hasMed()             const { return medicine > 0; }
bool Inventory::hasAmmo()            const { return ammo     > 0; }
bool Inventory::canAfford(int price) const { return money   >= price; }

// ─── Food ─────────────────────────────────────────────────────────────────────

void Inventory::addFood(int amount)
{
    food = clamp(food + amount, 0, MAX_FOOD);
}

void Inventory::removeFood(int amount)
{
    food = clamp(food - amount, 0, MAX_FOOD);
}

void Inventory::useFood(Entity& entity)
{
    if (!hasFood()) { std::cout << "No food left.\n"; return; }
    food--;
    entity.addFood(5);
}

// ─── Water ────────────────────────────────────────────────────────────────────

void Inventory::addWater(int amount)
{
    water = clamp(water + amount, 0, MAX_WATER);
}

void Inventory::removeWater(int amount)
{
    water = clamp(water - amount, 0, MAX_WATER);
}

void Inventory::useWater(Entity& entity)
{
    if (!hasWater()) { std::cout << "No water left.\n"; return; }
    water--;
    entity.addWater(5);
}

// ─── Medicine ─────────────────────────────────────────────────────────────────

void Inventory::addMedicine(int amount)
{
    medicine = clamp(medicine + amount, 0, MAX_MEDICINE);
}

void Inventory::removeMedicine(int amount)
{
    medicine = clamp(medicine - amount, 0, MAX_MEDICINE);
}

void Inventory::useMedicine(Entity& entity)
{
    if (!hasMed()) { std::cout << "No medicine left.\n"; return; }
    medicine--;
    entity.heal(15);
}

// ─── Ammo ─────────────────────────────────────────────────────────────────────

void Inventory::addAmmo(int amount)
{
    ammo = clamp(ammo + amount, 0, MAX_AMMO);
}

void Inventory::removeAmmo(int amount)
{
    ammo = clamp(ammo - amount, 0, MAX_AMMO);
}

void Inventory::useAmmo()
{
    if (!hasAmmo()) { std::cout << "No ammo left.\n"; return; }
    ammo--;
}

// ─── Money ────────────────────────────────────────────────────────────────────

void Inventory::addMoney(int amount)
{
    money += amount;
}

void Inventory::spendMoney(int amount)
{
    if (!canAfford(amount)) { std::cout << "Not enough money.\n"; return; }
    money -= amount;
}

void Inventory::setMoney(int amount)
{
    money = amount;
}
#include "Inventory.h"
#include <iostream>

Inventory::Inventory() : food(0), water(0), medicine(0), ammo(0), money(0) {}

bool Inventory::hasFood() const { return food > 0; }
bool Inventory::hasWater() const { return water > 0; }
bool Inventory::hasMed() const { return medicine > 0; }
bool Inventory::hasAmmo() const { return ammo > 0; }
bool Inventory::canAfford(int price) const { return money >= price; }

void Inventory::addFood(int amount)
{
    food += amount;
    if (food > MAX_FOOD)
        food = MAX_FOOD;
}

void Inventory::useFood(Entity& entity)
{
    if (!hasFood())
    {
        std::cout << "No food left.\n";
        return;
    }
    food -= 1;
    entity.addFood(5);
}

void Inventory::removeFood(int amount)
{
    food -= amount;
    if (food < 0)
        food = 0;
}

void Inventory::addWater(int amount)
{
    water += amount;
    if (water > MAX_WATER)
        water = MAX_WATER;
}

void Inventory::useWater(Entity& entity)
{
    if (!hasWater())
    {
        std::cout << "No water left.\n";
        return;
    }
    water -= 1;
    entity.addWater(5);
}

void Inventory::removeWater(int amount)
{
    water -= amount;
    if (water < 0)
        water = 0;
}

void Inventory::addMedicine(int amount) { medicine += amount; }

void Inventory::useMedicine(Entity &entity)
{
    if (!hasMed())
    {
        std::cout << "No medicine left.\n";
        return;
    }
    medicine -= 1;
    entity.heal(15);
}

void Inventory::addAmmo(int amount) { ammo += amount; }

void Inventory::useAmmo()
{
    if (!hasAmmo())
    {
        std::cout << "No ammo left.\n";
        return;
    }
    ammo -= 1;
}

void Inventory::addMoney(int amount)
{
    money += amount;
}

void Inventory::spendMoney(int amount)
{
    if (!canAfford(amount))
    {
        std::cout << "Not enough money.\n";
        return;
    }
    money -= amount;
}
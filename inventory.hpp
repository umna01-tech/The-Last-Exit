#pragma once

#include "Entity.hpp"

class Inventory
{
private:
    static const int MAX_FOOD = 50;
    static const int MAX_WATER = 50;

    int food;
    int water;
    int medicine;
    int ammo;
    int money;

public:
    Inventory();

    bool hasFood() const;
    bool hasWater() const;
    bool hasMed() const;
    bool hasAmmo() const;
    bool canAfford(int price) const;

    int getFood() const { return food; }
    int getWater() const { return water; }
    int getMedicine() const { return medicine; }
    int getAmmo() const { return ammo; }
    int getMoney() const { return money; }

    void addFood(int amount);
    void useFood(Entity& entity);
    void removeFood(int amount);

    void addWater(int amount);
    void useWater(Entity& entity);
    void removeWater(int amount);

    void addMedicine(int amount);
    void useMedicine(Entity& entity);

    void addAmmo(int amount);
    void useAmmo();

    void addMoney(int amount);
    void spendMoney(int amount);
    void setMoney(int amount);
};
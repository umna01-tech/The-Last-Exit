#pragma once

#include "Entity.hpp"

class Inventory
{
public:
    Inventory();

    // ── Queries ───────────────────────────────
    bool hasFood()            const;
    bool hasWater()           const;
    bool hasMed()             const;
    bool hasAmmo()            const;
    bool canAfford(int price) const;

    int getFood()     const { return food;     }
    int getWater()    const { return water;    }
    int getMedicine() const { return medicine; }
    int getAmmo()     const { return ammo;     }
    int getMoney()    const { return money;    }

    // ── Food ──────────────────────────────────
    void addFood(int amount);
    void removeFood(int amount);
    void useFood(Entity& entity);

    // ── Water ─────────────────────────────────
    void addWater(int amount);
    void removeWater(int amount);
    void useWater(Entity& entity);

    // ── Medicine ──────────────────────────────
    void addMedicine(int amount);
    void removeMedicine(int amount);
    void useMedicine(Entity& entity);

    // ── Ammo ──────────────────────────────────
    void addAmmo(int amount);
    void removeAmmo(int amount);
    void useAmmo();

    // ── Money ─────────────────────────────────
    void addMoney(int amount);
    void spendMoney(int amount);
    void setMoney(int amount);

private:
    static constexpr int MAX_FOOD     = 50;
    static constexpr int MAX_WATER    = 50;
    static constexpr int MAX_MEDICINE = 30;
    static constexpr int MAX_AMMO     = 200;

    int food;
    int water;
    int medicine;
    int ammo;
    int money;

    static int clamp(int value, int lo, int hi);
};
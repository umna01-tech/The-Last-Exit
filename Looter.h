#pragma once
#include "Enemy.h"

class Looter : public Enemy {
public:
    Looter(int health, int damage);
    void takeDamage(int amount) override;
    void attack(Player& player) override;
    void onDeath(Inventory& inventory) override;
    void stealResources(Inventory& inventory);
};
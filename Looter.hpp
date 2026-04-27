#pragma once
#include "Enemy.hpp"

class Looter : public Enemy {
public:
    Looter(int health, int damage);
    void stealResources(Inventory& inventory);
};
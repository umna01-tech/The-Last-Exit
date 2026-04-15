// Enemy.h
#pragma once
#include <string>

class Inventory;
class Player;

class Enemy {
protected:
    int health;
    int damage;

public:
    Enemy(int health_, int damage_);
    virtual ~Enemy();
    virtual void takeDamage(int amount);
    std::string getStatus() const;
    virtual void attack(Player& player);
    virtual void onDeath(Inventory& inventory);
    bool isDead() const { return health <= 0; }
};
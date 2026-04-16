#pragma once
#include "Entity.h"
 
class Player : public Entity {
 
private:
    int morale;
    int attack;
 
public:
    static const int MAX_MORALE   = 100;
    static const int START_MORALE = 50;
    static const int MIN_ATTACK   = 5;

    Player(const std::string& name_);
    virtual ~Player();
 
    // Overrides
    void update() override;
 
    // Moral system
    void addMorale(int amount);
    void reduceMorale(int amount);
    int  getMorale() const { return morale; }
 
    // Combat
    int getAttack() const { return attack; }
};
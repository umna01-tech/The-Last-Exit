#pragma once
#include "Entity.h"
#include "Inventory.h"
 
class Player : public Entity {
 
private:
    int humanity;
    int damage;
    Inventory inventory;
 
public:
    static const int MAX_HUMANITY   = 100;
    static const int START_HUMANITY = 50;
    static const int MAX_DAMAGE     = 5;

    Player(const std::string& name_);
    virtual ~Player();
 
    // Overrides
    void takeDamage(int amount) override; //WHEN WE WILL IMPLEMENT COMBAT SDL2
    void update() override;
 
    // Moral system
    void addHumanity(int amount);
    void reduceHumanity(int amount);
    int  getHumanity() const { return humanity; }
 
    // Combat
    int getDamage() const { return damage; }

    Inventory& getInventory() { return inventory; }

};
#include "Child.h"
#include "Player.h"
#include <iostream>

// Constructor
Child::Child(std::string name, int health, int food, int water, Player* player)
    : FamilyMember(name, health, food, water, "Child", player), moraleTimer_(5)
{
}

// Update
void Child::update() {
    FamilyMember::update();

    moraleTimer_--;

    if (moraleTimer_ <= 0) {
        if (player_ != nullptr) {
            player_->addFood(10);  // child helps by boosting food
            std::cout << "Child boosted morale! +10 food\n";
        }
        moraleTimer_ = 5;
    }
}
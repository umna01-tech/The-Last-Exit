#include "Child.h"
#include "Player.h"
#include <iostream>

// Constructor
Child::Child(const std::string& name_, Player& player_)
    : FamilyMember(name_,"Child", player_), moraleTimer(0){}

void Child::update() {
    FamilyMember::update();
    moraleTimer++;
    if (moraleTimer >= 5) {
        player.addFood(10);
        moraleTimer = 0;
    }
}
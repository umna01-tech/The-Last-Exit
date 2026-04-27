#include "Child.hpp"
#include "Player.hpp"
#include <iostream>

Child::Child(const std::string& name_, Player& player_)
    : FamilyMember(name_, "Child", player_), moraleTimer(0) {}

void Child::update() {
    FamilyMember::update();
    moraleTimer++;
    if (moraleTimer >= 5) {
        player.addMorale(5);
        moraleTimer = 0;
    }
}

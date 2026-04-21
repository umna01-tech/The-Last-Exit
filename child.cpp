#include "Child.hpp"
#include "Player.hpp"
#include <iostream>

Child::Child(const std::string& name_, Player& player_)
    : FamilyMember(name_, "Child", player_), moraleTimer(0) {}

void Child::update() {
    FamilyMember::update();
    moraleTimer++;
    if (moraleTimer >= 5) {
        player.addFood(10);
        moraleTimer = 0;
    }
}

std::string Child::getStatus() const {
    if (health > 70) return "Playful";
    if (health > 30) return "Scared";
    return "Needs care";
}
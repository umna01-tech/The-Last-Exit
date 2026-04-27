#include "FamilyMember.hpp"
#include "Player.hpp"

FamilyMember::FamilyMember(const std::string& name_, const std::string& role_, Player& player_)
    : Entity(name_), role(role_), player(player_) {}

FamilyMember::~FamilyMember() {}

void FamilyMember::takeDamage(int amount) {
    if (!isAlive()) return;
    health -= amount;
    if (health < 0) 
        health = 0;
}

void FamilyMember::update() {
    reduceFood(10);
    reduceWater(15);
}

std::string FamilyMember::getStatus() const {
    if (health > 70) return "Fine";
    if (health > 30) return "Hurt";
    return "Critical";
}
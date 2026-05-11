#include "Partner.hpp"
#include "Player.hpp"
#include "Entity.hpp"
#include "FamilyMember.hpp"
#include "Child.hpp"
#include <iostream>
 
Partner::Partner(const std::string& name_, const std::string& role_, Player& player_, Child& child_)
    : FamilyMember(name_, role_, player_), child(child_), healCooldown(0) {}
 
Partner::~Partner() {}
 
void Partner::healPlayer() {
    player.heal(15);
    child.heal(10);

    std::cout << "Partner healed the player and child!\n";
}
 
void Partner::update() {
    FamilyMember::update();
    healCooldown++;
    if (healCooldown >= 2) {
        healPlayer();
        healCooldown = 0;
    }
}
 
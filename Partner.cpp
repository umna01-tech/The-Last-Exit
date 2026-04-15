#pragma once
#include "FamilyMember.h"
#include "Player.h"
#include "Partner.h"
#include <iostream>

Partner::Partner(const std::string& name_, const std::string& role_, Player& player_)
    : FamilyMember(name_, "Partner", player_), healCooldown(0) {}
Partner::~Partner() {}

void Partner::healPlayer()
{
    player.heal(15);
    std::cout << "Partner healed the player!\n";
}

void Partner::update()
{
    FamilyMember::update();
    healCooldown++;
    if (healCooldown >= 3)
    {
        healPlayer();
        healCooldown = 0;
    }
}

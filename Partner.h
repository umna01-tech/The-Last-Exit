#pragma once
#include "FamilyMember.h"
#include <iostream>

class Partner : public FamilyMember {

private:
    int healCooldown;
public:
// Constructor
Partner(const std::string& name_, const std::string& role_, Player& player_);
~Partner() {}

void healPlayer();

void update();
};
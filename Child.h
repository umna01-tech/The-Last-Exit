#pragma once
#include "FamilyMember.h"

class Child : public FamilyMember {
private:
    int moraleTimer;

public:
    Child(const std::string& name_, Player& player_);
    void update() override;
};
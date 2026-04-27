#pragma once
#include "FamilyMember.hpp"
 
class Child : public FamilyMember {
private:
    int moraleTimer;
 
public:
    Child(const std::string& name_, Player& player_);
    void update() override;
};
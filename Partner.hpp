#pragma once
#include "FamilyMember.hpp"
 
class Partner : public FamilyMember {
private:
    int healCooldown;
 
public:
    Partner(const std::string& name_, const std::string& role_, Player& player_);
    ~Partner();
 
    void healPlayer();
    void update() override;
    std::string getStatus() const override;
};
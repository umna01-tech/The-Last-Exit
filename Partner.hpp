#pragma once
#include "FamilyMember.hpp"
class Child;
class Partner : public FamilyMember {
private:
    int healCooldown;
    Child& child;
 
public:
    Partner(const std::string& name_, const std::string& role_, Player& player_, Child& child_);
    ~Partner();
 
    void healPlayer();
    void update() override;
};
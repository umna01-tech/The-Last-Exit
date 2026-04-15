#pragma once
#include <string>
#include "Entity.h"

class Player;

class FamilyMember : public Entity
{
protected:
    std::string role;
    Player& player;

public:
    FamilyMember(const std::string& name_, const std::string& role_, Player& player_);
    virtual ~FamilyMember();
    void takeDamage(int amount) override;
    void update() override;
    std::string getRole() const { return role; }
};
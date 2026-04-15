#pragma once
#include "Player.h"
#include "FamilyMember.h"
#include "Enemy.h"
#include <vector>
#include <memory>
#include <string>

struct GameState
{
    Player player;
    int step;
    bool gameOver;
    std::vector<std::unique_ptr<FamilyMember>> family;
    std::vector<std::unique_ptr<Enemy>> enemies;
    GameState(const std::string &playerName) : player(playerName), step(0), gameOver(false){}
};
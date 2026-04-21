#pragma once
#include "Enemy.hpp"
#include <string>

class Mutant : public Enemy
{
public:
    Mutant(int health, int damage);
};
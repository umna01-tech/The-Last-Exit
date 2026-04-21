// Enemy.cpp
#include "Enemy.hpp"
#include "Player.hpp"
#include "Inventory.hpp"
#include <cstdlib>

Enemy::Enemy(int health_, int damage_) : health(health_), damage(damage_) {}

Enemy::~Enemy() {}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

std::string Enemy::getStatus() const {
    if (health == 0)  return "Defeated";
    if (health < 50)  return "Wounded";
    return "Hostile";
}

void Enemy::attack(Player& player) {
    player.takeDamage(damage);
}

void Enemy::onDeath(Inventory& inventory) {
    int drop = rand() % 4;
    switch (drop) {
        case 0: inventory.addFood(5);     
        break;
        case 1: inventory.addWater(5);    
        break; 
        case 2: inventory.addMedicine(1); 
        break;
        case 3: inventory.addAmmo(1);    
        break;
    }
}
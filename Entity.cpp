#include "Entity.h"

Entity::Entity(const std::string& name_)
    : name(name_), health(MAX_HEALTH), food(MAX_FOOD), water(MAX_WATER){}

Entity::~Entity() {}

void Entity::takeDamage(int amount) {
    if (!isAlive()) return;
    health -= amount;
    if (health < 0)
        health = 0;
}

void Entity::heal(int amount) {
    if (!isAlive()) return;
    health += amount;
    if (health > MAX_HEALTH)
        health = MAX_HEALTH;
}

void Entity::addFood(int amount) {
    if (!isAlive()) return;
    food += amount;
    if (food > MAX_FOOD)
        food = MAX_FOOD;
}

void Entity::addWater(int amount) {
    if (!isAlive()) return;
    water += amount;
    if (water > MAX_WATER)
        water = MAX_WATER;
}

void Entity::reduceFood(int amount) {
    if (!isAlive()) return;
    food -= amount;
    if (food < 0)
        food = 0;
    if (food == 0)
        takeDamage(5);
}

void Entity::reduceWater(int amount) {
    if (!isAlive()) return;
    water -= amount;
    if (water < 0)
        water = 0;
    if (water == 0)
        takeDamage(10);
}
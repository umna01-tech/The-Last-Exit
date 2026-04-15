#include "Player.h"
#include "Inventory.h"

Player::Player(const std::string& name_)
    : Entity(name_), humanity(START_HUMANITY), damage(MAX_DAMAGE) {} 

Player::~Player() {}

void Player::takeDamage(int amount) {
    Entity::takeDamage(amount);
    // Additional SDL2 logic can be added here
}

void Player::update() {
    reduceFood(2);
    reduceWater(3);
}

void Player::addHumanity(int amount) {
    humanity += amount;
    if (humanity > MAX_HUMANITY) {
        humanity = MAX_HUMANITY;
    }
}

void Player::reduceHumanity(int amount) {
    humanity -= amount;
    if (humanity < 0) {
        humanity = 0;
    }
}

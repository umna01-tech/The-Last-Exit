#include "Player.h"

Player::Player(const std::string& name_)
    : Entity(name_), morale(START_MORALE), attack(MIN_ATTACK) {} 

Player::~Player() {}

void Player::update() {
    reduceFood(2);
    reduceWater(3);
}

void Player::addMorale(int amount) {
    morale += amount;
    if (morale > MAX_MORALE) {
        morale = MAX_MORALE;
    }
}

void Player::reduceMorale(int amount) {
    morale -= amount;
    if (morale < 0) {
        morale = 0;
    }
}

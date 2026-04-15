#include "Stranger.h"
#include "Player.h"
#include <iostream>
#include <cstdlib>

// Constructor
Stranger::Stranger(std::string name, int health, int food, int water, Player* player)
    : FamilyMember(name, health, food, water, "Stranger", player), giftTimer_(4)
{
}

// Give random gift
void Stranger::giveGift() {
    if (player_ != nullptr) {
        int r = rand() % 3;

        if (r == 0) {
            player_->addFood(5);
            std::cout << "Stranger gave you food!\n";
        }
        else if (r == 1) {
            player_->heal(10);
            std::cout << "Stranger healed you!\n";
        }
        else {
            player_->addHumanity(5);
            std::cout << "Stranger inspired you! Humanity +5\n";
        }
    }
}

// Update
void Stranger::update() {
    FamilyMember::update();

    giftTimer_--;

    if (giftTimer_ <= 0) {
        giveGift();
        giftTimer_ = 4;
    }
}
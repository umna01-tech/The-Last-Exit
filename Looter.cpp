#include "Looter.hpp"
#include "Inventory.hpp"
#include <iostream>

Looter::Looter(int health, int damage) : Enemy(health, damage){}
// Steal resources
void Looter::stealResources(Inventory& Inventory) {
    int choice = rand() % 2;
    if ((choice == 0) && (Inventory.getFood() > 0) ){
        Inventory.removeFood(5);
        std::cout << "Looter stole 5 food from you!\n";
    } else if (choice == 1 && Inventory.getWater() > 0) {
        Inventory.removeWater(5);
        std::cout << "Looter stole 5 water from you!\n";
    } else {
        std::cout << "Looter tried to steal resources, but you have no water or food left!\n";
    }
}
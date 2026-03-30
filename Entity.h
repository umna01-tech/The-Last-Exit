#pragma once
#include <string>

class Entity {
protected:
    std::string name;
    int health;
    int food;
    int water;

public:
    static const int MAX_HEALTH = 100;
    static const int MAX_FOOD   = 100;
    static const int MAX_WATER  = 100;


    Entity(const std::string& name_);
    virtual ~Entity();

    // Getters
    std::string getName() const { return name;   }
    int getHealth()       const { return health; }
    int getFood()         const { return food;   }
    int getWater()        const { return water;  }

    // Status check
    bool isAlive() const { return health > 0; }

    // Actions
    virtual void takeDamage(int amount);
    void heal       (int amount);
    void addFood    (int amount);
    void addWater   (int amount);
    void reduceFood (int amount);
    void reduceWater(int amount);

    virtual void update() {}
    virtual std::string getStatus() const = 0;
};
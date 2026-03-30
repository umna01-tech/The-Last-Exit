# Entity
Base class for any entity in the game. Handles health, food, and water. Can't be used directly since `getStatus()` is pure virtual, so you need to subclass it.
## Files
- `Entity.h` — class declaration
- `Entity.cpp` — implementations
---
## Stats
All three start at 100 when an entity is created.
- `MAX_HEALTH` = 100
- `MAX_FOOD` = 100
- `MAX_WATER` = 100
---
## Constructor
cpp
Entity(const std::string& name_);
---
## Getters
getName()   // returns name
getHealth() // returns current health
getFood()   // returns current food
getWater()  // returns current water
isAlive()   // true if health > 0
## Methods
All methods do nothing if the entity is already dead.
cpp
takeDamage(int amount)  // reduces health, stops at 0
heal(int amount)        // increases health, stops at MAX_HEALTH

addFood(int amount)     // increases food, stops at MAX_FOOD
reduceFood(int amount)  // reduces food, deals 5 damage if it hits 0

addWater(int amount)    // increases water, stops at MAX_WATER
reduceWater(int amount) // reduces water, deals 10 damage if it hits 0
---
## Subclassing
getStatus() must be implemented. update() and takeDamage() can be overridden if needed.

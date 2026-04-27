#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include <memory>
#include "GameEvent.hpp"
#include "Enemy.hpp"
#include "Partner.hpp"
#include "Child.hpp"
#include "Inventory.hpp"
#include "Player.hpp"

class GameWorld {
public:
    GameWorld(Player& player, Partner& partner, Child& child, Inventory& inventory);

    enum class EndingType { 
        NONE, 
        DEAD, 
        MORALE, 
        GOOD 
    };

    //Room update
    void enterNextRoom();
    //Choice handling
    void fireChoice(int i);
    //is event active
    bool isEventActive() const { return eventActive; }
    bool isChoiceMade()  const { return choiceMade; }

    void draw(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f pos, sf::Vector2f size) const;
    int  handleClick(sf::Vector2f mp) const;

    bool   inCombat()     const { return combatActive;      }
    Enemy* currentEnemy()       { return activeEnemy.get(); }
    void   endCombat();

    int  getCurrentRoom() const { return currentRoom;  }
    int  getTotalRooms()  const { return TOTAL_ROOMS;  }

    EndingType getEnding()  const { return ending; }
    //is game over
    bool isFinished() const { return ending != EndingType::NONE; }

    //Player opts to rest, consuming resources to heal and boost morale
    void doRest();

private:
    Player& player;
    Partner& partner;
    Child& child;
    Inventory& inventory;

    EndingType ending = EndingType::NONE;

    static const int TOTAL_ROOMS = 20;
    int  currentRoom;
    bool eventActive;
    bool combatActive;
    bool choiceMade; // player must make a real choice before moving on

    RoomEvent activeEvent;
    std::unique_ptr<Enemy> activeEnemy;

    //bunch of possible events, gets picked randomly when you enter a room. 
    std::vector<RoomEvent> easyPool;
    std::vector<RoomEvent> medPool;
    std::vector<RoomEvent> hardPool;

    //tell event
    void buildEasyPool();
    void buildMedPool();
    void buildHardPool();

    //take room number, return tier
    RoomTier  tierForRoom(int room) const;
    //take tier, return event
    RoomEvent pickEvent(RoomTier tier);

    //update
    void applyRoomDrain();
    //random event
    void maybeFireRandomEvent(); 
    //fight option
    void triggerCombat(int hp, int dmg, bool isLooter);
    //is this the end? After every event
    void checkEndings();

    //actual image data in memory
    mutable sf::Texture locTexture;
    //remembers the filename
    mutable std::string  loadedImage;
    //sprite to draw, if image loaded successfully
    mutable std::optional<sf::Sprite> locSprite;
    //load image if not already loaded, set sprite
    void loadImage(const std::string& file) const;

    void eatFromStorage(bool careful);
    void maybePoisonSomeone();
    void maybeWasTrapped();
    void giveToStranger(int food, int water, int moraleReward, int ammoReward = 0);
    void tryRest();

    mutable std::vector<sf::FloatRect> choiceRects;

    void drawEventScreen(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f pos, sf::Vector2f size) const;
};

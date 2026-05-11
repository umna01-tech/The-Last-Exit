#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <string>
#include "Endings.hpp"
#include "Enemy.hpp"

class Player;
class Partner;
class Child;
class Inventory;
class Enemy;

enum class RoomTier { EASY, MEDIUM, HARD };
enum class EventType { RESOURCE, HAZARD, ENCOUNTER, COMBAT, REST, MORAL };

struct Choice {
    std::string text;
    std::function<void()> action;
};

struct RoomEvent {
    EventType   type;
    std::string locationName;
    std::string imageFile;
    std::string narrative;
    std::vector<Choice> choices;
};

class GameWorld {
public:
    static constexpr int TOTAL_ROOMS = 20;

    GameWorld(Player& player, Partner& partner, Child& child, Inventory& inventory);

    void enterNextRoom();
    void fireChoice(int i);
    void doRest();
    void endCombat();

    bool isFinished()      const { return ending != EndingType::NONE; }
    EndingType getEnding() const { return ending; }

    int  getCurrentRoom() const { return currentRoom; }
    bool isEventActive()  const { return eventActive; }
    bool isCombatActive() const { return combatActive; }
    bool isChoiceMade()   const { return choiceMade; }

    const RoomEvent& getActiveEvent() const { return activeEvent; }
    Enemy*           getActiveEnemy() const { return activeEnemy.get(); }

    void draw(sf::RenderWindow& window, const sf::Font& font,
              sf::Vector2f pos, sf::Vector2f size) const;
    int  handleClick(sf::Vector2f mp) const;

private:
    Player&    player;
    Partner&   partner;
    Child&     child;
    Inventory& inventory;

    int  currentRoom;
    bool eventActive;
    bool combatActive;
    bool choiceMade;

    RoomEvent              activeEvent;
    std::unique_ptr<Enemy> activeEnemy;

    EndingType ending = EndingType::NONE;

    std::vector<RoomEvent> easyPool;
    std::vector<RoomEvent> medPool;
    std::vector<RoomEvent> hardPool;

    void buildEasyPool();
    void buildMedPool();
    void buildHardPool();

    RoomTier  tierForRoom(int room) const;
    RoomEvent pickEvent(RoomTier tier);

    void applyRoomDrain();
    void maybeFireRandomEvent();
    void checkEndings();
    void triggerCombat(int hp, int dmg, bool isLooter);

    void maybePoisonSomeone();
    void eatFromStorage(bool careful);
    void maybeWasTrapped();
    void giveToStranger(int food, int water, int moraleReward, int ammoReward = 0);
    void tryRest();

    mutable sf::Texture                locTexture;
    mutable std::optional<sf::Sprite>  locSprite;
    mutable std::string                loadedImage;
    mutable std::vector<sf::FloatRect> choiceRects;

    void loadImage(const std::string& file) const;
    void drawEventScreen(sf::RenderWindow& window, const sf::Font& font,
                         sf::Vector2f pos, sf::Vector2f size) const;
};
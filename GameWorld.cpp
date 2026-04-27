#include "GameWorld.hpp"
#include "Player.hpp"
#include "Partner.hpp"
#include "Child.hpp"
#include "Inventory.hpp"
#include "Enemy.hpp"
#include "Looter.hpp"
#include "Mutant.hpp"
#include <algorithm>
#include <cstdlib>

GameWorld::GameWorld(Player& player, Partner& partner, Child& child, Inventory& inventory)
    : player(player), partner(partner), child(child), inventory(inventory)
    , currentRoom(-1), eventActive(false), combatActive(false)
    , choiceMade(false)
{
    buildEasyPool();
    buildMedPool();
    buildHardPool();
}

RoomTier GameWorld::tierForRoom(int room) const {
    if (room <= 6)  return RoomTier::EASY;
    if (room <= 13) return RoomTier::MEDIUM;
    return RoomTier::HARD;
}

RoomEvent GameWorld::pickEvent(RoomTier tier) {
    std::vector<RoomEvent>* pool = nullptr;
    switch (tier) {
        case RoomTier::EASY:   pool = &easyPool;  
            break;
        case RoomTier::MEDIUM: pool = &medPool;   
            break;
        case RoomTier::HARD:   pool = &hardPool;    
            break;
    }
    //go to the actual vector, pick a random slot, return whatever event is there
    return (*pool)[rand() % (int)pool->size()];
}

void GameWorld::applyRoomDrain() {
    player.update();
    partner.update();
    child.update();
}

// Random bad things that just happen — no choice, no warning
void GameWorld::maybeFireRandomEvent() {
    int roll = rand() % 100;

    // ~15% chance of illness hitting someone
    if (roll < 15) {
        int who = rand() % 3;
        if (who == 0) {
            player.takeDamage(30);
            player.reduceMorale(10);
            // prepend a notice to the narrative
            activeEvent.narrative =
                "[You wake up feverish. It must be something you ate, or just the air down here.\n"
                "You cursed everyone and everything as you dragged yourself out of bed."
                " You lose 30 health and 10 morale before you can even move.]\n\n"
                + activeEvent.narrative;
        } else if (who == 1) {
            partner.takeDamage(30);
            partner.reduceFood(20);
            activeEvent.narrative =
                "[Your Partner wakes up sick. There's nothing you can do right now.\n"
                "They threw up and shivered through the night.\n\n"
                " They push through it but lose 20 health.]\n\n"
                + activeEvent.narrative;
        } else {
            child.takeDamage(30);
            child.reduceFood(20);
            activeEvent.narrative =
                "[Your child is burning up. You hold them until it passes.\n"
                "They threw up and shivered through the night.\n\n"
                " It costs them 30 health and costs you 20 food.]\n\n"
                + activeEvent.narrative;
        }
        return;
    }

    // ~10% chance of a section of ceiling coming down
    else if (roll < 25) {
        int dmg = 10 + rand() % 11; // 10-20
        player.takeDamage(dmg);
        partner.takeDamage(dmg);
        child.takeDamage(dmg);
        player.reduceMorale(5);
        activeEvent.narrative =
            "[A section of concrete drops without warning. You barely dodge it.\n"
            " Still costs you and your family " + std::to_string(dmg) + " health. The tunnels are not safe.]\n\n"
            + activeEvent.narrative;
    }
    else if (roll < 35) {
    inventory.removeFood(5);
    activeEvent.narrative =
        "[Something got into the supplies overnight.\n"
        "5 food gone. You don't know when it happened.]\n\n"
        + activeEvent.narrative;
}
}

void GameWorld::enterNextRoom() {
    currentRoom++;
    eventActive    = true;
    combatActive   = false;
    choiceMade     = false;
    activeEnemy.reset();

    applyRoomDrain();
    checkEndings();
    if (isFinished()) return;

    activeEvent = pickEvent(tierForRoom(currentRoom));
    maybeFireRandomEvent(); 

    checkEndings();
    if (isFinished()) return;
}

void GameWorld::fireChoice(int i) {
    if (!eventActive) return;
    if (i < 0 || i >= (int)activeEvent.choices.size()) return;

    activeEvent.choices[i].action();
    choiceMade = true;

    if (!combatActive) {
        eventActive    = false;
        checkEndings();
    }
}

void GameWorld::doRest() {
    int needed = 9;
    if (inventory.getFood() < needed || inventory.getWater() < needed){
        activeEvent.narrative = "[Not enough supplies to rest properly. You push on.]\n\n" 
            + activeEvent.narrative;
        return;
    }
    inventory.removeFood(needed);
    inventory.removeWater(needed);
    player.heal(15); 
    partner.heal(15); 
    child.heal(15);
    player.addMorale(10);
    activeEvent.narrative = "[You rest. Food and water spent. Everyone recovers 15 health.]\n\n"
        + activeEvent.narrative;
}

void GameWorld::triggerCombat(int hp, int dmg, bool isLooter) {
    combatActive = true;
    if (isLooter)
        activeEnemy = std::make_unique<Looter>(hp, dmg);
    else
        activeEnemy = std::make_unique<Mutant>(hp, dmg);
}

void GameWorld::endCombat() {
    combatActive = false;
    if (activeEnemy) { 
        activeEnemy->onDeath(inventory); 
        activeEnemy.reset(); 
    }
    eventActive    = false;
    choiceMade     = true;
    checkEndings();
}


void GameWorld::checkEndings() {
    if (!player.isAlive()) { 
        ending = EndingType::DEAD; 
        return; 
    }
    if (currentRoom >= TOTAL_ROOMS - 1) {
        if      (player.getMorale() >= 60) ending = EndingType::GOOD;
        else if (player.getMorale() >= 25) ending = EndingType::MORALE;
        else                               ending = EndingType::DEAD;
    }
}

void GameWorld::loadImage(const std::string& file) const {
    if (file == loadedImage) return;
    loadedImage = file;
    locSprite.reset();
    if (locTexture.loadFromFile(file)) {
        locSprite.emplace(locTexture);
        sf::Vector2u sz = locTexture.getSize();
        float sc = std::min(576.f / sz.x, 196.f / sz.y);
        locSprite->setScale({sc, sc});
    }
}

void GameWorld::draw(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f pos, sf::Vector2f size) const {
        drawEventScreen(window, font, pos, size);
}

void GameWorld::drawEventScreen(sf::RenderWindow& window, const sf::Font& font,
                                 sf::Vector2f pos, sf::Vector2f size) const {
    float x = pos.x, y = pos.y, w = size.x;

    // Image area
    float imgH = 200.f;
    sf::RectangleShape imgBox({w, imgH});
    imgBox.setPosition({x, y});
    imgBox.setFillColor(sf::Color(15, 15, 24));
    imgBox.setOutlineColor(sf::Color(50, 50, 80));
    imgBox.setOutlineThickness(1.f);
    window.draw(imgBox);

    loadImage(activeEvent.imageFile);
    if (locSprite.has_value()) {
        locSprite->setPosition({x + 2.f, y + 2.f});
        window.draw(*locSprite);
    } else {
        sf::Text ph(font, "[ " + activeEvent.locationName + " ]", 15);
        ph.setFillColor(sf::Color(70, 70, 90));
        ph.setPosition({x + 12.f, y + 80.f});
        window.draw(ph);
    }

    // Location name bar overlay
    sf::RectangleShape nameBar({w, 26.f});
    nameBar.setPosition({x, y});
    nameBar.setFillColor(sf::Color(0, 0, 0, 170));
    window.draw(nameBar);
    sf::Text locTxt(font, activeEvent.locationName, 13);
    locTxt.setFillColor(sf::Color(200, 200, 160));
    locTxt.setPosition({x + 8.f, y + 5.f});
    window.draw(locTxt);

    // Narrative
    float textY = y + imgH + 4.f;
    float textH = 280.f;
    sf::RectangleShape textBox({w, textH});
    textBox.setPosition({x, textY});
    textBox.setFillColor(sf::Color(11, 11, 20));
    textBox.setOutlineColor(sf::Color(40, 40, 70));
    textBox.setOutlineThickness(1.f);
    window.draw(textBox);

    sf::Text narr(font, activeEvent.narrative, 20);
    narr.setFillColor(sf::Color(210, 205, 190));
    narr.setPosition({x + 12.f, textY + 8.f});
    window.draw(narr);

    // Choices
    float choiceY = textY + textH + 4.f;
    float choiceH = size.y - imgH - textH - 12.f;
    sf::RectangleShape choiceBox({w, choiceH});
    choiceBox.setPosition({x, choiceY});
    choiceBox.setFillColor(sf::Color(7, 7, 14));
    choiceBox.setOutlineColor(sf::Color(40, 40, 70));
    choiceBox.setOutlineThickness(1.f);
    window.draw(choiceBox);

    choiceRects.clear();
    float btnH = 38.f, gap = 5.f, btnY = choiceY + 6.f;
    bool locked = combatActive || choiceMade;

    for (int i = 0; i < (int)activeEvent.choices.size(); ++i) {
        sf::Color fill  = locked ? sf::Color(18,18,26) : sf::Color(26,36,55);
        sf::Color outln = locked ? sf::Color(35,35,50) : sf::Color(62,92,140);

        sf::RectangleShape btn({w - 20.f, btnH});
        btn.setPosition({x + 10.f, btnY});
        btn.setFillColor(fill);
        btn.setOutlineColor(outln);
        btn.setOutlineThickness(1.f);
        window.draw(btn);

        sf::Color txtCol = locked ? sf::Color(60,60,70) : sf::Color(175,205,255);
        sf::Text ct(font, activeEvent.choices[i].text, 13);
        ct.setFillColor(txtCol);
        ct.setPosition({x + 18.f, btnY + 11.f});
        window.draw(ct);

        choiceRects.push_back(btn.getGlobalBounds());
        btnY += btnH + gap;
    }

    // Combat banner
    if (combatActive && activeEnemy) {
        sf::RectangleShape banner({w, 34.f});
        banner.setPosition({x, y - 38.f});
        banner.setFillColor(sf::Color(48, 6, 6));
        banner.setOutlineColor(sf::Color(180, 35, 35));
        banner.setOutlineThickness(1.f);
        window.draw(banner);
        sf::Text ct(font, "!! COMBAT !!  " + activeEnemy->getStatus()
                        + "  — SHOOT or PUNCH to fight", 13);
        ct.setFillColor(sf::Color(255,100,100));
        ct.setPosition({x + 8.f, y - 32.f});
        window.draw(ct);
    }
}

int GameWorld::handleClick(sf::Vector2f mp) const {
    if (combatActive) return -1;
    // if choice made, only the wait-screen forward button is active
    // if (choiceMade) return -1;
    for (int i = 0; i < (int)choiceRects.size(); ++i)
        if (choiceRects[i].contains(mp)) return i;
    return -1;
}

void GameWorld::maybePoisonSomeone() {
    int who = rand() % 3;
    if (who == 0) {
        player.takeDamage(15);
        player.reduceMorale(10);
        activeEvent.narrative += "\n\n[Something was off. You feel it almost immediately.]";
    } else if (who == 1) {
        partner.takeDamage(15);
        player.reduceMorale(8);
        activeEvent.narrative += "\n\n[Your companion doubles over. Bad can.]";
    } else {
        child.takeDamage(10);
        player.reduceMorale(12);
        activeEvent.narrative += "\n\n[The child gets sick. You should have checked more carefully.]";
    }
}

void GameWorld::eatFromStorage(bool careful) {
    int food  = careful ? 1 + rand() % 3 : 2 + rand() % 5;
    int water = careful ? 1 + rand() % 3 : 2 + rand() % 5;
    inventory.addFood(food);
    inventory.addWater(water);

    bool toxic = !careful && rand() % 3 == 0; // 33% toxic only if careless
    if (toxic) {
        maybePoisonSomeone();
    } else {
        player.addMorale(careful ? 8 : 5);
        activeEvent.narrative += "\n\n[You got what you could. Good enough.]";
    }
}

void GameWorld::maybeWasTrapped() {
    if (rand() % 4 == 0) { // 25% chance it was actually dangerous
        player.addMorale(15);
        activeEvent.narrative += "\n\n[You hear movement behind the shelf moments later.\n"
                                   "Good call leaving it alone.]";
    } else {
        player.addMorale(3);
        activeEvent.narrative += "\n\n[Probably nothing. But you didn't risk it.]";
    }
}

void GameWorld::giveToStranger(int food, int water, int moraleReward, int ammoReward) {
    if (inventory.getFood() >= food && inventory.getWater() >= water) {
        inventory.removeFood(food);
        inventory.removeWater(water);
        player.addMorale(moraleReward);
        if (ammoReward > 0) inventory.addAmmo(ammoReward);
    } else {
        player.reduceMorale(5);
        activeEvent.narrative += "\n\n[You don't have enough to spare. They understand.]";
    }
}

void GameWorld::tryRest() {
    if (inventory.getFood() >= 9 && inventory.getWater() >= 9) {
        inventory.removeFood(9);
        inventory.removeWater(9);
        player.heal(15); partner.heal(15); child.heal(15);
        player.addMorale(15);
        activeEvent.narrative += "\n\n[Everyone sleeps. First real rest in a while.]";
    } else {
        player.reduceMorale(5);
        activeEvent.narrative += "\n\n[Not enough supplies to rest properly. You push on.]";
    }
}
void GameWorld::buildEasyPool() {
    easyPool.clear();

    // 1. Storage find
    easyPool.push_back({ EventType::RESOURCE, "Storage Closet", "assets/images/locations/storage.png",
        "You push open a rusted door. Shelves of old canned goods.\n"
        "Most are expired. Some are sealed. Hard to tell which is which.\n"
        "Your family eyes them without saying anything.",
        {
            { "Take everything you can carry", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    maybePoisonSomeone();
                } else if (roll < 55) {
                    int food = 1 + rand() % 3;
                    inventory.addFood(food);
                    inventory.addWater(1);
                    player.reduceMorale(3);
                    activeEvent.narrative += "\n\n[Most of it was bad. Salvaged "
                        + std::to_string(food) + " food and 1 water.]";
                } else {
                    int food = 4 + rand() % 4;
                    int water = 3 + rand() % 3;
                    inventory.addFood(food);
                    inventory.addWater(water);
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n[Better than expected."
                        + std::to_string(food) + " food, "
                        + std::to_string(water) + " water.]";
                }
            }},
            { "Check each one carefully", [this]{
                int roll = rand() % 100;
                if (roll < 15) {
                    // careful but still got poisoned somehow
                    maybePoisonSomeone();
                    activeEvent.narrative += "\n\n[You checked everything. You thought.]";
                } else if (roll < 50) {
                    // barely anything safe
                    inventory.addFood(1);
                    inventory.addWater(1);
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[Almost all of it was bad.\n"
                                              "1 food, 1 water. Barely worth stopping.]";
                } else {
                    // careful pays off
                    int food = 2 + rand() % 3;
                    int water = 2 + rand() % 3;
                    inventory.addFood(food);
                    inventory.addWater(water);
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[Took time but worth it. +"
                        + std::to_string(food) + " food, "
                        + std::to_string(water) + " water. All clean.]";
                }
            }},
            { "Leave it, could be watched", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    player.addMorale(18);
                    activeEvent.narrative += "\n\n[You hear someone come back for it.\n"
                                              "They were waiting. Good call.]";
                } else if (roll < 60) {
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[Nothing happened. You wonder\n"
                                              "what was in there.]";
                } else {
                    inventory.addFood(2);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[While leaving you spot a bag someone dropped.\n"
                                              "you grab it quickly. Inside: 2 food, sealed and safe.]";
                }
            }},
        }
    });

    // 2. Flooded passage
    easyPool.push_back({ EventType::HAZARD, "Service Tunnel", "assets/images/locations/tunnel.png",
        "A burst pipe has flooded the tunnel knee-deep.\n"
        "The water is cold but clean.\n",
        {
            { "Wade through carefully", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    int dmg = 8 + rand() % 10;
                    int who = rand() % 3;
                    if      (who == 0) player .takeDamage(dmg);
                    else if (who == 1) partner.takeDamage(dmg);
                    else               child  .takeDamage(dmg);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Someone slips hard. "
                        + std::to_string(dmg) + " damage.\nCareful isn't always enough.]";
                } else if (roll < 65) {
                    player.takeDamage(2);
                    partner.takeDamage(2);
                    child.takeDamage(2);
                    player.reduceMorale(3);
                    activeEvent.narrative += "\n\n[Slow. Cold. Everyone makes it through.]";
                } else {
                    int ammo = 1 + rand() % 3;
                    inventory.addAmmo(ammo);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[Something floats past. Ammo, sealed.\n"
                                              "+" + std::to_string(ammo) + " ammo.]";
                }
            }},
            { "Carry the others across", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    int dmg = 12 + rand() % 10;
                    player.takeDamage(dmg);
                    child.takeDamage(8);
                    player.reduceMorale(12);
                    activeEvent.narrative += "\n\n[You slip carrying them.\n"
                                              "You take " + std::to_string(dmg)
                                              + " damage. They take 8. Ugly.]";
                } else if (roll < 70) {
                    int dmg = 8 + rand() % 8;
                    player.takeDamage(dmg);
                    player.addMorale(6);
                    activeEvent.narrative += "\n\n[Costs you " + std::to_string(dmg)
                        + " health. Worth it.]";
                } else {
                    int dmg = 3 + rand() % 5;
                    player.takeDamage(dmg);
                    player.addMorale(15);
                    partner.heal(5);
                    activeEvent.narrative += "\n\n[Only costs " + std::to_string(dmg)
                        + " health. Your companions looks at you differently after.]\n";
                }
            }},
            { "Collect water then cross", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    int water = 2 + rand() % 3;
                    inventory.addWater(water);
                    player.takeDamage(12);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Looked clean. Wasn't.\n"
                                              + std::to_string(water)
                                              + " water but 12 damage. Bad trade.]";
                } else if (roll < 65) {
                    int water = 3 + rand() % 4;
                    inventory.addWater(water);
                    player.reduceMorale(4);
                    activeEvent.narrative += "\n\n[" + std::to_string(water)
                        + " water. Cold and miserable but practical.]";
                } else {
                    int water = 5 + rand() % 4;
                    inventory.addWater(water);
                    inventory.addFood(2);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[More than expected. +"
                        + std::to_string(water) + " water, +2 food on a shelf above the waterline.]";
                }
            }},
        }
    });

    easyPool.push_back({ EventType::ENCOUNTER, "Maintenance Bay", "assets/images/locations/shelter.png",
        "A small cat huddles in the corner, one leg clearly broken.\n"
        "It doesn't run. It's too weak to run.\n",
        {
            { "Use medicine to help it", [this]{
                int roll = rand() % 100;
                if (!inventory.hasMed()) {
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Nothing to help it with.\n"
                                              "You don't look back.]";
                } else if (roll < 20) {
                    inventory.removeMedicine(1);
                    player.reduceMorale(15);
                    child.takeDamage(5);
                    activeEvent.narrative += "\n\n[You try. It wasn't enough.\n"
                                              "Your companion buries it quietly.\n"
                                              "You wasted medicine.]";
                } else if (roll < 70) {
                    inventory.removeMedicine(1);
                    player.addMorale(20);
                    child.heal(5);
                    activeEvent.narrative += "\n\n[The cat calms down. Your companion holds it\n"
                                              "before setting it down gently.]";
                } else {
                    inventory.removeMedicine(1);
                    inventory.addFood(3);
                    player.addMorale(28);
                    child.heal(8);
                    activeEvent.narrative += "\n\n[The cat recovers. Leads you to a gap in the wall.\n"
                                              "Someone stashed food there. +3 food.]";
                }
            }},
            { "Leave it", [this]{
                int roll = rand() % 100;
                if (roll < 40) {
                    player.reduceMorale(20);
                    child.takeDamage(8);
                    activeEvent.narrative += "\n\n[Your family stops walking for a full minute.\n"
                                              "You don't speak for a long time after.]";
                } else if (roll < 80) {
                    player.reduceMorale(12);
                    child.takeDamage(5);
                    activeEvent.narrative += "\n\n[Your family says nothing.\n"
                                              "That's almost worse.]";
                } else {

                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[You move on. Later you realise it was\n"
                                              "probably too far gone anyway.\n"
                                              "Maybe that's just something you tell yourself.]";
                }
            }},
        }
    });

    easyPool.push_back({ EventType::RESOURCE, "Comms Room", "assets/images/locations/saferoom.png",
        "A speaker crackles to life. Someone is broadcasting.\n"
        "'Survivors at sector 7. Supplies at the east hub.'\n"
        "Could be real. Could be a trap to draw people out.",
        {
            { "Follow the signal", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    // trap
                    int dmg = 10 + rand() % 15;
                    player.takeDamage(dmg);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[Trap. You take " + std::to_string(dmg)
                        + " damage getting out. Barely.]";
                } else if (roll < 55) {
                    // picked over
                    int food = 1 + rand() % 2;
                    inventory.addFood(food);
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[Real but someone got here first.\n"
                                              "Scraps. +" + std::to_string(food) + " food.]";
                } else if (roll < 85) {
                    // good find
                    int food  = 3 + rand() % 4;
                    int water = 2 + rand() % 3;
                    inventory.addFood(food);
                    inventory.addWater(water);
                    player.addMorale(12);
                    activeEvent.narrative += "\n\n[It was real. +" + std::to_string(food)
                        + " food, +" + std::to_string(water) + " water.]";
                } else {
                    // jackpot — also find medicine
                    int food  = 4 + rand() % 4;
                    int water = 3 + rand() % 3;
                    inventory.addFood(food);
                    inventory.addWater(water);
                    inventory.addMedicine(1);
                    player.addMorale(20);
                    activeEvent.narrative += "\n\n[Real and untouched. +" + std::to_string(food)
                        + " food, +" + std::to_string(water)
                        + " water, +1 medicine.]";
                }
            }},
            { "Ignore it", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    // it was real and you missed it
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[You pass another group later carrying\n"
                                              "supplies from that direction.\n"
                                              "It was real.]";
                } else if (roll < 70) {
                    // nothing either way
                    player.addMorale(2);
                    activeEvent.narrative += "\n\n[You keep moving. Whatever it was,\n"
                                              "it's behind you now.]";
                } else {
                    // it was a trap and ignoring it saved you
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[You hear gunshots in that direction\n"
                                              "ten minutes later. You keep walking.]";
                }
            }},
        }
    });

    // 5. Rest spot
    easyPool.push_back({ EventType::REST, "Safe Alcove", "assets/images/locations/saferoom.png",
        "A dry alcove behind a heavy shelf. Dark. Hidden. Quiet.\n"
        "Everyone looks like they need this.\n"
        "Stopping costs supplies. Not stopping costs health.",
        {
            { "Rest here", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    // disturbed — waste of supplies
                    tryRest();
                    player.takeDamage(10);
                    player.reduceMorale(12);
                    activeEvent.narrative += "\n\n[Something woke you up mid-rest.\n"
                                              "Supplies spent. 10 damage. Barely slept.]";
                } else if (roll < 75) {
                    // normal rest
                    tryRest();
                } else {
                    // great rest — bonus
                    tryRest();
                    player.addMorale(10);
                    player.heal(5);
                    partner.heal(5);
                    child.heal(5);
                    activeEvent.narrative += "\n\n[Best sleep in days.\n"
                                              "Extra +10 morale, +5 health each.]";
                }
            }},
            { "Keep moving", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    // pushing through pays off — find something
                    int ammo = 1 + rand() % 4;
                    inventory.addAmmo(ammo);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[While the others wanted to stop you kept looking.\n"
                                              "Found " + std::to_string(ammo) + " ammo.]";
                } else if (roll < 70) {
                    // just tired
                    player.reduceMorale(5);
                    player.takeDamage(5);
                    activeEvent.narrative += "\n\n[No time. You tell yourself that.\n"
                                              "Everyone looks exhausted. Including you.]";
                } else {
                    // someone collapses
                    int dmg = 10 + rand() % 10;
                    partner.takeDamage(dmg);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[Your companion stumbles and goes down.\n"
                                              "They needed rest. " + std::to_string(dmg)
                                              + " damage from pushing too hard.]";
                }
            }},
        }
    });

    // 6. Toolbox ammo
    easyPool.push_back({ EventType::RESOURCE, "Tool Room", "assets/images/locations/storage.png",
        "A locked box. Your companion finds a crowbar and pops it.\n"
        "Ammunition — neatly stacked, still sealed.\n"
        "Someone was prepared. They're not here now.",
        {
            { "Take the ammo and go", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    // noise attracts something
                    int dmg = 8 + rand() % 10;
                    player.takeDamage(dmg);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Crowbar echoes. Something heard.\n"
                                              "You move fast. " + std::to_string(dmg) + " damage.]";
                } else if (roll < 70) {
                    int ammo = 3 + rand() % 5;
                    inventory.addAmmo(ammo);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[+" + std::to_string(ammo) + " ammo. Clean grab.]";
                } else {
                    // more than expected
                    int ammo = 6 + rand() % 6;
                    inventory.addAmmo(ammo);
                    inventory.addMedicine(1);
                    player.addMorale(12);
                    activeEvent.narrative += "\n\n[More than expected. +"
                        + std::to_string(ammo) + " ammo, +1 medicine.]";
                }
            }},
            { "Search further", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    // trap or attack
                    int dmg = 15 + rand() % 15;
                    player.takeDamage(dmg);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[Someone was still here.\n"
                                              "You take " + std::to_string(dmg)
                                              + " damage getting out. Found nothing.]";
                } else if (roll < 70) {
                    int ammo = 4 + rand() % 5;
                    int dmg  = 3 + rand() % 8;
                    inventory.addAmmo(ammo);
                    inventory.addMedicine(1);
                    player.takeDamage(dmg);
                    activeEvent.narrative += "\n\n[+" + std::to_string(ammo)
                        + " ammo, +1 medicine. Costs "
                        + std::to_string(dmg) + " health.]";
                } else {
                    // jackpot
                    int ammo = 6 + rand() % 6;
                    inventory.addAmmo(ammo);
                    inventory.addMedicine(2);
                    inventory.addFood(3);
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[Hidden compartment. +"
                        + std::to_string(ammo) + " ammo,\n+2 medicine, +3 food.]";
                }
            }},
        }
    });

    // 7. Dying stranger
    easyPool.push_back({ EventType::ENCOUNTER, "Narrow Passage", "assets/images/locations/tunnel.png",
        "An old man sits against the wall. He's not going to get up.\n"
        "He holds out a hand. 'Water. Please.'\n"
        "Your companion is already watching you.",
        {
            { "Give him water", [this]{
                int roll = rand() % 100;
                if (inventory.getWater() < 2) {
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[You don't have enough.\n"
                                              "You keep walking.]";
                } else if (roll < 15) {
                    // he had something bad — water was wrong call
                    inventory.removeWater(2);
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[He takes it. Grabs your wrist.\n"
                                              "You pull free and move on.\n"
                                              "Something felt wrong.]";
                } else if (roll < 70) {
                    inventory.removeWater(2);
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[He nods. Drinks slowly.\n"
                                              "Doesn't say thank you. Doesn't need to.]";
                } else {
                    // he rewards you
                    inventory.removeWater(2);
                    inventory.addAmmo(2);
                    player.addMorale(20);
                    activeEvent.narrative += "\n\n[He presses something into your hand.\n"
                                              "Two shells. All he has. +2 ammo.]";
                }
            }},
            { "Give food and water", [this]{
                int roll = rand() % 100;
                if (inventory.getFood() < 2 || inventory.getWater() < 2) {
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Not enough to give.\n"
                                              "You keep walking.]";
                } else if (roll < 20) {
                    // he takes it and asks for more
                    inventory.removeFood(4);
                    inventory.removeWater(3);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[He takes it. Then asks for more.\n"
                                              "You walk away. Lost 4 food, 3 water.]";
                } else if (roll < 75) {
                    giveToStranger(2, 2, 20, 2);
                    activeEvent.narrative += "\n\n[He reaches into his coat. Two shells.\n"
                                              "'All I have left.']";
                } else {
                    // extra reward
                    giveToStranger(2, 2, 30, 3);
                    inventory.addMedicine(1);
                    activeEvent.narrative += "\n\n[He hands you everything he had left.\n"
                                              "+2 ammo, +1 medicine.]";
                }
            }},
            { "Walk past him", [this]{
                int roll = rand() % 100;
                if (roll < 50) {
                    int hit = 12 + rand() % 10;
                    player.reduceMorale(hit);
                    activeEvent.narrative += "\n\n[Your companion doesn't say anything.\n"
                                              "Neither do you. -" + std::to_string(hit) + " morale.]";
                } else if (roll < 80) {
                    player.reduceMorale(18);
                    child.takeDamage(5);
                    activeEvent.narrative += "\n\n[Your companion stops.\n"
                                              "Looks back at him.\n"
                                              "Keeps walking. But something shifted.]";
                } else {
                    // he calls after you — information
                    player.addMorale(5);
                    inventory.addAmmo(1);
                    activeEvent.narrative += "\n\n['There's a gun two rooms back. Left wall.'\n"
                                              "You almost didn't hear it.\n"
                                              "+1 ammo.]";
                }
            }},
        }
    });

    // 8. Stranger asking for help
    easyPool.push_back({ EventType::ENCOUNTER, "Side Passage", "assets/images/locations/shelter.png",
        "A figure steps out of the dark. Hands raised, unarmed.\n"
        "'Please. I just need to get past this section. I'm alone.'\n"
        "They look scared. Or they want you to think they do.",
        {
            { "Let them travel with you", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    // they steal and run
                    int stolen = 3 + rand() % 4;
                    inventory.removeFood(std::min(stolen, inventory.getFood()));
                    player.reduceMorale(18);
                    activeEvent.narrative += "\n\n[Gone in the night. Along with "
                        + std::to_string(stolen) + " food.\n"
                        "You should have known.]";
                } else if (roll < 45) {
                    // they slow you down and eat your supplies
                    inventory.removeFood(std::min(3, inventory.getFood()));
                    inventory.removeWater(std::min(2, inventory.getWater()));
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[They eat more than their share.\n"
                                              "Split off without a word. -3 food, -2 water.]";
                } else if (roll < 80) {
                    // they help
                    inventory.removeFood(std::min(2, inventory.getFood()));
                    inventory.removeWater(std::min(2, inventory.getWater()));
                    inventory.addAmmo(3);
                    player.addMorale(20);
                    activeEvent.narrative += "\n\n[They pull their weight.\n"
                                              "Leave 3 shells before splitting off.]";
                } else {
                    // they help a lot
                    inventory.removeFood(std::min(2, inventory.getFood()));
                    inventory.addAmmo(4);
                    inventory.addMedicine(1);
                    player.addMorale(28);
                    activeEvent.narrative += "\n\n[They knew these tunnels.\n"
                                              "Saved you an hour. Left +4 ammo, +1 medicine.]";
                }
            }},
            { "Give food and send them away", [this]{
                int roll = rand() % 100;
                if (inventory.getFood() < 2) {
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Nothing to give.\n"
                                              "They nod and disappear.]";
                } else if (roll < 20) {
                    // they don't leave — follow you
                    inventory.removeFood(2);
                    player.reduceMorale(12);
                    player.takeDamage(8);
                    activeEvent.narrative += "\n\n[They take the food.\n"
                                              "Then follow you anyway.\n"
                                              "You have to get physical. 8 damage.]";
                } else if (roll < 75) {
                    giveToStranger(2, 0, 10);
                    activeEvent.narrative += "\n\n[They take it without argument.\n"
                                              "Go the other direction.]";
                } else {
                    // they leave something behind
                    giveToStranger(2, 0, 12);
                    inventory.addAmmo(1);
                    activeEvent.narrative += "\n\n[They press a shell into your hand before leaving.\n"
                                              "+1 ammo.]";
                }
            }},
            { "Block them", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    // they fight back
                    int dmg = 8 + rand() % 10;
                    player.takeDamage(dmg);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[They weren't as unarmed as they looked.\n"
                                              + std::to_string(dmg) + " damage before they run.]";
                } else if (roll < 70) {
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[They back away slowly.\n"
                                              "You don't watch them go.]";
                } else {
                    // they were actually dangerous — good call
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n[You hear them try another group further down.\n"
                                              "Screaming shortly after.\n"
                                              "Good call.]";
                }
            }},
        }
    });
}
void GameWorld::buildMedPool() {
    medPool.clear();

    // 1. Looter ambush
    medPool.push_back({ EventType::COMBAT, "Collapsed Corridor", "assets/images/locations/corridor.png",
        "A hand grabs your collar from behind a door.\n"
        "A looter slams you against the wall. Pipe raised.\n"
        "'Everything you have. Right now.'",
        {
            { "Fight back", [this]{
                int roll = rand() % 100;
                if (roll < 30) {
                    int dmg = 20 + rand() % 15;
                    player.takeDamage(dmg);
                    inventory.removeFood(std::min(3, inventory.getFood()));
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[He gets the better of you.\n"
                        + std::to_string(dmg) + " damage. He takes some food anyway.\n"
                        "He's still standing. Still dangerous.]";
                } else if (roll < 70) {
                    int dmg = 10 + rand() % 10;
                    player.takeDamage(dmg);
                    triggerCombat(60, 8, true);
                    activeEvent.narrative += "\n\n[First blow goes to him. "
                        + std::to_string(dmg) + " damage.\n"
                        "He's slowing down. Hurt but not done.]";
                } else {
                    int dmg = 5 + rand() % 5;
                    player.takeDamage(dmg);
                    inventory.addAmmo(2);
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[Quick. Brutal. He's down.\n"
                        "Only costs " + std::to_string(dmg) + " health. +2 ammo off the body.]";
                }
            }},
            { "Hand over food", [this]{
                int roll = rand() % 100;
                if (inventory.getFood() < 5) {
                    player.reduceMorale(8);
                    triggerCombat(60, 8, true);
                    activeEvent.narrative += "\n\n[Not enough to give.\n"
                        "He doesn't accept that answer.\n"
                        "He's still standing. Still dangerous.]";
                } else if (roll < 25) {
                    inventory.removeFood(std::min(8, inventory.getFood()));
                    inventory.removeWater(std::min(4, inventory.getWater()));
                    player.reduceMorale(22);
                    activeEvent.narrative += "\n\n[He takes the food.\n"
                        "Then takes the water too.\n"
                        "There was nothing you could do.]";
                } else if (roll < 75) {
                    inventory.removeFood(5);
                    player.reduceMorale(12);
                    activeEvent.narrative += "\n\n[He counts it. Nods.\n"
                        "Steps aside. -5 food.]";
                } else {
                    inventory.removeFood(5);
                    player.addMorale(5);
                    inventory.addAmmo(1);
                    activeEvent.narrative += "\n\n['Mutant three rooms ahead. Left side.'\n"
                        "He pockets the food and walks.\n"
                        "-5 food, +1 ammo.]";
                }
            }},
            { "Throw something and sprint", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    int dmg = 15 + rand() % 10;
                    player.takeDamage(dmg);
                    player.reduceMorale(18);
                    activeEvent.narrative += "\n\n[He's faster than he looked.\n"
                        + std::to_string(dmg) + " damage before you get clear.]";
                } else if (roll < 65) {
                    player.takeDamage(5);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[You get out. Barely.\n"
                        "5 damage from the scramble.]";
                } else {
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n[Clean escape.\n"
                        "You hear him shouting at the thing you threw.\n"
                        "Already gone.]";
                }
            }},
        }
    });

    // 2. Gas leak
    medPool.push_back({ EventType::HAZARD, "Industrial Section", "assets/images/locations/shaft.png",
        "The smell hits you first. Sharp. Wrong.\n"
        "Your companion starts coughing before you even see it.\n"
        "Gas. Leaking from a split pipe. You need to move.",
        {
            { "Sprint through", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    int dmg = 20 + rand() % 10;
                    player.takeDamage(dmg);
                    partner.takeDamage(dmg);
                    child.takeDamage(dmg - 5);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[Worse than it looked.\n"
                        + std::to_string(dmg) + " damage each.\n"
                        "Child takes " + std::to_string(dmg - 5) + ".]";
                } else if (roll < 70) {
                    player.takeDamage(12);
                    partner.takeDamage(12);
                    child.takeDamage(12);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[Everyone through. Everyone hurting.\n"
                        "12 damage each.]";
                } else {
                    player.takeDamage(5);
                    partner.takeDamage(5);
                    child.takeDamage(5);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[Faster than expected.\n"
                        "Only 5 damage each. You got lucky.]";
                }
            }},
            { "Wet cloth filter", [this]{
                int roll = rand() % 100;
                if (inventory.getWater() < 2) {
                    player.takeDamage(15);
                    partner.takeDamage(15);
                    child.takeDamage(15);
                    player.reduceMorale(12);
                    activeEvent.narrative += "\n\n[No water for the filter.\n"
                        "15 damage each going through dry.]";
                } else if (roll < 20) {
                    inventory.removeWater(2);
                    player.takeDamage(12);
                    partner.takeDamage(12);
                    child.takeDamage(12);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[Filter barely helped.\n"
                        "Wasted water. 12 damage each anyway.]";
                } else if (roll < 75) {
                    inventory.removeWater(2);
                    player.takeDamage(8);
                    partner.takeDamage(8);
                    child.takeDamage(8);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Filter helps. 8 damage each.\n"
                        "Better than nothing.]";
                } else {
                    inventory.removeWater(2);
                    player.takeDamage(3);
                    partner.takeDamage(3);
                    child.takeDamage(3);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[Worked well. Only 3 damage each.\n"
                        "Smart call.]";
                }
            }},
            { "Find another route", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    player.takeDamage(14);
                    partner.takeDamage(14);
                    child.takeDamage(14);
                    player.reduceMorale(14);
                    activeEvent.narrative += "\n\n[Route was blocked.\n"
                        "Ended up going through anyway. 14 damage each.]";
                } else if (roll < 55) {
                    if (inventory.getFood() >= 5) {
                        inventory.removeFood(5);
                        activeEvent.narrative += "\n\n[Long way around.\n"
                            "Costs 5 food but everyone's unharmed.]";
                    } else {
                        player.takeDamage(10);
                        partner.takeDamage(10);
                        child.takeDamage(10);
                        activeEvent.narrative += "\n\n[Ran out of alternate path.\n"
                            "Had to go through. 10 damage each.]";
                    }
                } else {
                    inventory.removeFood(std::min(3, inventory.getFood()));
                    int found = 1 + rand() % 3;
                    inventory.addMedicine(found);
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[Longer route but worth it.\n"
                        "Found +" + std::to_string(found) + " medicine on the way.]";
                }
            }},
        }
    });

    // 3. Starving family
    medPool.push_back({ EventType::MORAL, "Crossroads", "assets/images/locations/crossroads.png",
        "Three people block the tunnel. A man, a woman, and a child.\n"
        "All hollow-eyed. The child is barely standing.\n"
        "'Please. She hasn't eaten in two days.'",
        {
            { "Share generously", [this]{
                int roll = rand() % 100;
                if (inventory.getFood() < 5) {
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Not enough to share.\n"
                        "They watch you go.]";
                } else if (roll < 15) {
                    inventory.removeFood(5);
                    inventory.removeWater(3);
                    int moredmg = 3 + rand() % 3;
                    inventory.removeFood(std::min(moredmg, inventory.getFood()));
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[Others saw. Crowded in.\n"
                        "Lost more than planned.\n"
                        "-5 food, -3 water, -" + std::to_string(moredmg) + " more.]";
                } else if (roll < 70) {
                    inventory.removeFood(5);
                    inventory.removeWater(3);
                    player.addMorale(25);
                    activeEvent.narrative += "\n\n[The child eats without looking up.\n"
                        "The man mouths something.\n"
                        "You don't wait to hear it.]";
                } else {
                    inventory.removeFood(5);
                    inventory.removeWater(3);
                    inventory.addMedicine(1);
                    player.addMorale(32);
                    activeEvent.narrative += "\n\n[The woman presses a small kit into your hand.\n"
                        "'We found it. We can't use it.' +1 medicine.]";
                }
            }},
            { "Give a little", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    inventory.removeFood(std::min(5, inventory.getFood()));
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[You offer 2. They take 5.\n"
                        "The man looks apologetic.\n"
                        "The woman doesn't.]";
                } else if (roll < 75) {
                    if (inventory.getFood() >= 2) {
                        inventory.removeFood(2);
                        player.addMorale(10);
                    } else player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[Small. Not enough.\n"
                        "But something.]";
                } else {
                    inventory.removeFood(2);
                    inventory.addAmmo(2);
                    player.addMorale(18);
                    activeEvent.narrative += "\n\n['We found these. No use to us.'\n"
                        "+2 ammo.]";
                }
            }},
            { "Walk past", [this]{
                int roll = rand() % 100;
                if (roll < 40) {
                    player.reduceMorale(20);
                    child.takeDamage(5);
                    activeEvent.narrative += "\n\n[Your child pulls at your sleeve.\n"
                        "You keep walking.\n"
                        "They stop pulling.]";
                } else if (roll < 75) {
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[The woman says nothing.\n"
                        "That's worse than if she had.]";
                } else {
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n[You hear them try it on the next group.\n"
                        "Things turn ugly fast.\n"
                        "You were right not to stop.]";
                }
            }},
        }
    });

    // 4. Cave-in
    medPool.push_back({ EventType::HAZARD, "Rubble Field", "assets/images/locations/corridor.png",
        "The ceiling drops without warning.\n"
        "Your companion shoves the others clear and takes the impact.\n"
        "They're on their knees. Clutching their side. Breathing hard.",
        {
            { "Treat the injury immediately", [this]{
                int roll = rand() % 100;
                if (!inventory.hasMed()) {
                    partner.takeDamage(15);
                    player.reduceMorale(18);
                    activeEvent.narrative += "\n\n[Nothing to treat it with.\n"
                        "They push through. It costs them 15 health.]";
                } else if (roll < 15) {
                    inventory.removeMedicine(1);
                    partner.takeDamage(8);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[Something internal. Medicine helped\n"
                        "but not enough. Partner loses 8 health anyway.]";
                } else if (roll < 75) {
                    inventory.removeMedicine(1);
                    partner.heal(25);
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[They wince through the treatment.\n"
                        "Stand up. Nod. Keep moving.]";
                } else {
                    inventory.removeMedicine(1);
                    partner.heal(35);
                    inventory.addAmmo(2);
                    player.addMorale(18);
                    activeEvent.narrative += "\n\n[Back on their feet fast.\n"
                        "Spots something in the rubble. +2 ammo.]";
                }
            }},
            { "Help them up and keep going", [this]{
                int roll = rand() % 100;
                if (roll < 30) {
                    partner.takeDamage(25);
                    player.reduceMorale(20);
                    activeEvent.narrative += "\n\n[They made it two rooms.\n"
                        "Then went down hard.\n"
                        "Should have treated it. 25 damage.]";
                } else if (roll < 70) {
                    partner.takeDamage(10);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[They walk it off.\n"
                        "Mostly. 10 damage from the untreated injury.]";
                } else {
                    partner.takeDamage(3);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n['I've had worse.'\n"
                        "Maybe they have. Only 3 damage.]";
                }
            }},
        }
    });

    // 5. Marked supplies
    medPool.push_back({ EventType::RESOURCE, "Storage Vault", "assets/images/locations/storage.png",
        "A heavy steel door, ajar. Inside: medicine, ammo, food.\n"
        "A warning scratched into the metal:\n"
        "'MINE. Touch it and I will find you.'",
        {
            { "Take everything", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    inventory.addMedicine(2);
                    inventory.addAmmo(4);
                    inventory.addFood(5);
                    int dmg = 15 + rand() % 15;
                    player.takeDamage(dmg);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[Got it all. Owner came back.\n"
                        + std::to_string(dmg) + " damage getting out with it.]";
                } else if (roll < 65) {
                    inventory.addMedicine(2);
                    inventory.addAmmo(4);
                    inventory.addFood(5);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[+2 medicine, +4 ammo, +5 food.\n"
                        "Nobody came. Yet.]";
                } else {
                    inventory.addMedicine(3);
                    inventory.addAmmo(6);
                    inventory.addFood(6);
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[More than the warning suggested.\n"
                        "+3 medicine, +6 ammo, +6 food.]";
                }
            }},
            { "Take only medicine", [this]{
                int roll = rand() % 100;
                if (roll < 15) {
                    inventory.addMedicine(2);
                    int dmg = 8 + rand() % 8;
                    player.takeDamage(dmg);
                    player.reduceMorale(12);
                    activeEvent.narrative += "\n\n[They noticed anyway.\n"
                        + std::to_string(dmg) + " damage before you got clear.\n"
                        "Still have the medicine.]";
                } else if (roll < 75) {
                    inventory.addMedicine(2);
                    activeEvent.narrative += "\n\n[Quick. Quiet. +2 medicine.\n"
                        "Nobody saw.]";
                } else {
                    inventory.addMedicine(3);
                    activeEvent.narrative += "\n\n[There was more tucked behind.\n"
                        "+3 medicine.]";
                }
            }},
            { "Walk away", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    inventory.addFood(3);
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n[Wise call. On the way out\n"
                        "you spot a dropped pack. +3 food.]";
                } else if (roll < 65) {
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[Not worth the risk.\n"
                        "You keep moving.]";
                } else {
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[Someone behind you goes in.\n"
                        "Shouting shortly after.\n"
                        "You were right to walk away.]";
                }
            }},
        }
    });

    // 6. Two armed men
    medPool.push_back({ EventType::COMBAT, "Flooded Subway", "assets/images/locations/station.png",
        "Two men block the platform. Armed. They saw you first.\n"
        "Your companion steps in front of the others.\n"
        "'Stay behind me.'",
        {
            { "Fight together", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    partner.takeDamage(25);
                    player.takeDamage(15);
                    triggerCombat(75, 10, true);
                    activeEvent.narrative += "\n\n[Ambushed before you were ready.\n"
                        "Partner takes 25. You take 15.\n"
                        "They're still standing. This isn't over.]";
                } else if (roll < 70) {
                    partner.takeDamage(15);
                    triggerCombat(75, 10, true);
                    activeEvent.narrative += "\n\n[Partner takes the first hit. 15 damage.\n"
                        "They're slowing down. Hurt but not done.]";
                } else {
                    partner.takeDamage(5);
                    player.takeDamage(5);
                    inventory.addAmmo(3);
                    player.addMorale(12);
                    activeEvent.narrative += "\n\n[Surprised them. 5 damage each.\n"
                        "+3 ammo off the bodies.]";
                }
            }},
            { "Fire warning shots", [this]{
                int roll = rand() % 100;
                if (inventory.getAmmo() < 3) {
                    partner.takeDamage(15);
                    triggerCombat(75, 10, true);
                    activeEvent.narrative += "\n\n[Not enough ammo to bluff.\n"
                        "They called it.\n"
                        "They're still standing. Still dangerous.]";
                } else if (roll < 20) {
                    inventory.removeAmmo(3);
                    player.takeDamage(12);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[They shoot back. 12 damage.\n"
                        "They run but so do you. -3 ammo.]";
                } else if (roll < 70) {
                    inventory.removeAmmo(3);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[They scatter. -3 ammo. Clean escape.]";
                } else {
                    inventory.removeAmmo(3);
                    inventory.addAmmo(5);
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[One drops their gun running.\n"
                        "Net gain. -3 spent, +5 found.]";
                }
            }},
            { "Drop supplies and bolt", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    inventory.removeFood(5);
                    inventory.removeWater(5);
                    player.takeDamage(10);
                    player.reduceMorale(22);
                    activeEvent.narrative += "\n\n[They took the supplies and followed anyway.\n"
                        "-5 food, -5 water, 10 damage.]";
                } else if (roll < 70) {
                    inventory.removeFood(5);
                    inventory.removeWater(5);
                    player.reduceMorale(18);
                    activeEvent.narrative += "\n\n[They took it and stopped.\n"
                        "-5 food, -5 water. You're out.]";
                } else {
                    inventory.removeFood(3);
                    inventory.removeWater(2);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[They grabbed what hit the floor.\n"
                        "Didn't chase. -3 food, -2 water.]";
                }
            }},
        }
    });

    // 7. Generator room stranger
    medPool.push_back({ EventType::ENCOUNTER, "Generator Room", "assets/images/locations/shelter.png",
        "A working generator hums. Actual light.\n"
        "In the corner — a person. Wounded. Can't walk on their own.\n"
        "'I heard the generator. I thought maybe someone was still here.'",
        {
            { "Help them and take the kit", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    inventory.removeFood(std::min(5, inventory.getFood()));
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[Kit was empty.\n"
                        "You gave food you couldn't spare.\n"
                        "They couldn't even say thank you.]";
                } else if (roll < 70) {
                    if (inventory.getFood() >= 5) inventory.removeFood(5);
                    inventory.addMedicine(3);
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[Kit had what you needed.\n"
                        "+3 medicine. They seemed steadier when you left.]";
                } else {
                    if (inventory.getFood() >= 5) inventory.removeFood(5);
                    inventory.addMedicine(3);
                    inventory.addAmmo(2);
                    player.addMorale(22);
                    activeEvent.narrative += "\n\n['Safe route two lefts from here. No mutants.'\n"
                        "+3 medicine, +2 ammo.]";
                }
            }},
            { "Take the kit and leave", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[Kit was empty.\n"
                        "They watched you search it.\n"
                        "You left without a word.]";
                } else if (roll < 70) {
                    inventory.addMedicine(3);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[+3 medicine.\n"
                        "They didn't try to stop you.\n"
                        "That almost made it worse.]";
                } else {
                    inventory.addMedicine(4);
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[More than you expected.\n"
                        "+4 medicine. You don't look back.]";
                }
            }},
            { "Shut off the generator and leave", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    player.reduceMorale(20);
                    activeEvent.narrative += "\n\n[You hear something in the dark behind you\n"
                        "after the light goes out.\n"
                        "You walk faster.]";
                } else if (roll < 70) {
                    player.addMorale(3);
                    activeEvent.narrative += "\n\n[Less noise. Safer.\n"
                        "You don't think about what's left in the dark.]";
                } else {
                    player.addMorale(12);
                    activeEvent.narrative += "\n\n[Two rooms later someone has left\n"
                        "supplies near the wall with a note:\n"
                        "'Thanks for the dark. I needed to move.']";
                }
            }},
        }
    });

    // 8. Mutant in vent
    medPool.push_back({ EventType::COMBAT, "Vent Corridor", "assets/images/locations/shaft.png",
        "Something moves in the ventilation above you.\n"
        "Too big for a rat. Moving too deliberately.\n"
        "It drops before you can react.",
        {
            { "Open fire immediately", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    int dmg = 18 + rand() % 12;
                    player.takeDamage(dmg);
                    triggerCombat(80, 14, false);
                    activeEvent.narrative += "\n\n[First shot missed. It reached you.\n"
                        + std::to_string(dmg) + " damage before you got a clean shot.\n"
                        "It's still moving. Still coming.]";
                } else if (roll < 65) {
                    int dmg = 8 + rand() % 8;
                    player.takeDamage(dmg);
                    triggerCombat(80, 14, false);
                    activeEvent.narrative += "\n\n[Hit it. Didn't stop it.\n"
                        + std::to_string(dmg) + " damage to you.\n"
                        "It's slowing down. Hurt but not done.]";
                } else {
                    player.addMorale(12);
                    activeEvent.narrative += "\n\n[Clean shot. It drops.\n"
                        "Your hands shake for a minute after.]";
                }
            }},
            { "Throw food as bait and run", [this]{
                int roll = rand() % 100;
                if (inventory.getFood() < 3) {
                    triggerCombat(80, 14, false);
                    activeEvent.narrative += "\n\n[Nothing to throw.\n"
                        "It comes straight at you.\n"
                        "It's still standing. Still dangerous.]";
                } else if (roll < 20) {
                    inventory.removeFood(3);
                    int dmg = 12 + rand() % 10;
                    player.takeDamage(dmg);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[It ignored the food completely.\n"
                        + std::to_string(dmg) + " damage before you got distance.]";
                } else if (roll < 70) {
                    inventory.removeFood(3);
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[It went for the food.\n"
                        "You went the other direction. -3 food.]";
                } else {
                    inventory.removeFood(3);
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n[Running blind you find a shortcut.\n"
                        "Saved time. -3 food.]";
                }
            }},
            { "Hold still — 50/50", [this]{
                int roll = rand() % 100;
                if (roll < 50) {
                    triggerCombat(80, 14, false);
                    activeEvent.narrative += "\n\n[It saw you anyway.\n"
                        "Or smelled you.\n"
                        "It's still standing. Still dangerous.]";
                } else if (roll < 80) {
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n[It passed right by.\n"
                        "You didn't breathe for thirty seconds.]";
                } else {
                    player.addMorale(15);
                    inventory.addAmmo(2);
                    activeEvent.narrative += "\n\n[It moved on. Dropped something.\n"
                        "Ammo — human ammo.\n"
                        "From someone it already found. +2 ammo.]";
                }
            }},
        }
    });
}

void GameWorld::buildHardPool() {
    hardPool.clear();

    // 1. Large mutant
    hardPool.push_back({ EventType::COMBAT, "Abandoned Metro", "assets/images/locations/station.png",
        "Total silence.\n"
        "Then a sound — low, wet, wrong.\n"
        "Something massive drops from the ceiling. Too many limbs.\n"
        "It hasn't seen you yet. That won't last.",
        {
            { "Open fire", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    // gun jams or misses — it reaches you first
                    int dmg = 25 + rand() % 15;
                    player.takeDamage(dmg);
                    partner.takeDamage(15);
                    child.takeDamage(10);
                    triggerCombat(130, 22, false);
                    activeEvent.narrative += "\n\n[Missed. It covered the distance in one move.\n"
                        + std::to_string(dmg) + " damage to you. 15 to your family.\n"
                        "It's barely slowed down. This is bad.]";
                } else if (roll < 60) {
                    int dmg = 15 + rand() % 10;
                    player.takeDamage(dmg);
                    triggerCombat(130, 22, false);
                    activeEvent.narrative += "\n\n[You hit it. It didn't care.\n"
                        + std::to_string(dmg) + " damage to you before you got distance.\n"
                        "It's slowing down. Hurt but nowhere near done.]";
                } else {
                    int dmg = 8 + rand() % 8;
                    player.takeDamage(dmg);
                    triggerCombat(80, 22, false);
                    activeEvent.narrative += "\n\n[First shot landed clean.\n"
                        + std::to_string(dmg) + " damage to you from the initial lunge.\n"
                        "It's wounded. Still dangerous but you have the advantage.]";
                }
            }},
            { "Throw food as bait and run", [this]{
                int roll = rand() % 100;
                if (inventory.getFood() < 4) {
                    triggerCombat(130, 22, false);
                    activeEvent.narrative += "\n\n[Nothing to throw.\n"
                        "It turns toward the sound you made reaching for nothing.\n"
                        "It's still standing. Still dangerous.]";
                } else if (roll < 25) {
                    // ignores food entirely
                    inventory.removeFood(4);
                    int dmg = 20 + rand() % 15;
                    player.takeDamage(dmg);
                    partner.takeDamage(10);
                    player.reduceMorale(18);
                    activeEvent.narrative += "\n\n[It ignored the food.\n"
                        "Came straight for you.\n"
                        + std::to_string(dmg) + " damage to you. 10 to your companion.\n"
                        "You barely got out.]";
                } else if (roll < 65) {
                    inventory.removeFood(4);
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[It went for the food.\n"
                        "You ran. -4 food.\n"
                        "You can hear it eating behind you for a long time.]";
                } else {
                    // escape and find something while running
                    inventory.removeFood(4);
                    inventory.addAmmo(2);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[It went for the food.\n"
                        "Running blind you find a dead end with a body.\n"
                        "+2 ammo. -4 food.]";
                }
            }},
            { "Stay perfectly still", [this]{
                int roll = rand() % 100;
                if (roll < 45) {
                    // it finds you
                    triggerCombat(130, 22, false);
                    activeEvent.narrative += "\n\n[It turned slowly.\n"
                        "Stopped.\n"
                        "Looked directly at you.\n"
                        "It's still standing. Still dangerous.\n"
                        "It didn't need to see you.]";
                } else if (roll < 70) {
                    // it passes but grazes someone
                    int dmg = 10 + rand() % 8;
                    partner.takeDamage(dmg);
                    player.reduceMorale(12);
                    activeEvent.narrative += "\n\n[It passed within a metre.\n"
                        "Clipped your companion on the way.\n"
                        + std::to_string(dmg) + " damage. It didn't stop.\n"
                        "You don't move for another three minutes.]";
                } else {
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[It moved on.\n"
                        "You don't know why.\n"
                        "You don't want to think about why.\n"
                        "You keep moving.]";
                }
            }},
        }
    });

    // 2. Armed looters — coordinated ambush
    hardPool.push_back({ EventType::COMBAT, "Maintenance Shaft", "assets/images/locations/shaft.png",
        "Two armed looters. They had this tunnel watched.\n"
        "Your companion steps in front of the child.\n"
        "'Run. Take the child and run.'\n"
        "You both know you're not running.",
        {
            { "Fight together", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    // goes badly from the start
                    partner.takeDamage(30);
                    player.takeDamage(18);
                    triggerCombat(90, 12, true);
                    activeEvent.narrative += "\n\n[They were ready for you.\n"
                        "Companion takes 30. You take 18.\n"
                        "They're still standing. Barely.\n"
                        "So are you.]";
                } else if (roll < 65) {
                    partner.takeDamage(20);
                    triggerCombat(90, 12, true);
                    activeEvent.narrative += "\n\n[Companion takes the first hit. 20 damage.\n"
                        "They're slowing down. Hurt but not done.\n"
                        "You're in it now.]";
                } else {
                    // you surprise them
                    partner.takeDamage(8);
                    player.takeDamage(8);
                    inventory.addAmmo(4);
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[You moved faster than they expected.\n"
                        "8 damage each. +4 ammo off the bodies.\n"
                        "The child saw everything.]";
                }
            }},
            { "Fire to intimidate", [this]{
                int roll = rand() % 100;
                if (inventory.getAmmo() < 3) {
                    partner.takeDamage(20);
                    triggerCombat(90, 12, true);
                    activeEvent.narrative += "\n\n[Not enough ammo. They knew it.\n"
                        "Companion takes 20 damage.\n"
                        "They're still standing. Still dangerous.]";
                } else if (roll < 30) {
                    // they shoot back — harder
                    inventory.removeAmmo(3);
                    player.takeDamage(18);
                    partner.takeDamage(12);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[They shot back without hesitating.\n"
                        "18 to you. 12 to companion.\n"
                        "You got distance. Barely. -3 ammo.]";
                } else if (roll < 70) {
                    inventory.removeAmmo(3);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[They backed off.\n"
                        "-3 ammo. You move before they change their minds.]";
                } else {
                    // one panics and drops weapon
                    inventory.removeAmmo(3);
                    inventory.addAmmo(6);
                    player.addMorale(18);
                    activeEvent.narrative += "\n\n[One broke and ran.\n"
                        "Dropped their weapon.\n"
                        "-3 ammo spent. +6 ammo found.]";
                }
            }},
            { "Run — drop all food", [this]{
                int roll = rand() % 100;
                int f = inventory.getFood();
                inventory.removeFood(f);
                if (roll < 30) {
                    // they follow anyway
                    player.takeDamage(15);
                    partner.takeDamage(10);
                    player.reduceMorale(28);
                    activeEvent.narrative += "\n\n[You dropped everything.\n"
                        "They followed anyway.\n"
                        "15 to you. 10 to companion before you lost them.\n"
                        "All food gone. -" + std::to_string(f) + " food.]";
                } else if (roll < 70) {
                    player.reduceMorale(22);
                    activeEvent.narrative += "\n\n[They took the food.\n"
                        "Stopped chasing.\n"
                        "All " + std::to_string(f) + " food gone.\n"
                        "You don't speak for a long time after.]";
                } else {
                    // you find food while running
                    inventory.addFood(2);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[You ran. They took it.\n"
                        "Two rooms later you find a bag someone dropped.\n"
                        "-" + std::to_string(f) + " food lost. +2 food found.]";
                }
            }},
        }
    });

    // 3. Buried person
    hardPool.push_back({ EventType::MORAL, "Collapsed Hall", "assets/images/locations/corridor.png",
        "A voice from under the rubble. 'Help me. Please. I have children.'\n"
        "You can see a hand. Still moving.\n"
        "Freeing them would take time. And make noise.\n"
        "Your companion is already looking at you.",
        {
            { "Dig them out", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    // too late — and the noise attracted something
                    player.takeDamage(15);
                    partner.takeDamage(15);
                    player.reduceMorale(10);
                    triggerCombat(70, 12, true);
                    activeEvent.narrative += "\n\n[You got them out.\n"
                        "They didn't make it.\n"
                        "The noise brought company.\n"
                        "15 damage each. Still fighting.]";
                } else if (roll < 65) {
                    player.takeDamage(10);
                    partner.takeDamage(10);
                    inventory.addAmmo(2);
                    inventory.addMedicine(1);
                    player.addMorale(30);
                    activeEvent.narrative += "\n\n[You got them out. They were real.\n"
                        "10 damage each from the digging.\n"
                        "+2 ammo, +1 medicine. +30 morale.\n"
                        "They walked on their own.]";
                } else {
                    // they had more than expected
                    player.takeDamage(8);
                    partner.takeDamage(8);
                    inventory.addAmmo(4);
                    inventory.addMedicine(2);
                    inventory.addFood(3);
                    player.addMorale(38);
                    activeEvent.narrative += "\n\n[You got them out.\n"
                        "8 damage each. They gave everything they had.\n"
                        "+4 ammo, +2 medicine, +3 food.]";
                }
            }},
            { "Toss medicine and keep moving", [this]{
                int roll = rand() % 100;
                if (!inventory.hasMed()) {
                    player.reduceMorale(25);
                    activeEvent.narrative += "\n\n[Nothing to give.\n"
                        "You keep walking.\n"
                        "The voice gets quieter behind you.\n"
                        "Then stops.]";
                } else if (roll < 25) {
                    // they call after you — something bad
                    inventory.removeMedicine(1);
                    player.reduceMorale(20);
                    child.takeDamage(8);
                    activeEvent.narrative += "\n\n[You toss it down and keep walking.\n"
                        "They call after you.\n"
                        "Something about what's ahead.\n"
                        "You don't hear all of it.\n"
                        "Your child flinches at something further on.]";
                } else if (roll < 75) {
                    inventory.removeMedicine(1);
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[You toss it down without stopping.\n"
                        "You don't hear them after that.\n"
                        "You tell yourself that means it helped.]";
                } else {
                    // they call out useful information
                    inventory.removeMedicine(1);
                    inventory.addAmmo(2);
                    player.addMorale(20);
                    activeEvent.narrative += "\n\n['Left wall. Two rooms. Bag behind the pipe.'\n"
                        "You find it.\n"
                        "+2 ammo.]";
                }
            }},
            { "Walk away", [this]{
                int roll = rand() % 100;
                if (roll < 35) {
                    player.reduceMorale(30);
                    child.takeDamage(10);
                    partner.takeDamage(5);
                    activeEvent.narrative += "\n\n[The voice follows you.\n"
                        "Not literally.\n"
                        "Your companion doesn't speak for hours.\n"
                        "Your child won't look at you.]";
                } else if (roll < 70) {
                    player.reduceMorale(25);
                    activeEvent.narrative += "\n\n[You walk away.\n"
                        "The hand was still moving when you turned the corner.\n"
                        "-30 morale.]";
                } else {
                    // it was a trap — right call
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[Twenty metres later you hear it.\n"
                        "Not a person. Something using a person's voice.\n"
                        "You walk faster without looking back.]";
                }
            }},
        }
    });

    // 4. Flooding — forced sacrifice
    hardPool.push_back({ EventType::HAZARD, "Flooding Chamber", "assets/images/locations/flooded.png",
        "The chamber is flooding fast. Seconds, not minutes.\n"
        "Your pack is too heavy to swim with.\n"
        "You have to drop something. Right now.\n"
        "There is no good answer.",
        {
            { "Drop all food", [this]{
                int roll = rand() % 100;
                int f = inventory.getFood();
                inventory.removeFood(f);
                if (roll < 25) {
                    // swim goes wrong
                    int dmg = 15 + rand() % 10;
                    player.takeDamage(dmg);
                    child.takeDamage(10);
                    player.reduceMorale(20);
                    activeEvent.narrative += "\n\n[Current was stronger than it looked.\n"
                        + std::to_string(dmg) + " damage to you. 10 to the child.\n"
                        "All " + std::to_string(f) + " food gone.\n"
                        "Everyone made it. Barely.]";
                } else if (roll < 70) {
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[You make it through.\n"
                        "All " + std::to_string(f) + " food gone.\n"
                        "The water was cold enough that it hurts to breathe.]";
                } else {
                    // find food on the other side
                    inventory.addFood(3);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[-" + std::to_string(f) + " food.\n"
                        "On the other side someone's pack floated up.\n"
                        "+3 food recovered.]";
                }
            }},
            { "Drop all ammo", [this]{
                int roll = rand() % 100;
                int a = inventory.getAmmo();
                inventory.removeAmmo(a);
                if (roll < 25) {
                    int dmg = 12 + rand() % 10;
                    player.takeDamage(dmg);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[Still too heavy somehow.\n"
                        + std::to_string(dmg) + " damage fighting the current.\n"
                        "All " + std::to_string(a) + " ammo gone.]";
                } else if (roll < 70) {
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[You make it through.\n"
                        "All " + std::to_string(a) + " ammo gone.\n"
                        "Whatever's ahead you're facing it empty.]";
                } else {
                    // find ammo on other side
                    inventory.addAmmo(3);
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[-" + std::to_string(a) + " ammo.\n"
                        "Something caught in the grate on the other side.\n"
                        "+3 ammo.]";
                }
            }},
            { "Drop all medicine", [this]{
                int roll = rand() % 100;
                int m = inventory.getMedicine();
                inventory.removeMedicine(m);
                if (roll < 25) {
                    // someone gets sick immediately
                    child.takeDamage(20);
                    player.reduceMorale(18);
                    activeEvent.narrative += "\n\n[The cold water hits the child hard.\n"
                        "20 damage. No medicine left to treat it.\n"
                        "All " + std::to_string(m) + " medicine gone.]";
                } else if (roll < 70) {
                    player.reduceMorale(12);
                    activeEvent.narrative += "\n\n[You make it through.\n"
                        "All " + std::to_string(m) + " medicine gone.\n"
                        "Hope nobody gets hurt from here.]";
                } else {
                    inventory.addMedicine(1);
                    player.reduceMorale(6);
                    activeEvent.narrative += "\n\n[-" + std::to_string(m) + " medicine.\n"
                        "One vial survived in a sealed pocket.\n"
                        "+1 medicine.]";
                }
            }},
        }
    });

    // 5. Companion fever
    hardPool.push_back({ EventType::HAZARD, "Dark Passage", "assets/images/locations/tunnel.png",
        "Your companion collapses. Burning up. Shaking.\n"
        "They try to stand. They can't.\n"
        "'I'm fine. I'm fine.'\n"
        "They are not fine.",
        {
            { "Use 2 medicine — treat it properly", [this]{
                int roll = rand() % 100;
                if (inventory.getMedicine() >= 2) {
                    if (roll < 20) {
                        // treatment works but slowly
                        inventory.removeMedicine(2);
                        partner.heal(15);
                        player.reduceMorale(5);
                        activeEvent.narrative += "\n\n[Medicine helped. Not as much as you hoped.\n"
                            "+15 health. They're stable.\n"
                            "For now.]";
                    } else if (roll < 75) {
                        inventory.removeMedicine(2);
                        partner.heal(30);
                        player.addMorale(15);
                        activeEvent.narrative += "\n\n[It worked.\n"
                            "They're back on their feet.\n"
                            "+30 health.]";
                    } else {
                        // full recovery plus they push harder
                        inventory.removeMedicine(2);
                        partner.heal(40);
                        inventory.addAmmo(1);
                        player.addMorale(20);
                        activeEvent.narrative += "\n\n[Full recovery. Fast.\n"
                            "While you were treating them they spotted something nearby.\n"
                            "+40 health. +1 ammo.]";
                    }
                } else if (inventory.hasMed()) {
                    inventory.removeMedicine(1);
                    partner.heal(12);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Only one left.\n"
                        "It helps. Not enough.\n"
                        "+12 health. They're still weak.]";
                } else {
                    partner.takeDamage(35);
                    player.reduceMorale(35);
                    activeEvent.narrative += "\n\n[Nothing to give them.\n"
                        "You watch them get worse.\n"
                        "35 damage. They're still walking.\n"
                        "You don't know how.]";
                }
            }},
            { "Keep moving and hope it breaks", [this]{
                int roll = rand() % 100;
                if (roll < 50) {
                    // gets much worse
                    int dmg = 25 + rand() % 15;
                    partner.takeDamage(dmg);
                    player.reduceMorale(28);
                    activeEvent.narrative += "\n\n[It didn't break.\n"
                        "It got worse.\n"
                        + std::to_string(dmg) + " damage.\n"
                        "They're still moving. You don't know for how long.]";
                } else if (roll < 80) {
                    partner.takeDamage(15);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[Partially. 15 damage.\n"
                        "They're still weak but moving.\n"
                        "It could have been worse.]";
                } else {
                    // breaks completely on its own
                    partner.heal(8);
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[It broke on its own.\n"
                        "They sweat it out walking.\n"
                        "+8 health. You don't know how.]";
                }
            }},
        }
    });

    // 6. Ambush — net trap
    hardPool.push_back({ EventType::COMBAT, "Ambush Point", "assets/images/locations/corridor.png",
        "They were waiting. Three of them.\n"
        "A net drops from above — your companion is tangled.\n"
        "You have exactly one second.",
        {
            { "Fight — protect them", [this]{
                int roll = rand() % 100;
                if (roll < 25) {
                    // badly outnumbered
                    partner.takeDamage(35);
                    player.takeDamage(20);
                    triggerCombat(100, 15, true);
                    activeEvent.narrative += "\n\n[Three against one.\n"
                        "Companion takes 35 in the net.\n"
                        "You take 20 before you got a hand free.\n"
                        "They're still standing. All three of them.]";
                } else if (roll < 65) {
                    partner.takeDamage(25);
                    triggerCombat(100, 15, true);
                    activeEvent.narrative += "\n\n[Companion takes 25 in the net.\n"
                        "You're fighting. One on three.\n"
                        "They're slowing down. Barely.]";
                } else {
                    // you surprise them badly
                    partner.takeDamage(10);
                    inventory.addAmmo(3);
                    player.addMorale(15);
                    activeEvent.narrative += "\n\n[You moved before they expected.\n"
                        "Companion takes 10 getting free.\n"
                        "Two ran. One didn't get up.\n"
                        "+3 ammo.]";
                }
            }},
            { "Throw everything — food and water", [this]{
                int roll = rand() % 100;
                int food  = std::min(8, inventory.getFood());
                int water = std::min(8, inventory.getWater());
                inventory.removeFood(food);
                inventory.removeWater(water);
                if (roll < 30) {
                    // not enough — they want more
                    int more = std::min(4, inventory.getFood());
                    inventory.removeFood(more);
                    partner.takeDamage(15);
                    player.reduceMorale(30);
                    activeEvent.narrative += "\n\n[Not enough.\n"
                        "They took more. And hit your companion anyway.\n"
                        "-" + std::to_string(food) + " food, -"
                        + std::to_string(water) + " water, -"
                        + std::to_string(more) + " more food.\n"
                        "Companion takes 15 in the net.]";
                } else if (roll < 75) {
                    player.reduceMorale(25);
                    activeEvent.narrative += "\n\n[They took it.\n"
                        "Cut the net and walked.\n"
                        "-" + std::to_string(food) + " food, -"
                        + std::to_string(water) + " water.\n"
                        "You get your companion free.]";
                } else {
                    // one of them drops something
                    inventory.addAmmo(2);
                    player.reduceMorale(18);
                    activeEvent.narrative += "\n\n[They grabbed what they could and ran.\n"
                        "One dropped a pouch.\n"
                        "-" + std::to_string(food) + " food, -"
                        + std::to_string(water) + " water. +2 ammo.]";
                }
            }},
        }
    });

    // 7. Lone child
    hardPool.push_back({ EventType::MORAL, "Final Stretch", "assets/images/locations/exit.png",
        "A child — alone, no older than yours — sits in the tunnel.\n"
        "No adults in sight. Been here a while.\n"
        "They look at you without saying anything.\n"
        "Your companion is already looking at you.",
        {
            { "Take them with you", [this]{
                int roll = rand() % 100;
                int food  = std::min(3, inventory.getFood());
                int water = std::min(3, inventory.getWater());
                inventory.removeFood(food);
                inventory.removeWater(water);
                if (roll < 20) {
                    // they slow you down and attract trouble
                    player.takeDamage(12);
                    player.reduceMorale(5);
                    activeEvent.narrative += "\n\n[They panicked at a sound further on.\n"
                        "Brought something running.\n"
                        "12 damage getting clear.\n"
                        "-" + std::to_string(food) + " food, -" + std::to_string(water) + " water.]";
                } else if (roll < 70) {
                    player.addMorale(35);
                    activeEvent.narrative += "\n\n[They don't speak.\n"
                        "They keep up.\n"
                        "-" + std::to_string(food) + " food, -" + std::to_string(water) + " water.\n"
                        "+35 morale.]";
                } else {
                    // they knew something
                    player.addMorale(40);
                    inventory.addFood(2);
                    activeEvent.narrative += "\n\n[They lead you past a section\n"
                        "you would have walked straight into.\n"
                        "+40 morale. +2 food from their own pack.]";
                }
            }},
            { "Leave food and water and keep going", [this]{
                int roll = rand() % 100;
                int food  = std::min(3, inventory.getFood());
                int water = std::min(3, inventory.getWater());
                inventory.removeFood(food);
                inventory.removeWater(water);
                if (roll < 25) {
                    // your child reacts badly
                    child.takeDamage(8);
                    player.reduceMorale(8);
                    activeEvent.narrative += "\n\n[Your child stops walking.\n"
                        "Won't move for a full minute.\n"
                        "-" + std::to_string(food) + " food, -" + std::to_string(water) + " water.]";
                } else if (roll < 75) {
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[You leave it beside them.\n"
                        "Keep walking.\n"
                        "-" + std::to_string(food) + " food, -"
                        + std::to_string(water) + " water. +10 morale.]";
                } else {
                    // they call something after you
                    player.addMorale(18);
                    activeEvent.narrative += "\n\n[As you turn the corner they say something.\n"
                        "You don't catch all of it.\n"
                        "Something about the left wall two rooms ahead.\n"
                        "You check. They were right.]";
                }
            }},
            { "Keep moving", [this]{
                int roll = rand() % 100;
                if (roll < 40) {
                    player.reduceMorale(20);
                    child.takeDamage(10);
                    partner.takeDamage(5);
                    activeEvent.narrative += "\n\n[Your child reaches back toward them.\n"
                        "You keep walking.\n"
                        "Your companion puts a hand on your child's shoulder.\n"
                        "Nobody says anything for a long time.]";
                } else if (roll < 75) {
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[You walk past.\n"
                        "They don't say anything.\n"
                        "Neither do you.\n"
                        "-15 morale.]";
                } else {
                    // it wasn't what it looked like
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[Twenty metres later you hear voices.\n"
                        "Adults. Looking for them.\n"
                        "They were never alone.\n"
                        "You keep moving.]";
                }
            }},
        }
    });

    // 8. Stranger offering a deal
    hardPool.push_back({ EventType::ENCOUNTER, "Reactor Annex", "assets/images/locations/shelter.png",
        "Someone steps out of the dark. Armed — but hands low.\n"
        "'I know the way to the surface. Shortest route.\n"
        " No mutants. No looters. I've walked it twice.\n"
        " I need your medicine. All of it.'",
        {
            { "Take the deal", [this]{
                int roll = rand() % 100;
                int m = inventory.getMedicine();
                inventory.removeMedicine(m);
                if (roll < 25) {
                    // route was real but ambush on it
                    int dmg = 20 + rand() % 15;
                    player.takeDamage(dmg);
                    partner.takeDamage(15);
                    player.reduceMorale(15);
                    activeEvent.narrative += "\n\n[Route was real.\n"
                        "Someone else knew about it too.\n"
                        + std::to_string(dmg) + " to you. 15 to companion.\n"
                        "-" + std::to_string(m) + " medicine. No refunds.]";
                } else if (roll < 70) {
                    player.heal(20);
                    partner.heal(20);
                    child.heal(20);
                    player.addMorale(10);
                    activeEvent.narrative += "\n\n[Route was clean.\n"
                        "Saved hours. Saved health.\n"
                        "+20 health each.\n"
                        "-" + std::to_string(m) + " medicine.]";
                } else {
                    // route was even better — they left a stash
                    player.heal(25);
                    partner.heal(25);
                    child.heal(25);
                    inventory.addFood(4);
                    player.addMorale(20);
                    activeEvent.narrative += "\n\n[Route was clean and they left\n"
                        "a stash halfway through it.\n"
                        "+25 health each. +4 food.\n"
                        "-" + std::to_string(m) + " medicine.]";
                }
            }},
            { "Refuse", [this]{
                int roll = rand() % 100;
                if (roll < 20) {
                    // they warn you of something ahead anyway
                    player.addMorale(8);
                    activeEvent.narrative += "\n\n['Your loss.'\n"
                        "They walk.\n"
                        "Then stop. Turn back.\n"
                        "'Avoid the third junction left. Just avoid it.'\n"
                        "You do. You never find out what was there.]";
                } else if (roll < 65) {
                    player.reduceMorale(10);
                    activeEvent.narrative += "\n\n[They nod and disappear.\n"
                        "You take the long route.\n"
                        "-10 morale.]";
                } else {
                    // you find a shortcut on your own
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[You find your own way through.\n"
                        "Slower. But yours.\n"
                        "Nobody owes you anything for it.]";
                }
            }},
            { "Take the route by force", [this]{
                int roll = rand() % 100;
                if (roll < 30) {
                    // they were better than they looked
                    int dmg = 22 + rand() % 15;
                    player.takeDamage(dmg);
                    partner.takeDamage(18);
                    player.reduceMorale(15);
                    triggerCombat(85, 16, true);
                    activeEvent.narrative += "\n\n[They were better than they looked.\n"
                        + std::to_string(dmg) + " to you. 18 to companion.\n"
                        "They're still standing. Barely.\n"
                        "So are you.]";
                } else if (roll < 70) {
                    int dmg = 12 + rand() % 10;
                    player.takeDamage(dmg);
                    triggerCombat(85, 16, true);
                    activeEvent.narrative += "\n\n[" + std::to_string(dmg) + " damage before\n"
                        "you got the upper hand.\n"
                        "They're slowing down. Hurt but not done.]";
                } else {
                    // you overwhelm them fast
                    int dmg = 5 + rand() % 5;
                    player.takeDamage(dmg);
                    inventory.addMedicine(1);
                    player.addMorale(5);
                    activeEvent.narrative += "\n\n[Over fast.\n"
                        + std::to_string(dmg) + " damage to you.\n"
                        "They had medicine on them. +1.\n"
                        "They showed you the route before they couldn't anymore.]";
                }
            }},
        }
    });
}
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <algorithm>
#include <iomanip>

using namespace std;

// ==================== FAMILY MEMBER CLASS ====================
class FamilyMember {
public:
    string name;
    int health;        // 0-100, 0 = dead
    bool hasIllness;
    bool isAlive;
    string relationship;
    
    FamilyMember(string n, string rel) : name(n), health(100), hasIllness(false), isAlive(true), relationship(rel) {}
    
    void takeDamage(int damage) {
        if (!isAlive) return;
        health -= damage;
        if (health <= 0) {
            health = 0;
            isAlive = false;
            cout << "\n⚠️ TRAGEDY: " << name << " has died! ⚠️\n";
        } else {
            cout << name << " took " << damage << " damage. Health: " << health << "\n";
        }
    }
    
    void heal(int amount) {
        if (!isAlive) return;
        health += amount;
        if (health > 100) health = 100;
        cout << name << " healed by " << amount << ". Health: " << health << "\n";
    }
    
    void treatIllness() {
        if (hasIllness && isAlive) {
            hasIllness = false;
            cout << name << " has been cured of illness!\n";
        }
    }
    
    void display() const {
        string status = isAlive ? (hasIllness ? "Sick" : "Healthy") : "Deceased";
        cout << name << " (" << relationship << ") - Health: " << health << ", Status: " << status << "\n";
    }
};

// ==================== INVENTORY CLASS ====================
class Inventory {
public:
    int money;
    int rations;      // Food units
    int medicine;     // Medicine doses
    int protectiveGear; // Gas masks, etc.
    
    Inventory() : money(0), rations(0), medicine(0), protectiveGear(0) {}
    
    void display() const {
        cout << "\n📦 INVENTORY 📦\n";
        cout << "💰 Money: $" << money << "\n";
        cout << "🍞 Rations: " << rations << " units\n";
        cout << "💊 Medicine: " << medicine << " doses\n";
        cout << "🛡️ Protective Gear: " << protectiveGear << " pieces\n";
    }
    
    bool hasResources() const {
        return (rations > 0 || medicine > 0 || protectiveGear > 0);
    }
};

// ==================== STORE CLASS ====================
class Store {
public:
    static void displayStore(Inventory& inv, int difficulty) {
        cout << "\n🏪 PRE-GAME SHOPPING 🏪\n";
        cout << "You have $" << inv.money << " to prepare for the journey.\n\n";
        
        vector<pair<string, int>> prices = {
            {"Rations (10 units)", 20},
            {"Medicine (3 doses)", 30},
            {"Protective Gear (2 pieces)", 25}
        };
        
        bool shopping = true;
        while (shopping && inv.money > 0) {
            cout << "\nWhat would you like to buy?\n";
            cout << "1. " << prices[0].first << " - $" << prices[0].second << "\n";
            cout << "2. " << prices[1].first << " - $" << prices[1].second << "\n";
            cout << "3. " << prices[2].first << " - $" << prices[2].second << "\n";
            cout << "4. Finish shopping\n";
            cout << "Your money: $" << inv.money << "\nChoice: ";
            
            int choice;
            cin >> choice;
            
            if (choice == 4) break;
            
            if (choice >= 1 && choice <= 3) {
                int cost = prices[choice-1].second;
                if (inv.money >= cost) {
                    inv.money -= cost;
                    switch(choice) {
                        case 1: inv.rations += 10; cout << "Added 10 rations!\n"; break;
                        case 2: inv.medicine += 3; cout << "Added 3 medicine doses!\n"; break;
                        case 3: inv.protectiveGear += 2; cout << "Added 2 protective gear!\n"; break;
                    }
                } else {
                    cout << "Not enough money!\n";
                }
            }
        }
        cout << "\nShopping complete!\n";
        inv.display();
    }
    
    static void midGameTrade(Inventory& inv) {
        cout << "\n🤝 You encounter a traveling merchant... 🤝\n";
        cout << "1. Buy 5 rations for $15\n";
        cout << "2. Buy 1 medicine for $12\n";
        cout << "3. Sell 5 rations for $8\n";
        cout << "4. Ignore merchant\nChoice: ";
        
        int choice;
        cin >> choice;
        
        switch(choice) {
            case 1:
                if (inv.money >= 15) {
                    inv.money -= 15;
                    inv.rations += 5;
                    cout << "Bought 5 rations!\n";
                } else cout << "Not enough money!\n";
                break;
            case 2:
                if (inv.money >= 12) {
                    inv.money -= 12;
                    inv.medicine += 1;
                    cout << "Bought 1 medicine!\n";
                } else cout << "Not enough money!\n";
                break;
            case 3:
                if (inv.rations >= 5) {
                    inv.rations -= 5;
                    inv.money += 8;
                    cout << "Sold 5 rations!\n";
                } else cout << "Not enough rations!\n";
                break;
            default: cout << "You ignore the merchant.\n";
        }
    }
};

// ==================== EVENT CLASS ====================
class Event {
public:
    static void randomEncounter(vector<FamilyMember>& family, Inventory& inv, int& steps, int difficulty) {
        int eventRoll = rand() % 100;
        
        if (eventRoll < 15) { // 15% chance
            cout << "\n🎲 RANDOM ENCOUNTER! 🎲\n";
            int subEvent = rand() % 4;
            
            switch(subEvent) {
                case 0: { // Toxic gas leak
                    cout << "💨 Toxic gas fills the tunnel!\n";
                    if (inv.protectiveGear > 0) {
                        inv.protectiveGear--;
                        cout << "You use protective gear. Everyone is safe!\n";
                    } else {
                        cout << "No protective gear! Everyone takes damage!\n";
                        for (auto& member : family) {
                            if (member.isAlive) member.takeDamage(20 + (difficulty * 5));
                        }
                    }
                    break;
                }
                case 1: { // Find supplies
                    int found = 5 + rand() % 15;
                    inv.rations += found;
                    cout << "You find abandoned supplies! +" << found << " rations!\n";
                    break;
                }
                case 2: { // Enemy encounter
                    cout << "👥 Hostile survivors confront you!\n";
                    if (inv.protectiveGear > 0) {
                        cout << "You intimidate them with your gear. They back off.\n";
                    } else {
                        cout << "A fight breaks out! Family members get injured.\n";
                        for (auto& member : family) {
                            if (member.isAlive && (rand() % 2 == 0)) {
                                member.takeDamage(15);
                            }
                        }
                    }
                    break;
                }
                case 3: { // Disease outbreak
                    cout << "🦠 A disease spreads through your group!\n";
                    if (inv.medicine > 0) {
                        inv.medicine--;
                        cout << "You use medicine to prevent infection.\n";
                    } else {
                        for (auto& member : family) {
                            if (member.isAlive && !member.hasIllness && (rand() % 3 == 0)) {
                                member.hasIllness = true;
                                cout << member.name << " has fallen ill!\n";
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    
    static void dailyConsumption(vector<FamilyMember>& family, Inventory& inv) {
        int aliveCount = 0;
        for (auto& member : family) {
            if (member.isAlive) aliveCount++;
        }
        
        if (aliveCount == 0) return;
        
        // Consume rations
        int consumed = aliveCount;
        if (inv.rations >= consumed) {
            inv.rations -= consumed;
            cout << "🍽️ Consumed " << consumed << " rations.\n";
        } else {
            cout << "⚠️ NOT ENOUGH RATIONS! Everyone is starving! ⚠️\n";
            for (auto& member : family) {
                if (member.isAlive) member.takeDamage(15);
            }
        }
        
        // Illness effects
        for (auto& member : family) {
            if (member.isAlive && member.hasIllness) {
                member.takeDamage(5);
            }
        }
        
        // Medicine usage for illnesses
        for (auto& member : family) {
            if (member.isAlive && member.hasIllness && inv.medicine > 0) {
                cout << "Do you want to use medicine on " << member.name << "? (1=yes/0=no): ";
                int use;
                cin >> use;
                if (use == 1) {
                    inv.medicine--;
                    member.treatIllness();
                }
            }
        }
    }
};

// ==================== SCORING SYSTEM ====================
class ScoringSystem {
public:
    static int calculateScore(const vector<FamilyMember>& family, const Inventory& inv, int steps, int difficulty) {
        int score = 0;
        
        // Family survival points (each alive member = 100 points)
        int aliveCount = 0;
        for (const auto& member : family) {
            if (member.isAlive) {
                aliveCount++;
                score += 100;
                // Bonus for high health
                if (member.health > 80) score += 30;
                else if (member.health > 50) score += 15;
            }
        }
        
        // Resource points
        score += inv.money / 2;
        score += inv.rations * 3;
        score += inv.medicine * 10;
        score += inv.protectiveGear * 8;
        
        // Time efficiency (fewer steps = higher score)
        int timeBonus = max(0, 500 - (steps * 5));
        score += timeBonus;
        
        // Difficulty multiplier
        if (difficulty == 0) score *= 0.8;  // Easy - lower multiplier
        else if (difficulty == 1) score *= 1.0; // Medium
        else score *= 1.3; // Hard - higher multiplier for skilled play
        
        return score;
    }
    
    static void displayScoreboard(const vector<FamilyMember>& family, const Inventory& inv, int steps, int difficulty) {
        cout << "\n" << string(50, '=') << "\n";
        cout << "🏆 FINAL SCORE DASHBOARD 🏆\n";
        cout << string(50, '=') << "\n";
        
        int aliveCount = 0;
        int totalHealth = 0;
        for (const auto& member : family) {
            if (member.isAlive) {
                aliveCount++;
                totalHealth += member.health;
            }
        }
        
        cout << "\n👨‍👩‍👧‍👦 FAMILY STATUS:\n";
        cout << "   Survivors: " << aliveCount << "/" << family.size() << "\n";
        if (aliveCount > 0) {
            cout << "   Average Health: " << (totalHealth / aliveCount) << "%\n";
        }
        
        cout << "\n📦 REMAINING RESOURCES:\n";
        cout << "   💰 Money: $" << inv.money << "\n";
        cout << "   🍞 Rations: " << inv.rations << "\n";
        cout << "   💊 Medicine: " << inv.medicine << "\n";
        cout << "   🛡️ Gear: " << inv.protectiveGear << "\n";
        
        cout << "\n⏱️ JOURNEY STATS:\n";
        cout << "   Steps Taken: " << steps << "\n";
        cout << "   Difficulty: " << (difficulty == 0 ? "Easy" : difficulty == 1 ? "Medium" : "Hard") << "\n";
        
        int finalScore = calculateScore(family, inv, steps, difficulty);
        cout << "\n⭐ FINAL SCORE: " << finalScore << " ⭐\n";
        
        // Grade based on score
        cout << "\n🏅 RATING: ";
        if (finalScore >= 800) cout << "S - EXCELLENT! Master survivor!\n";
        else if (finalScore >= 600) cout << "A - Great! Your family is safe.\n";
        else if (finalScore >= 400) cout << "B - Good, but could be better.\n";
        else if (finalScore >= 200) cout << "C - Barely made it.\n";
        else cout << "D - Tragedy struck your family.\n";
        
        cout << string(50, '=') << "\n";
    }
};

// ==================== GAME ENGINE ====================
class GameEngine {
private:
    vector<FamilyMember> family;
    Inventory inventory;
    int difficulty;
    int steps;
    bool gameOver;
    
public:
    GameEngine() : steps(0), gameOver(false) {
        srand(time(0));
        setupDifficulty();
        createFamily();
        initialShopping();
    }
    
    void setupDifficulty() {
        cout << "\n🎮 SELECT DIFFICULTY 🎮\n";
        cout << "1. Easy - More starting money, abundant supplies\n";
        cout << "2. Medium - Balanced experience\n";
        cout << "3. Hard - Scarce resources, deadly encounters\n";
        cout << "Choice: ";
        
        int choice;
        cin >> choice;
        difficulty = choice - 1;
        
        switch(difficulty) {
            case 0:
                inventory.money = 300;
                inventory.rations = 20;
                inventory.medicine = 5;
                inventory.protectiveGear = 3;
                cout << "\n🌟 EASY MODE: You feel confident with your supplies.\n";
                break;
            case 1:
                inventory.money = 150;
                inventory.rations = 10;
                inventory.medicine = 2;
                inventory.protectiveGear = 1;
                cout << "\n⚖️ MEDIUM MODE: You must be careful with resources.\n";
                break;
            case 2:
                inventory.money = 75;
                inventory.rations = 5;
                inventory.medicine = 0;
                inventory.protectiveGear = 0;
                cout << "\n💀 HARD MODE: Survival will be a miracle!\n";
                break;
            default:
                difficulty = 1;
                inventory.money = 150;
                cout << "\nDefaulting to Medium mode.\n";
        }
    }
    
    void createFamily() {
        cout << "\n👨‍👩‍👧‍👦 CREATE YOUR FAMILY 👨‍👩‍👧‍👦\n";
        string name;
        
        cout << "Enter your name (Father/Mother): ";
        cin >> name;
        family.push_back(FamilyMember(name, "Parent"));
        
        cout << "Enter spouse's name: ";
        cin >> name;
        family.push_back(FamilyMember(name, "Spouse"));
        
        cout << "Enter first child's name: ";
        cin >> name;
        family.push_back(FamilyMember(name, "Child"));
        
        cout << "Enter second child's name: ";
        cin >> name;
        family.push_back(FamilyMember(name, "Child"));
        
        cout << "\nYour family:\n";
        for (auto& member : family) {
            member.display();
        }
    }
    
    void initialShopping() {
        Store::displayStore(inventory, difficulty);
    }
    
    void showStatus() {
        cout << "\n" << string(40, '-') << "\n";
        cout << "📍 STEP " << steps << " | Safe Zone Progress: " << min(100, steps * 10) << "%\n";
        inventory.display();
        cout << "\n👨‍👩‍👧‍👦 FAMILY STATUS:\n";
        for (auto& member : family) {
            member.display();
        }
        cout << string(40, '-') << "\n";
    }
    
    void checkGameOver() {
        bool anyoneAlive = false;
        for (auto& member : family) {
            if (member.isAlive) {
                anyoneAlive = true;
                break;
            }
        }
        
        if (!anyoneAlive) {
            cout << "\n💀 GAME OVER - Your entire family has perished! 💀\n";
            gameOver = true;
        }
        
        if (steps >= 10) { // 10 steps to reach safe zone
            cout << "\n🏁 You have reached the SAFE ZONE! 🏁\n";
            gameOver = true;
        }
    }
    
    void processDecision() {
        cout << "\n🎯 WHAT WILL YOU DO? 🎯\n";
        cout << "1. Move forward cautiously (low risk, slow progress)\n";
        cout << "2. Move quickly (higher risk, faster progress)\n";
        cout << "3. Rest and tend to family (heal 10 HP, consume 1 ration)\n";
        cout << "4. Look for supplies (risk encounter but may find items)\n";
        cout << "Choice: ";
        
        int choice;
        cin >> choice;
        
        switch(choice) {
            case 1:
                steps += 1;
                cout << "\nYou move cautiously through the tunnels...\n";
                Event::randomEncounter(family, inventory, steps, difficulty);
                Event::dailyConsumption(family, inventory);
                break;
                
            case 2:
                steps += 2;
                cout << "\nYou rush ahead, making good progress!\n";
                // Higher encounter chance
                if (rand() % 100 < 40) {
                    Event::randomEncounter(family, inventory, steps, difficulty);
                }
                Event::dailyConsumption(family, inventory);
                break;
                
            case 3:
                cout << "\nYou rest and care for your family.\n";
                if (inventory.rations >= 1) {
                    inventory.rations--;
                    for (auto& member : family) {
                        if (member.isAlive) {
                            member.heal(10);
                        }
                    }
                    cout << "Family rested and healed 10 HP.\n";
                } else {
                    cout << "No rations to rest properly!\n";
                }
                break;
                
            case 4:
                cout << "\nYou search for supplies...\n";
                if (rand() % 100 < 50) {
                    int found = 2 + rand() % 8;
                    inventory.rations += found;
                    cout << "Success! Found " << found << " rations!\n";
                } else {
                    cout << "You found nothing but attracted attention!\n";
                    Event::randomEncounter(family, inventory, steps, difficulty);
                }
                Event::dailyConsumption(family, inventory);
                break;
                
            default:
                cout << "Invalid choice! You waste time deciding.\n";
                steps++;
        }
        
        // Random merchant encounter (20% chance after each step)
        if (rand() % 100 < 20 && !gameOver) {
            Store::midGameTrade(inventory);
        }
    }
    
    void run() {
        cout << "\n💣💥 THE BOMB HAS DETONATED! 💥💣\n";
        cout << "You and your family must navigate the underground tunnels to reach the Safe Zone.\n";
        cout << "Make wise decisions to keep everyone alive!\n\n";
        
        while (!gameOver) {
            showStatus();
            processDecision();
            checkGameOver();
            
            if (gameOver) break;
            
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        }
        
        ScoringSystem::displayScoreboard(family, inventory, steps, difficulty);
    }
};

// ==================== MAIN FUNCTION ====================
int main() {
    cout << "\n";
    cout << "██████╗░ ░█████╗░ ███╗░░░███╗ ██████╗░ ███████╗\n";
    cout << "██╔══██╗ ██╔══██╗ ████╗░████║ ██╔══██╗ ██╔════╝\n";
    cout << "██████╔╝ ██║░░██║ ██╔████╔██║ ██████╔╝ █████╗░░\n";
    cout << "██╔══██╗ ██║░░██║ ██║╚██╔╝██║ ██╔══██╗ ██╔══╝░░\n";
    cout << "██║░░██║ ╚█████╔╝ ██║░╚═╝░██║ ██║░░██║ ███████╗\n";
    cout << "╚═╝░░╚═╝ ░╚════╝░ ╚═╝░░░░░╚═╝ ╚═╝░░╚═╝ ╚══════╝\n";
    cout << "         UNDERGROUND SURVIVAL GAME\n";
    cout << "===============================================\n\n";
    
    GameEngine game;
    game.run();
    
    cout << "\nThanks for playing!\n";
    return 0;
}
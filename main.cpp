#include <SFML/Graphics.hpp>
#include "Inventory.hpp"
#include <string>

enum class GameState {
    MAIN_MENU,
    DIFFICULTY,
    NAME_ENTRY,
    STORE,
    BOMB_CUTSCENE,
    UNDERGROUND,
    GAME_OVER
};
 
enum class Difficulty {
    EASY,
    MEDIUM,
    HARD
};

void applyDifficulty(Difficulty diff, Inventory& inventory) {
    if (diff == Difficulty::EASY)
        inventory.setMoney(1000);
    else if (diff == Difficulty::MEDIUM)
        inventory.setMoney(500);
    else
        inventory.setMoney(250);
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1280u, 720u}),
        "Survival Underground",
        sf::State::Windowed);

    window.setFramerateLimit(60);

    GameState state = GameState::MAIN_MENU;
    Difficulty difficulty = Difficulty::MEDIUM;

    Inventory inventory;

    sf::RectangleShape easyBtn({200.f, 60.f});
    easyBtn.setPosition({540.f, 250.f});
    easyBtn.setFillColor(sf::Color::Green);

    sf::RectangleShape medBtn({200.f, 60.f});
    medBtn.setPosition({540.f, 350.f});
    medBtn.setFillColor(sf::Color::Yellow);

    sf::RectangleShape hardBtn({200.f, 60.f});
    hardBtn.setPosition({540.f, 450.f});
    hardBtn.setFillColor(sf::Color::Red);

    sf::Font font;
    if (!font.openFromFile("assets\\fonts\\arial.ttf"))
        return -1;

    sf::Clock clock;

    while (window.isOpen()) {

        // ───────────── EVENT LOOP ─────────────
        while (const std::optional event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>())
                window.close();

            if (state == GameState::MAIN_MENU) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left) {
                        state = GameState::DIFFICULTY;
                    }
                }
            }

            if (state == GameState::DIFFICULTY) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left) {

                        sf::Vector2f mpos = window.mapPixelToCoords(
                            sf::Mouse::getPosition(window));

                        if (easyBtn.getGlobalBounds().contains(mpos)) {
                            difficulty = Difficulty::EASY;
                            applyDifficulty(difficulty, inventory);
                            state = GameState::NAME_ENTRY;
                        }
                        else if (medBtn.getGlobalBounds().contains(mpos)) {
                            difficulty = Difficulty::MEDIUM;
                            applyDifficulty(difficulty, inventory);
                            state = GameState::NAME_ENTRY;
                        }
                        else if (hardBtn.getGlobalBounds().contains(mpos)) {
                            difficulty = Difficulty::HARD;
                            applyDifficulty(difficulty, inventory);
                            state = GameState::NAME_ENTRY;
                        }
                    }
                }
            }
        }

        // ───────────── UPDATE ─────────────
        float dt = clock.restart().asSeconds();
        (void)dt;

        // ───────────── RENDER ─────────────
        window.clear(sf::Color::Black);

        if (state == GameState::MAIN_MENU) {

            sf::Text title(font, "SURVIVAL UNDERGROUND", 48);
            title.setPosition({350.f, 250.f});
            window.draw(title);

            sf::Text prompt(font, "Click to start", 22);
            prompt.setPosition({520.f, 350.f});
            window.draw(prompt);
        }

        if (state == GameState::DIFFICULTY) {

            sf::Text title(font, "SELECT DIFFICULTY", 40);
            title.setPosition({450.f, 150.f});
            window.draw(title);

            window.draw(easyBtn);
            window.draw(medBtn);
            window.draw(hardBtn);

            sf::Text moneyText(font, "Money: " + std::to_string(inventory.getMoney()), 20);
            moneyText.setPosition({50.f, 50.f});
            window.draw(moneyText);
        }

        if (state == GameState::NAME_ENTRY) {
            sf::Text t(font, "NAME ENTRY (TODO)", 30);
            t.setPosition({400.f, 300.f});
            window.draw(t);
        }

        window.display();
    }

    return 0;
}
#include <SFML/Graphics.hpp>
#include "Inventory.hpp"
#include "Store.hpp"
#include <string>
#include <optional>

// ─────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────
enum class GameState {
    MAIN_MENU,
    DIFFICULTY,
    CUTSCENE_1,
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

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────
void applyDifficulty(Difficulty diff, Inventory& inventory) {
    if      (diff == Difficulty::EASY)   inventory.setMoney(1000);
    else if (diff == Difficulty::MEDIUM) inventory.setMoney(500);
    else                                 inventory.setMoney(250);
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main() {
    sf::RenderWindow window(
        sf::VideoMode({1280u, 720u}),
        "Survival Underground",
        sf::State::Windowed
    );
    window.setFramerateLimit(60);

    // ── State ─────────────────────────────────
    GameState  state      = GameState::MAIN_MENU;
    Difficulty difficulty = Difficulty::MEDIUM;
    Inventory  inventory;

    // ── Font ──────────────────────────────────
    sf::Font font;
    if (!font.openFromFile("assets\\fonts\\arial.ttf"))
        return -1;

    // ── Store ─────────────────────────────────
    Store store(font);

    // ── Difficulty buttons ────────────────────
    sf::RectangleShape easyBtn({200.f, 60.f});
    easyBtn.setPosition({540.f, 250.f});
    easyBtn.setFillColor(sf::Color::Green);

    sf::RectangleShape medBtn({200.f, 60.f});
    medBtn.setPosition({540.f, 350.f});
    medBtn.setFillColor(sf::Color::Yellow);

    sf::RectangleShape hardBtn({200.f, 60.f});
    hardBtn.setPosition({540.f, 450.f});
    hardBtn.setFillColor(sf::Color::Red);

    // ── Cutscene 1 texture / sprite ───────────
    sf::Texture cutscene1Tex;
    std::optional<sf::Sprite> cutscene1Sprite;

    if (cutscene1Tex.loadFromFile("assets\\images\\cutscene1.png")) {
        cutscene1Sprite.emplace(cutscene1Tex);

        sf::Vector2u texSize = cutscene1Tex.getSize();
        float scaleX = 900.f / static_cast<float>(texSize.x);
        float scaleY = 480.f / static_cast<float>(texSize.y);
        float scale  = std::min(scaleX, scaleY);
        cutscene1Sprite->setScale({scale, scale});

        float scaledW = texSize.x * scale;
        cutscene1Sprite->setPosition({(1280.f - scaledW) / 2.f, 40.f});
    }

    sf::Clock clock;

    // ─────────────────────────────────────────
    //  Game loop
    // ─────────────────────────────────────────
    while (window.isOpen()) {

        // ── Events ────────────────────────────
        while (const std::optional event = window.pollEvent()) {

            if (event->is<sf::Event::Closed>()) {
                window.close();
                continue;
            }

            if (state == GameState::MAIN_MENU) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
                    if (mouse->button == sf::Mouse::Button::Left)
                        state = GameState::DIFFICULTY;
            }
            else if (state == GameState::DIFFICULTY) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left) {
                        sf::Vector2f mpos = window.mapPixelToCoords(
                            sf::Mouse::getPosition(window)
                        );
                        if (easyBtn.getGlobalBounds().contains(mpos)) {
                            difficulty = Difficulty::EASY;
                            applyDifficulty(difficulty, inventory);
                            state = GameState::CUTSCENE_1;
                        }
                        else if (medBtn.getGlobalBounds().contains(mpos)) {
                            difficulty = Difficulty::MEDIUM;
                            applyDifficulty(difficulty, inventory);
                            state = GameState::CUTSCENE_1;
                        }
                        else if (hardBtn.getGlobalBounds().contains(mpos)) {
                            difficulty = Difficulty::HARD;
                            applyDifficulty(difficulty, inventory);
                            state = GameState::CUTSCENE_1;
                        }
                    }
                }
            }
            else if (state == GameState::CUTSCENE_1) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
                    if (mouse->button == sf::Mouse::Button::Left)
                        state = GameState::STORE;
            }
            else if (state == GameState::STORE) {
                store.handleEvent(*event, window, inventory);
                if (store.isDone()) {
                    store.reset();
                    state = GameState::BOMB_CUTSCENE;
                }
            }
            else if (state == GameState::BOMB_CUTSCENE) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
                    if (mouse->button == sf::Mouse::Button::Left)
                        state = GameState::UNDERGROUND;
            }
            else if (state == GameState::UNDERGROUND) {
                // TODO: underground input
            }
            else if (state == GameState::GAME_OVER) {
                if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
                    if (mouse->button == sf::Mouse::Button::Left)
                        state = GameState::MAIN_MENU;
            }
        }

        // ── Update ────────────────────────────
        float dt = clock.restart().asSeconds();
        (void)dt;

        // ── Render ────────────────────────────
        window.clear(sf::Color::Black);

        if (state == GameState::MAIN_MENU) {
            sf::Text title(font, "SURVIVAL UNDERGROUND", 48);
            title.setPosition({350.f, 250.f});
            window.draw(title);

            sf::Text prompt(font, "Click anywhere to start", 22);
            prompt.setFillColor(sf::Color(180, 180, 180));
            prompt.setPosition({490.f, 350.f});
            window.draw(prompt);
        }
        else if (state == GameState::DIFFICULTY) {
            sf::Text title(font, "SELECT DIFFICULTY", 40);
            title.setPosition({450.f, 150.f});
            window.draw(title);

            window.draw(easyBtn);
            window.draw(medBtn);
            window.draw(hardBtn);

            sf::Text easyLabel(font, "EASY",   22); easyLabel.setFillColor(sf::Color::Black); easyLabel.setPosition({615.f, 265.f}); window.draw(easyLabel);
            sf::Text medLabel (font, "MEDIUM", 22); medLabel .setFillColor(sf::Color::Black); medLabel .setPosition({600.f, 365.f}); window.draw(medLabel);
            sf::Text hardLabel(font, "HARD",   22); hardLabel.setFillColor(sf::Color::Black); hardLabel.setPosition({615.f, 465.f}); window.draw(hardLabel);

            sf::Text moneyText(font, "Starting money: $" + std::to_string(inventory.getMoney()), 20);
            moneyText.setPosition({50.f, 50.f});
            window.draw(moneyText);
        }
        else if (state == GameState::CUTSCENE_1) {
            window.clear(sf::Color(10, 10, 20));

            if (cutscene1Sprite.has_value()) {
                window.draw(*cutscene1Sprite);
            } else {
                sf::RectangleShape placeholder({900.f, 480.f});
                placeholder.setPosition({190.f, 40.f});
                placeholder.setFillColor(sf::Color(40, 40, 60));
                placeholder.setOutlineColor(sf::Color(120, 120, 160));
                placeholder.setOutlineThickness(2.f);
                window.draw(placeholder);

                sf::Text missing(font, "[cutscene1.png not found]", 20);
                missing.setFillColor(sf::Color(150, 150, 180));
                missing.setPosition({460.f, 275.f});
                window.draw(missing);
            }

            sf::Text warning(font, "\"A BOMB IS ABOUT TO GO OFF!\"", 36);
            warning.setFillColor(sf::Color(255, 60, 60));
            warning.setStyle(sf::Text::Bold);
            sf::FloatRect wb = warning.getLocalBounds();
            warning.setOrigin({wb.size.x / 2.f, 0.f});
            warning.setPosition({640.f, 550.f});
            window.draw(warning);

            sf::Text continuePrompt(font, "[ click to continue ]", 18);
            continuePrompt.setFillColor(sf::Color(180, 180, 180));
            sf::FloatRect cb = continuePrompt.getLocalBounds();
            continuePrompt.setOrigin({cb.size.x / 2.f, 0.f});
            continuePrompt.setPosition({640.f, 650.f});
            window.draw(continuePrompt);
        }
        else if (state == GameState::STORE) {
            store.draw(window, inventory);   // Store handles its own clear + draw
        }
        else if (state == GameState::BOMB_CUTSCENE) {
            sf::Text t(font, "BOMB CUTSCENE (TODO)", 30);
            t.setPosition({460.f, 330.f});
            window.draw(t);

            sf::Text prompt(font, "[ click to continue ]", 18);
            prompt.setFillColor(sf::Color(180, 180, 180));
            prompt.setPosition({530.f, 400.f});
            window.draw(prompt);
        }
        else if (state == GameState::UNDERGROUND) {
            sf::Text t(font, "UNDERGROUND (TODO)", 30);
            t.setPosition({480.f, 330.f});
            window.draw(t);
        }
        else if (state == GameState::GAME_OVER) {
            sf::Text t(font, "GAME OVER", 60);
            t.setFillColor(sf::Color::Red);
            t.setPosition({460.f, 280.f});
            window.draw(t);

            sf::Text prompt(font, "Click to return to main menu", 22);
            prompt.setFillColor(sf::Color(180, 180, 180));
            prompt.setPosition({460.f, 380.f});
            window.draw(prompt);
        }

        window.display();
    }

    return 0;
}
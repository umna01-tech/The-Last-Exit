#include <SFML/Graphics.hpp>
#include "inventory.hpp"
#include "Store.hpp"
#include "Player.hpp"
#include "Partner.hpp"
#include "Child.hpp"
#include "GameWorld.hpp"
#include "HUD.hpp"
#include <string>
#include <optional>
#include <memory>

enum class GameState {
    MAIN_MENU,
    DIFFICULTY,
    CUTSCENE_1,
    STORE,
    BOMB_CUTSCENE,
    UNDERGROUND,
    GAME_OVER
};

enum class Difficulty { EASY, MEDIUM, HARD };

static void applyDifficulty(Difficulty diff, Inventory& inv) {
    if      (diff == Difficulty::EASY)   inv.setMoney(1000);
    else if (diff == Difficulty::MEDIUM) inv.setMoney(500);
    else                                 inv.setMoney(250);
}

struct GameSession {
    Inventory  inventory;
    Player     player;
    Partner    partner;
    Child      child;
    std::unique_ptr<GameWorld> world;
    std::unique_ptr<HUD>       hud;

    GameSession() : player(""), partner("", "", player), child("", player) {}
};

int main() {
    sf::RenderWindow window(sf::VideoMode({1280u, 720u}),
        "The Last Exit", sf::State::Windowed);
    window.setFramerateLimit(60);

    GameState  state      = GameState::MAIN_MENU;
    Difficulty difficulty = Difficulty::MEDIUM;

    auto session = std::make_unique<GameSession>();

    sf::Font font;
    if (!font.openFromFile("assets/fonts/arial.ttf")) return -1;

    // ── Difficulty buttons ────────────────────────────────────────────────────
    sf::RectangleShape easyBtn({200.f, 60.f});
    easyBtn.setPosition({540.f, 250.f});
    easyBtn.setFillColor(sf::Color::Green);

    sf::RectangleShape medBtn({200.f, 60.f});
    medBtn.setPosition({540.f, 350.f});
    medBtn.setFillColor(sf::Color::Yellow);

    sf::RectangleShape hardBtn({200.f, 60.f});
    hardBtn.setPosition({540.f, 450.f});
    hardBtn.setFillColor(sf::Color::Red);

    // ── Cutscene image ────────────────────────────────────────────────────────
    sf::Texture cs1Tex;
    std::optional<sf::Sprite> cs1Sprite;
    if (cs1Tex.loadFromFile("assets/images/cutscene1.png")) {
        cs1Sprite.emplace(cs1Tex);
        sf::Vector2u sz = cs1Tex.getSize();
        float sc = std::min(900.f / sz.x, 480.f / sz.y);
        cs1Sprite->setScale({sc, sc});
        cs1Sprite->setPosition({(1280.f - sz.x * sc) / 2.f, 40.f});
    }

    Store store(font);

    // ── Fade ──────────────────────────────────────────────────────────────────
    float     fadeAlpha = 0.f;
    bool      fadingOut = false, fadingIn = false;
    GameState nextState = GameState::MAIN_MENU;

    auto startFadeOut = [&](GameState next) {
        fadingOut = true;
        fadingIn  = false;
        nextState = next;
        fadeAlpha = 0.f;
    };

    // ── Centered text helper ──────────────────────────────────────────────────
    auto drawCentered = [&](const std::string& s, unsigned sz,
                            sf::Color col, float y, bool bold = false) {
        sf::Text t(font, s, sz);
        t.setFillColor(col);
        if (bold) t.setStyle(sf::Text::Bold);
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin({b.size.x / 2.f, 0.f});
        t.setPosition({640.f, y});
        window.draw(t);
    };

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // ══════════════════════════════════════════ EVENTS ════════════════════
        while (const std::optional ev = window.pollEvent()) {

            if (ev->is<sf::Event::Closed>()) { window.close(); continue; }

            // ── Main menu ─────────────────────────────────────────────────────
            if (state == GameState::MAIN_MENU) {
                if (const auto* m = ev->getIf<sf::Event::MouseButtonPressed>())
                    if (m->button == sf::Mouse::Button::Left)
                        state = GameState::DIFFICULTY;
            }

            // ── Difficulty ────────────────────────────────────────────────────
            else if (state == GameState::DIFFICULTY) {
                if (const auto* m = ev->getIf<sf::Event::MouseButtonPressed>()) {
                    if (m->button == sf::Mouse::Button::Left) {
                        sf::Vector2f mp = window.mapPixelToCoords(
                            sf::Mouse::getPosition(window));
                        if (easyBtn.getGlobalBounds().contains(mp)) {
                            difficulty = Difficulty::EASY;
                            applyDifficulty(difficulty, session->inventory);
                            state = GameState::CUTSCENE_1;
                        } else if (medBtn.getGlobalBounds().contains(mp)) {
                            difficulty = Difficulty::MEDIUM;
                            applyDifficulty(difficulty, session->inventory);
                            state = GameState::CUTSCENE_1;
                        } else if (hardBtn.getGlobalBounds().contains(mp)) {
                            difficulty = Difficulty::HARD;
                            applyDifficulty(difficulty, session->inventory);
                            state = GameState::CUTSCENE_1;
                        }
                    }
                }
            }

            // ── Cutscene ──────────────────────────────────────────────────────
            else if (state == GameState::CUTSCENE_1) {
                if (const auto* m = ev->getIf<sf::Event::MouseButtonPressed>())
                    if (m->button == sf::Mouse::Button::Left)
                        state = GameState::STORE;
            }

            // ── Store ─────────────────────────────────────────────────────────
            else if (state == GameState::STORE) {
                store.handleEvent(*ev, window, session->inventory);
                if (store.isDone()) {
                    store.reset();
                    startFadeOut(GameState::BOMB_CUTSCENE);
                }
            }

            // ── Bomb cutscene ─────────────────────────────────────────────────
            else if (state == GameState::BOMB_CUTSCENE) {
                if (const auto* m = ev->getIf<sf::Event::MouseButtonPressed>())
                    if (m->button == sf::Mouse::Button::Left)
                        startFadeOut(GameState::UNDERGROUND);
            }

            // ── Underground ───────────────────────────────────────────────────
            else if (state == GameState::UNDERGROUND && session->world && session->hud) {

                if (const auto* m = ev->getIf<sf::Event::MouseButtonPressed>()) {
                    if (m->button == sf::Mouse::Button::Left) {
                        sf::Vector2f mp = window.mapPixelToCoords(
                            sf::Mouse::getPosition(window));
                        // HUD handles all clicks — combat, choices, and
                        // the forward button — internally
                        session->hud->onMousePressed(mp);
                    }
                }

                if (const auto* m = ev->getIf<sf::Event::MouseMoved>()) {
                    sf::Vector2f mp = window.mapPixelToCoords(
                        {m->position.x, m->position.y});
                    if (session->hud) session->hud->onMouseMoved(mp);
                }

                if (const auto* m = ev->getIf<sf::Event::MouseButtonReleased>()) {
                    if (m->button == sf::Mouse::Button::Left) {
                        sf::Vector2f mp = window.mapPixelToCoords(
                            sf::Mouse::getPosition(window));
                        if (session->hud) session->hud->onMouseReleased(mp);
                    }
                }

                if (const auto* sw = ev->getIf<sf::Event::MouseWheelScrolled>())
                    if (session->hud) session->hud->onScroll(sw->delta);
            }

            // ── Game over ─────────────────────────────────────────────────────
            else if (state == GameState::GAME_OVER) {
                if (const auto* m = ev->getIf<sf::Event::MouseButtonPressed>()) {
                    if (m->button == sf::Mouse::Button::Left) {
                        // full reset — destroy old session, build fresh one
                        session = std::make_unique<GameSession>();
                        store.reset();
                        state = GameState::MAIN_MENU;
                    }
                }
            }
        }

        // ══════════════════════════════════════════ UPDATE ════════════════════

        if (fadingOut) {
            fadeAlpha += 220.f * dt;
            if (fadeAlpha >= 255.f) {
                fadeAlpha = 255.f;
                fadingOut = false;
                fadingIn  = true;
                state     = nextState;

                if (state == GameState::UNDERGROUND && !session->world) {
                    session->world = std::make_unique<GameWorld>(
                        session->player, session->partner,
                        session->child,  session->inventory);
                    session->hud = std::make_unique<HUD>(
                        font,
                        session->player, session->partner,
                        session->child,  session->inventory,
                        *session->world);
                    session->world->enterNextRoom();
                }
            }
        }

        if (fadingIn) {
            fadeAlpha -= 220.f * dt;
            if (fadeAlpha <= 0.f) { fadeAlpha = 0.f; fadingIn = false; }
        }

        // Check if game has ended via world endings
        if (state == GameState::UNDERGROUND && session->world) {
            if (session->world->isFinished())
                state = GameState::GAME_OVER;
        }

        // ══════════════════════════════════════════ RENDER ════════════════════
        window.clear(sf::Color(5, 5, 10));

        if (state == GameState::MAIN_MENU) {
            drawCentered("THE LAST EXIT",            52, sf::Color::White,       220.f, true);
            drawCentered("A survival story.",         20, sf::Color(140,140,140), 295.f);
            drawCentered("Click anywhere to begin.", 18, sf::Color(100,100,100), 370.f);
        }

        else if (state == GameState::DIFFICULTY) {
            drawCentered("HOW BAD IS IT OUT THERE?", 36, sf::Color::White, 150.f, true);
            window.draw(easyBtn);
            window.draw(medBtn);
            window.draw(hardBtn);
            sf::Text el(font,"EASY",  22); el.setFillColor(sf::Color::Black); el.setPosition({615.f,265.f}); window.draw(el);
            sf::Text ml(font,"MEDIUM",22); ml.setFillColor(sf::Color::Black); ml.setPosition({600.f,365.f}); window.draw(ml);
            sf::Text hl(font,"HARD",  22); hl.setFillColor(sf::Color::Black); hl.setPosition({615.f,465.f}); window.draw(hl);
            sf::Text mt(font, "Starting budget: $"
                + std::to_string(session->inventory.getMoney()), 16);
            mt.setFillColor(sf::Color(160,160,160));
            mt.setPosition({50.f, 50.f});
            window.draw(mt);
        }

        else if (state == GameState::CUTSCENE_1) {
            window.clear(sf::Color(10,10,20));
            if (cs1Sprite) window.draw(*cs1Sprite);
            else {
                sf::RectangleShape ph({900.f,480.f});
                ph.setPosition({190.f,40.f});
                ph.setFillColor(sf::Color(40,40,60));
                ph.setOutlineColor(sf::Color(120,120,160));
                ph.setOutlineThickness(2.f);
                window.draw(ph);
            }
            drawCentered("BREAKING NEWS — A bomb has detonated somewhere in the city.", 22, sf::Color(255,60,60),   540.f, true);
            drawCentered("Evacuation has collapsed. Panic. Fires. No signal.",           18, sf::Color(220,220,220), 575.f);
            drawCentered("You grab what you can and go underground.",                    18, sf::Color(255,200,50),  608.f);
            drawCentered("[ click to continue ]",                                        15, sf::Color(120,120,120), 680.f);
        }

        else if (state == GameState::STORE) {
            store.draw(window, session->inventory);
        }

        else if (state == GameState::BOMB_CUTSCENE) {
            window.clear(sf::Color(5,5,10));
            drawCentered("BOOM.",                                                  80, sf::Color(255,80,40),   160.f, true);
            drawCentered("The ground shakes. The sky turns red.",                 26, sf::Color(220,180,140), 300.f);
            drawCentered("You run. There is no choice anymore.",                  22, sf::Color(200,200,200), 350.f);
            drawCentered("20 sectors. 20 chances to die.",                        20, sf::Color(160,160,160), 400.f);
            drawCentered("Nothing ahead will be easy. Nothing will be fair.",     18, sf::Color(140,140,140), 435.f);
            drawCentered("Good luck.",                                            32, sf::Color(255,220,80),  490.f, true);
            drawCentered("[ click to begin ]",                                    16, sf::Color(120,120,120), 620.f);
        }

        else if (state == GameState::UNDERGROUND && session->hud) {
            session->hud->draw(window);
        }

        else if (state == GameState::GAME_OVER) {
            drawCentered("YOU DID NOT MAKE IT",          52, sf::Color(200,40,40),  240.f, true);
            drawCentered("The tunnels took everything.", 24, sf::Color(160,160,160), 340.f);
            drawCentered("Click to try again.",          18, sf::Color(100,100,100), 430.f);
        }

        // Fade overlay
        if (fadeAlpha > 0.f) {
            sf::RectangleShape fr({1280.f, 720.f});
            fr.setFillColor(sf::Color(0, 0, 0, static_cast<uint8_t>(fadeAlpha)));
            window.draw(fr);
        }

        window.display();
    }

    return 0;
}
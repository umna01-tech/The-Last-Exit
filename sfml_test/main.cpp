#include <SFML/Graphics.hpp>
#include "screens/Difficulty.h"
#include <string>

enum class Screen {
    MainMenu,
    Difficulty,
    NameEntry,
    Store,
    Blast,
    GamePlay,
    GameOver
};

int main() {
    sf::RenderWindow window(sf::VideoMode({1280, 720}), "THE LAST EXIT");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/fonts/arial.ttf")) 
    {
        return -1;
    }

    // Title
    sf::Text title(font, "THE LAST EXIT", 84);
    title.setFillColor(sf::Color::White);
    title.setPosition({350.f, 200.f});

    // Play button (simple rectangle + text)
    sf::RectangleShape playBtn({200.f, 100.f});
    playBtn.setFillColor(sf::Color(80, 80, 80));
    playBtn.setPosition({540.f, 380.f});

    sf::Text playText(font, "PLAY", 64);
    playText.setFillColor(sf::Color::White);
    playText.setPosition({580.f, 390.f});

    Screen currentScreen = Screen::MainMenu;

    while (window.isOpen()) {
        // --- Events ---
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::MouseButtonPressed>()) {
                auto& mouse = event->getIf<sf::Event::MouseButtonPressed>()->position;
                sf::Vector2f mousePos(mouse);

                if (currentScreen == Screen::MainMenu) {
                    if (playBtn.getGlobalBounds().contains(mousePos)) {
                        currentScreen = Screen::Difficulty;
                    }
                }
            }
        }

        // --- Draw ---
        window.clear(sf::Color::Black);

        if (currentScreen == Screen::MainMenu) {
            window.draw(title);
            window.draw(playBtn);
            window.draw(playText);
        } else if (currentScreen == Screen::Difficulty) {
            sf::Text msg(font, "Difficulty Screen - Coming Next!", 36);
            msg.setFillColor(sf::Color::White);
            msg.setPosition({300.f, 300.f});
            window.draw(msg);
        }

        window.display();
    }

    return 0;
}
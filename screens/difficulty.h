#pragma once
#include <SFML/Graphics.hpp>

class Difficulty {
public:
    sf::RectangleShape easyBtn, medBtn, hardBtn;
    sf::Text easyText, medText, hardText, title;
    int playerMoney = 0;

    Difficulty(sf::Font& font) :
        easyBtn({200.f, 60.f}),
        medBtn({200.f, 60.f}),
        hardBtn({200.f, 60.f}),
        title(font, "Choose Difficulty", 52),
        easyText(font, "EASY", 32),
        medText(font, "MEDIUM", 32),
        hardText(font, "HARD", 32)
    {
        // Title
        title.setFillColor(sf::Color::White);
        title.setPosition({400.f, 100.f});

        // Easy
        easyBtn.setFillColor(sf::Color(0, 150, 0));
        easyBtn.setPosition({540.f, 250.f});
        easyText.setFillColor(sf::Color::White);
        easyText.setPosition({610.f, 260.f});

        // Medium
        medBtn.setFillColor(sf::Color(150, 150, 0));
        medBtn.setPosition({540.f, 350.f});
        medText.setFillColor(sf::Color::White);
        medText.setPosition({580.f, 360.f});

        // Hard
        hardBtn.setFillColor(sf::Color(150, 0, 0));
        hardBtn.setPosition({540.f, 450.f});
        hardText.setFillColor(sf::Color::White);
        hardText.setPosition({610.f, 460.f});
    }

    void draw(sf::RenderWindow& window) {
        window.draw(title);
        window.draw(easyBtn);
        window.draw(easyText);
        window.draw(medBtn);
        window.draw(medText);
        window.draw(hardBtn);
        window.draw(hardText);
    }

    // returns true if a button was clicked
    bool handleClick(sf::Vector2f mousePos, int& money) {
        if (easyBtn.getGlobalBounds().contains(mousePos)) {
            money = 1000;
            return true;
        }
        if (medBtn.getGlobalBounds().contains(mousePos)) {
            money = 500;
            return true;
        }
        if (hardBtn.getGlobalBounds().contains(mousePos)) {
            money = 250;
            return true;
        }
        return false;
    }
};
#include "Fenetre.h"
#include "Cell.h"
#include "MainController.h"

int main() {
    Fenetre F(1300, 800);
    Cell start;
    Cell end;
    bool startPointDefined = false;
    bool endPointDefined = false;
    MainController game(F, 30, 50);
    sf::RenderWindow& window = F.getWindow();
    bool needsRedraw = true;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                break;
            }
            if (event.type == sf::Event::Resized || event.type == sf::Event::GainedFocus) {
                needsRedraw = true;
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left) {
                const auto mousePos = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
                game.handleMouseClick(F, start, end, startPointDefined, endPointDefined, mousePos);
                needsRedraw = true;
            }
        }
        if (!window.isOpen()) {
            break;
        }
        if (needsRedraw) {
            window.clear(sf::Color::Black);
            game.drawGrid(F);
            game.drawChecklist(F);
            game.drawButtons(F);
            window.display();
            needsRedraw = false;
        } else {
            // Keep processing input without rendering an unchanged screen.
            sf::sleep(sf::milliseconds(8));
        }
    }
    return 0;
}

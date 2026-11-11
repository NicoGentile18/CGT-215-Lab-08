#include <iostream>
#include <algorithm>
#include <string>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>

using namespace std;
using namespace sf;
using namespace sfp;

int main() {
    RenderWindow window(VideoMode(800, 600), "Duck Hunter", Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);
    window.setKeyRepeatEnabled(false);

    Texture crossbowTexture;
    Texture arrowTexture;
    Texture duckTexture;
    Image duckImage;
    Font font;

    if (!crossbowTexture.loadFromFile("images/crossbow.png") ||
        !arrowTexture.loadFromFile("images/arrow.png") ||
        !duckImage.loadFromFile("images/duck.png") ||
        !font.loadFromFile("DejaVuSans.ttf")) {
        cout << "Could not load an asset. Keep the images folder and font beside the project." << endl;
        return 1;
    }

    duckImage.flipHorizontally();
    if (!duckTexture.loadFromImage(duckImage)) {
        return 1;
    }

    Sprite crossbow(crossbowTexture);
    crossbow.setOrigin(30, 30);
    crossbow.setPosition(400, 550);

    World world(Vector2f(0, 0));
    world.setIgnoreMovement(false);
    PhysicsShapeList<PhysicsSprite> ducks;

    PhysicsRectangle rightBoundary;
    rightBoundary.setSize(Vector2f(20, 600));
    rightBoundary.setCenter(Vector2f(910, 300));
    rightBoundary.setStatic(true);
    world.AddPhysicsBody(rightBoundary);

    PhysicsSprite arrow;
    arrow.setTexture(arrowTexture);

    int score = 0;
    int arrowsLeft = 5;
    int spawnTimerMS = 0;
    int duckRow = 0;
    bool arrowActive = false;
    bool gameOver = false;

    rightBoundary.onCollision = [&world, &ducks, &arrow](PhysicsBodyCollisionResult result) {
        if (result.object2 != arrow) {
            PhysicsSprite& duck = static_cast<PhysicsSprite&>(result.object2);
            world.RemovePhysicsBody(duck);
            ducks.QueueRemove(duck);
        }
    };

    arrow.onCollision = [&world, &ducks, &arrow, &arrowActive, &score, &rightBoundary](PhysicsBodyCollisionResult result) {
        if (arrowActive && result.object2 != rightBoundary) {
            PhysicsSprite& duck = static_cast<PhysicsSprite&>(result.object2);
            world.RemovePhysicsBody(duck);
            ducks.QueueRemove(duck);
            world.RemovePhysicsBody(arrow);
            arrowActive = false;
            score++;
        }
    };

    Text arrowsText("", font, 24);
    arrowsText.setPosition(20, 555);
    arrowsText.setFillColor(Color::White);
    arrowsText.setOutlineColor(Color::Black);
    arrowsText.setOutlineThickness(1);

    Text scoreText("", font, 24);
    scoreText.setFillColor(Color::White);
    scoreText.setOutlineColor(Color::Black);
    scoreText.setOutlineThickness(1);

    Text gameOverText("GAME OVER", font, 54);
    gameOverText.setFillColor(Color::White);
    gameOverText.setOutlineColor(Color::Black);
    gameOverText.setOutlineThickness(3);
    FloatRect gameOverBounds = gameOverText.getLocalBounds();
    gameOverText.setOrigin(gameOverBounds.left + gameOverBounds.width / 2,
                           gameOverBounds.top + gameOverBounds.height / 2);
    gameOverText.setPosition(400, 300);

    Clock clock;

    while (window.isOpen()) {
        int deltaTimeMS = min(static_cast<int>(clock.restart().asMilliseconds()), 30);

        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space) {
                if (gameOver) {
                    window.close();
                }
                else if (!arrowActive && arrowsLeft > 0) {
                    arrow.setCenter(Vector2f(400, 500));
                    arrow.setVelocity(Vector2f(0, -0.5f));
                    world.AddPhysicsBody(arrow);
                    arrowActive = true;
                    arrowsLeft--;
                }
            }
        }

        if (!window.isOpen()) {
            break;
        }

        if (!gameOver) {
            spawnTimerMS -= deltaTimeMS;
            if (spawnTimerMS <= 0) {
                PhysicsSprite& duck = ducks.Create();
                duck.setTexture(duckTexture);
                duck.setSize(Vector2f(80, 80));
                duck.setCenter(Vector2f(-40, 90 + 45 * (duckRow % 3)));
                duck.setVelocity(Vector2f(0.12f, 0));
                world.AddPhysicsBody(duck);
                duckRow++;
                spawnTimerMS += 1200;
            }

            if (deltaTimeMS > 0) {
                world.UpdatePhysics(deltaTimeMS);
            }

            if (arrowActive && arrow.getCenter().y < -60) {
                world.RemovePhysicsBody(arrow);
                arrowActive = false;
            }

            world.UpdatePhysics(0, 1);
            ducks.DoRemovals();

            if (arrowsLeft == 0 && !arrowActive) {
                gameOver = true;
            }
        }

        arrowsText.setString("Arrows: " + to_string(arrowsLeft));
        scoreText.setString("Score: " + to_string(score));
        FloatRect scoreBounds = scoreText.getLocalBounds();
        scoreText.setPosition(780 - scoreBounds.left - scoreBounds.width, 555);

        window.clear(Color(90, 170, 225));
        for (PhysicsSprite& duck : ducks) {
            window.draw(duck);
        }
        window.draw(crossbow);
        if (arrowActive) {
            window.draw(arrow);
        }
        window.draw(arrowsText);
        window.draw(scoreText);
        if (gameOver) {
            window.draw(gameOverText);
        }
        window.display();
    }

    for (PhysicsSprite& duck : ducks) {
        world.RemovePhysicsBody(duck);
        ducks.QueueRemove(duck);
    }
    world.UpdatePhysics(0, 1);
    ducks.DoRemovals();

    return 0;
}

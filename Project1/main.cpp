#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

int main() {
    RenderWindow window(VideoMode::getDesktopMode(), "shtasgm", Style::None); // fullscreen window
    window.setMouseCursorVisible(false); // hide cursor
    Clock clock;

    // textures and font
    const Texture bg1Sprite("bg_1.png");
    const Texture bg2Sprite("bg_2.png");
    const Texture bg3Sprite("bg_3.png");
    const Texture playerSprite("char_spritesheet.png");
    const Font font("font.ttf");

    // sprites
    Sprite bg1(bg1Sprite);
    Sprite bg2(bg2Sprite);
    Sprite bg3(bg3Sprite);
    Sprite player(playerSprite);

    // text overlay
    Text text(font, "A or D : Move Left or Right\nSpace : Jump\nLeft Click : Attack\nBackspace : KYS\nDelete : Respawn\nEscape : Exit Game", 30);
    text.setPosition({ 20.f, 10.f });
    text.setOutlineColor(Color::Black);
    text.setOutlineThickness(3);

    // sprite dimensions (in px)
    const int frameWidth = 56;
    const int frameHeight = 56;

    // movement and animation settings
    const float animationSpeed = 0.075f; // seconds per frame
    const float moveSpeed = 250.f;       // movement speed multiplier
    const float jumpSpeed = -250.f;      // jump speed multiplier
    const float ground = 751.f;          // Y position of the ground
    const float gravity = 980.f;         // gravity force
    Vector2f velocity(0.f, 0.f);         // velocity of the player on both axes
    float frameTimer = 0.f;              // animation frame timer

    // animation frame indices
    int totalIdle = 6;
    int idleX = 0;
    int idleY = 0;
    int totalRun = 8;
    int runX = 0;
    int runY = 2;
    int totalAttack = 6;
    int attackX = 0;
    int attackY = 1;
    int totalJump = 5;
    int jumpX = 0;
    int jumpY = 3;
    int totalFall = 5;
    int fallX = 0;
    int fallY = 4;
    int totalDeath1 = 8;
    int totalDeath2 = 4;
    int deathX = 0;
    int deathY = 5;

    // player states
    int onGround = 0;
    int attacking = 0;
    int jumping = 0;
    int falling = 0;
    int moving = 0;
    int dying = 0;
    int dead = 0;

    // setup player initial position and scaling
    player.setScale({ 4.f, 4.f }); // scale up sprite size 4x
    player.setOrigin({ frameWidth, 0 }); // set origin for flipping
    player.setPosition({ 33 + 4 * frameWidth, ground + 0.5f });

    // main game loop
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds(); // frame delta time

        // poll system events
        while (optional event = window.pollEvent()) {
            if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
                window.close(); // closes the window if escape is pressed
            }
        }

        // initialize movement vector
        velocity.x = 0.f;
        moving = 0;

        // handle input
        if (dead) {
            // allow respawn if dead
            if (Keyboard::isKeyPressed(Keyboard::Key::Delete)) {
                dead = 0;
                player.setPosition({ 33 + 4 * frameWidth, ground + 0.5f });
            }
        }
        else {
            // only accept movement and action input if not dying or attacking
            if (!dying && !attacking) {
                // move right
                if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
                    moving = 1;
                    velocity.x += moveSpeed;
                    player.setScale({ 4.f, 4.f }); // face right
                    player.setOrigin({ frameWidth, 0 });
                }
                // move left
                if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
                    moving = 1;
                    velocity.x -= moveSpeed;
                    player.setScale({ -4.f, 4.f }); // face left (flip horizontally)
                    player.setOrigin({ 0, 0 });
                }
                // jump
                if (Keyboard::isKeyPressed(Keyboard::Key::Space) && onGround) {
                    velocity.y = jumpSpeed;
                    jumpX = 0;
                    fallX = 0;
                    onGround = 0;
                }
                // attack (only allowed when idle or moving)
                if (Mouse::isButtonPressed(Mouse::Button::Left) && !jumping && !falling && !attacking) {
                    attacking = 1;
                    attackX = 0;
                }
                // trigger death animation
                if (Keyboard::isKeyPressed(Keyboard::Key::Backspace)) {
                    dying = 1;
                }
            }
        }

        velocity.y += gravity * dt; // apply gravity
        player.move(velocity * dt); // apply horizontal and vertical movement

        // determine player state in the air
        if (!onGround) {
            if (velocity.y < 0) {
                jumping = 1;
            }
            else if (velocity.y > 0) {
                falling = 1;
            }
        }

        // clamp player to the ground
        if (player.getPosition().y >= ground) {
            player.setPosition({ player.getPosition().x, ground });
            velocity.y = 0.f;
            onGround = 1;
        }
        else {
            onGround = 0;
        }

        // handle animation states
        if (jumping) {
            // jumping animation
            frameTimer += dt;
            if (frameTimer >= animationSpeed) {
                frameTimer = 0.f;
                jumpX++;
                if (jumpX >= totalJump) {
                    jumping = 0;
                    jumpX = 0;
                }
                else {
                    player.setTextureRect(IntRect({ jumpX * frameWidth, jumpY * frameHeight }, { frameWidth, frameHeight }));
                }
            }
        }
        else if (falling) {
            // falling animation
            frameTimer += dt;
            if (frameTimer >= animationSpeed) {
                frameTimer = 0.f;
                fallX++;
                if (fallX >= totalFall) {
                    falling = 0;
                    fallX = 0;
                }
                else {
                    player.setTextureRect(IntRect({ fallX * frameWidth, fallY * frameHeight }, { frameWidth, frameHeight }));
                }
            }
        }
        else if (attacking) {
            // attacking animation
            velocity.x = 0.f; // freeze horizontal movement while attacking
            frameTimer += dt;
            if (frameTimer >= animationSpeed) {
                frameTimer = 0.f;
                attackX++;
                if (attackX >= totalAttack) {
                    attacking = 0;
                    attackX = 0;
                }
                else {
                    player.setTextureRect(IntRect({ attackX * frameWidth, attackY * frameHeight }, { frameWidth, frameHeight }));
                }
            }
        }
        else if (moving) {
            // running animation
            frameTimer += dt;
            if (frameTimer >= animationSpeed) {
                frameTimer = 0.f;
                runX = (runX + 1) % totalRun;
                player.setTextureRect(IntRect({ runX * frameWidth, runY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (dying) {
            // death animation
            frameTimer += dt;
            if (frameTimer >= animationSpeed) {
                frameTimer = 0.f;
                deathX++;
                if (deathX >= totalDeath1 && deathY == 5) {
                    deathX = 0;
                    deathY = 6;
                }
                else if (deathX >= totalDeath2 && deathY == 6) {
                    deathX = 0;
                    deathY = 5;
                    dying = 0;
                    dead = 1;
                }
                else {
                    player.setTextureRect(IntRect({ deathX * frameWidth, deathY * frameHeight }, { frameWidth, frameHeight }));
                }
            }
        }
        else if (dead) {
            // show death still frame
            player.setTextureRect(IntRect({ frameWidth * 3, frameHeight * 6 }, { frameWidth, frameHeight }));
        }
        else {
            // idle animation
            frameTimer += dt;
            if (frameTimer >= animationSpeed) {
                frameTimer = 0.f;
                idleX = (idleX + 1) % totalIdle;
                player.setTextureRect(IntRect({ idleX * frameWidth, idleY * frameHeight }, { frameWidth, frameHeight }));
            }
        }

        // render all objects
        window.clear();
        window.draw(bg3);
        window.draw(bg2);
        window.draw(bg1);
        window.draw(text);
        window.draw(player);
        window.display();
    }
    return 0;
}

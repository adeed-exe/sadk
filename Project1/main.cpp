#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

int main() {
    RenderWindow window(VideoMode::getDesktopMode(), "shtasgm", Style::None);
    window.setMouseCursorVisible(false);
    window.setFramerateLimit(120);
    Clock clock;

    // textures and font
    Texture bg1Sprite("bg_1.png");
    Texture bg2Sprite("bg_2.png");
    Texture bg3Sprite("bg_3.png");
    Texture playerSprite("char_spritesheet.png");
    Texture enemySprite("enemy_spritesheet.png");
    Texture heartSprite("heart_spritesheet.png");
    Font font("font.ttf");

    // sprites
    Sprite bg1(bg1Sprite), bg2(bg2Sprite), bg3(bg3Sprite);
    Sprite player(playerSprite), enemy(enemySprite), heart(heartSprite);

    // constants
    const int frameWidth = 56, frameHeight = 56;
    const float scale = 5.f, animationSpeed = 0.075f;
    const float playerMoveSpeed = 250.f, enemyMoveSpeed = 125.f;
    const float ground = 745.f;

    // animation, movement and cooldown
    Vector2f playerVelocity(0.f, 0.f), enemyVelocity(0.f, 0.f);
    float playerFrameTimer = 0.f, enemyFrameTimer = 0.f;
    float enemyAttackCooldown = 1.f, enemyAttackTimer = 0.f;
    float damageCooldown = 0.5f, playerDamageTimer = 2.f, enemyDamageTimer = 2.f;
    float enemyStaggerDuration = 1.f;

    // frame indices
    int playerTotalAttack = 5, playerAttackX = 0, playerAttackY = 2;
    int playerTotalCharge = 5, playerChargeX = 0, playerChargeY = 3;
    int playerTotalDeath = 3, playerDeathX = 0, playerDeathY = 6;
    int playerTotalIdle = 5, playerIdleX = 0, playerIdleY = 0;
    int playerTotalRun = 7, playerRunX = 0, playerRunY = 1;

    int enemyTotalAttack = 5, enemyAttackX = 0, enemyAttackY = 2;
    int enemyTotalCharge = 5, enemyChargeX = 0, enemyChargeY = 3;
    int enemyTotalDeath = 3, enemyDeathX = 0, enemyDeathY = 5;
    int enemyTotalIdle = 5, enemyIdleX = 0, enemyIdleY = 0;
    int enemyTotalRun = 7, enemyRunX = 0, enemyRunY = 1;

    // atates
    int playerAttacking = 0, playerCharging = 0, playerMoving = 0, playerDead = 0, playerHp = 6;
    int enemyAttacking = 0, enemyCharging = 0, enemyMoving = 0, enemyDead = 0, enemyHp = 2;

    // player setup
    player.setScale({ scale, scale });
    player.setOrigin({ frameWidth, 0 });
    player.setPosition({ 33 + scale * frameWidth, ground });

    // enemy setup
    enemy.setScale({ -scale, scale });
    enemy.setOrigin({ 0, 0 });
    enemy.setPosition({ 1607 + scale * frameWidth, ground });

    // UI setup
    Text text(font, "A or D : Move Left or Right\nLeft Click : Attack\nDelete : Respawn\nEscape : Exit Game", 30);
    text.setPosition({ 20.f, 10.f });
    text.setOutlineColor(Color::Black);
    text.setOutlineThickness(3);
    heart.setPosition({ 1742.f, 10.f });

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (optional event = window.pollEvent()) {
            if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
                window.close();
            }
        }

        // reset movement and update timers
        playerVelocity.x = 0.f;
        enemyVelocity.x = 0.f;
        playerMoving = enemyMoving = 0;
        enemyAttackTimer += dt;
        playerDamageTimer += dt;
        enemyDamageTimer += dt;

        // hearts and damage flashing
        heart.setTextureRect(IntRect({ 0, (6 - playerHp) * frameWidth }, { 3 * frameWidth, frameHeight }));
        player.setColor(playerDamageTimer < damageCooldown ? Color(255, 0, 0) : Color(255, 255, 255));
        enemy.setColor(enemyDamageTimer < damageCooldown ? Color(255, 0, 0) : Color(255, 255, 255));

        // handle input
        if (!playerHp) {
            if (Keyboard::isKeyPressed(Keyboard::Key::Delete)) {
                playerHp = 6;
                player.setPosition({ 33 + scale * frameWidth, ground });
                playerDead = 0;
            }
        }
        else if (playerHp && !playerAttacking && !playerCharging) {
            if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
                playerMoving = 1;
                playerVelocity.x += playerMoveSpeed;
                player.setScale({ scale, scale });
                player.setOrigin({ frameWidth, 0 });
            }
            if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
                playerMoving = 1;
                playerVelocity.x -= playerMoveSpeed;
                player.setScale({ -scale, scale });
                player.setOrigin({ 0, 0 });
            }
            if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                playerAttacking = 1;
                playerAttackX = 0;
            }
            else if (Mouse::isButtonPressed(Mouse::Button::Right)) {
                playerCharging = 1;
                playerChargeX = 0;
            }
        }

        // enemy AI
        if (!enemyDead) {
            float distance = player.getPosition().x - enemy.getPosition().x;
            if (distance >= frameWidth) {
                enemyMoving = 1;
                enemyVelocity.x += enemyMoveSpeed;
                enemy.setScale({ scale, scale });
                enemy.setOrigin({ frameWidth, 0 });
            }
            else if (distance <= -frameWidth) {
                enemyMoving = 1;
                enemyVelocity.x -= enemyMoveSpeed;
                enemy.setScale({ -scale, scale });
                enemy.setOrigin({ 0, 0 });
            }
            else if (!playerDead && enemyAttackTimer >= enemyAttackCooldown && enemyDamageTimer >= enemyStaggerDuration) {
                enemyAttacking = 1;
                enemyAttackTimer = 0.f;
                enemyVelocity.x = 0.f;
            }
        }

        player.move(playerVelocity * dt);
        enemy.move(enemyVelocity * dt);

        // animate player
        if (playerAttacking) {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                if (++playerAttackX == 4 && enemyHp) {
                    enemyHp--;
                    enemyDamageTimer = 0.f;
                }
                if (playerAttackX >= playerTotalAttack) playerAttacking = 0;
                player.setTextureRect(IntRect({ playerAttackX * frameWidth, playerAttackY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (playerCharging) {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                if (++playerChargeX >= playerTotalCharge) playerCharging = 0;
                player.setTextureRect(IntRect({ playerChargeX * frameWidth, playerChargeY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (!playerHp && !playerDead) {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                if (++playerDeathX >= playerTotalDeath) playerDead = 1;
                player.setTextureRect(IntRect({ playerDeathX * frameWidth, playerDeathY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (playerDead) {
            player.setTextureRect(IntRect({ frameWidth * 3, frameHeight * 6 }, { frameWidth, frameHeight }));
        }
        else if (playerMoving) {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                playerRunX = (playerRunX + 1) % playerTotalRun;
                player.setTextureRect(IntRect({ playerRunX * frameWidth, playerRunY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                playerIdleX = (playerIdleX + 1) % playerTotalIdle;
                player.setTextureRect(IntRect({ playerIdleX * frameWidth, playerIdleY * frameHeight }, { frameWidth, frameHeight }));
            }
        }

        // animate enemy
        if (enemyAttacking) {
            enemyFrameTimer += dt;
            if (enemyFrameTimer >= animationSpeed) {
                enemyFrameTimer = 0.f;
                if (++enemyAttackX == 4 && playerHp) {
                    playerHp--;
                    playerDamageTimer = 0.f;
                }
                if (enemyAttackX >= enemyTotalAttack) enemyAttacking = 0;
                enemy.setTextureRect(IntRect({ enemyAttackX * frameWidth, enemyAttackY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (!enemyHp && !enemyDead) {
            enemyFrameTimer += dt;
            if (enemyFrameTimer >= animationSpeed) {
                enemyFrameTimer = 0.f;
                if (++enemyDeathX >= enemyTotalDeath) enemyDead = 1;
                enemy.setTextureRect(IntRect({ enemyDeathX * frameWidth, enemyDeathY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (enemyDead) {
            enemy.setTextureRect(IntRect({ frameWidth * 3, frameHeight * 5 }, { frameWidth, frameHeight }));
        }
        else if (enemyMoving) {
            enemyFrameTimer += dt;
            if (enemyFrameTimer >= animationSpeed) {
                enemyFrameTimer = 0.f;
                enemyRunX = (enemyRunX + 1) % enemyTotalRun;
                enemy.setTextureRect(IntRect({ enemyRunX * frameWidth, enemyRunY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else {
            enemyFrameTimer += dt;
            if (enemyFrameTimer >= animationSpeed) {
                enemyFrameTimer = 0.f;
                enemyIdleX = (enemyIdleX + 1) % enemyTotalIdle;
                enemy.setTextureRect(IntRect({ enemyIdleX * frameWidth, enemyIdleY * frameHeight }, { frameWidth, frameHeight }));
            }
        }

        // render
        window.clear();
        window.draw(bg3);
        window.draw(bg2);
        window.draw(bg1);
        window.draw(text);
        window.draw(player);
        window.draw(enemy);
        window.draw(heart);
        window.display();
    }
    return 0;
}

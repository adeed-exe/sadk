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

    // resources
    Texture bg1Sprite("Assets/Sprites/bg_1.png");
    Texture bg2Sprite("Assets/Sprites/bg_2.png");
    Texture bg3Sprite("Assets/Sprites/bg_3.png");
    Texture playerSprite("Assets/Sprites/char_spritesheet.png");
    Texture enemySprite("Assets/Sprites/enemy_spritesheet.png");
    Texture heartSprite("Assets/Sprites/heart_spritesheet.png");
    Font font("Assets/Fonts/font.ttf");
    SoundBuffer runBuffer("Assets/Audio/run.wav");
    SoundBuffer attackBuffer("Assets/Audio/attack.wav");
    SoundBuffer ostBuffer("Assets/Audio/ost.mp3");
    SoundBuffer victoryBuffer("Assets/Audio/victory.wav");
    SoundBuffer defeatBuffer("Assets/Audio/defeat.wav");

    // sprites
    Sprite bg1(bg1Sprite), bg2(bg2Sprite), bg3(bg3Sprite);
    Sprite player(playerSprite), enemy(enemySprite), heart(heartSprite);

    // sounds
    Sound playerRun(runBuffer);
    Sound enemyRun(runBuffer);
    playerRun.setVolume(60.f);
    Sound playerAttack(attackBuffer);
    playerAttack.setVolume(120);
    Sound ost(ostBuffer);
    ost.setLooping(true);
    ost.setVolume(30.f);
    ost.play();
    Sound victory(victoryBuffer);
    victory.setVolume(50.f);
    Sound defeat(defeatBuffer);
    defeat.setVolume(30.f);

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
    int playerTotalDeath = 3, playerDeathX = 0, playerDeathY = 6;
    int playerTotalIdle = 5, playerIdleX = 0, playerIdleY = 0;
    int playerTotalRun = 7, playerRunX = 0, playerRunY = 1;

    int enemyTotalAttack = 5, enemyAttackX = 0, enemyAttackY = 2;
    int enemyTotalDeath = 3, enemyDeathX = 0, enemyDeathY = 5;
    int enemyTotalIdle = 5, enemyIdleX = 0, enemyIdleY = 0;
    int enemyTotalRun = 7, enemyRunX = 0, enemyRunY = 1;

    // states
    int playerAttacking = 0, playerRunning = 0, playerDead = 0, playerHp = 6;
    int enemyAttacking = 0, enemyRunning = 0, enemyDead = 0, enemyHp = 2;
    int startScreen = 1, gamePlaying = 0, endScreen = 0;

    // character setup
    player.setPosition({ window.getSize().x / 2.f - 300, ground });
    enemy.setPosition({ window.getSize().x / 2.f + 300, ground });
    player.setScale({ scale, scale });
    enemy.setScale({ -scale, scale });

    // UI setup
    Text begin(font, "Press Space to Play!", 40);
    begin.setOrigin(begin.getLocalBounds().getCenter());
    begin.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f });
    begin.setOutlineColor(Color::Black);
    begin.setOutlineThickness(4);
    Text won(font, "You Won!", 50);
    won.setOrigin(won.getLocalBounds().getCenter());
    won.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f - 30.f});
    won.setOutlineColor(Color::Black);
    won.setOutlineThickness(4);
    Text won2(font, "Press Enter to Play Again or Escape to Exit", 30);
    won2.setOrigin(won2.getLocalBounds().getCenter());
    won2.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f + 30.f });
    won2.setOutlineColor(Color::Black);
    won2.setOutlineThickness(4);
    Text died(font, "You Died!", 50);
    died.setOrigin(died.getLocalBounds().getCenter());
    died.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f - 30.f });
    died.setOutlineColor(Color::Black);
    died.setOutlineThickness(4);
    Text died2(font, "Press Enter to Respawn or Escape to Exit", 30);
    died2.setOrigin(died2.getLocalBounds().getCenter());
    died2.setPosition({ window.getSize().x / 2.f, window.getSize().y / 2.f + 30.f });
    died2.setOutlineColor(Color::Black);
    died2.setOutlineThickness(4);
    Text instructions(font, "A or D : Move Left or Right\nLeft Click : Attack", 30);
    instructions.setPosition({ 20.f, 10.f });
    instructions.setOutlineColor(Color::Black);
    instructions.setOutlineThickness(3);
    heart.setOrigin({ frameWidth * 1.5f, frameHeight / 2.f });
    heart.setScale({ 0.65f, 0.65f });

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (optional event = window.pollEvent()) {
            if (Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
                window.close();
            }
        }

        // move player and enemy to the playing position
        if (!(startScreen || endScreen) && !gamePlaying) {
            player.setPosition({ 33.f + scale * frameWidth, ground });
            player.setOrigin({ frameWidth, 0 });
            enemy.setPosition({ window.getSize().x - 33.f, ground});
            enemy.setOrigin({ 0, 0 });
            heart.setPosition({ player.getPosition().x - 144.f, player.getPosition().y + 113.f });
            gamePlaying = 1;
        }

        // reset movement and update timers
        playerVelocity.x = 0.f;
        enemyVelocity.x = 0.f;
        playerRunning = enemyRunning = 0;
        enemyAttackTimer += dt;
        playerDamageTimer += dt;
        enemyDamageTimer += dt;
        float distance = player.getPosition().x - enemy.getPosition().x;

        // hearts and damage flashing
        heart.setTextureRect(IntRect({ 0, (6 - playerHp) * frameWidth }, { 3 * frameWidth, frameHeight }));
        player.setColor(playerDamageTimer < damageCooldown ? Color(255, 0, 0, 128) : Color(255, 255, 255));
        heart.setColor(playerDamageTimer < damageCooldown * 0.2f ? Color(255, 255, 255, 0) : Color(255, 255, 255));
        enemy.setColor(enemyDamageTimer < damageCooldown ? Color(255, 0, 0, 128) : Color(255, 255, 255));

        // handle input
        if (startScreen) {
            if (Keyboard::isKeyPressed(Keyboard::Key::Space)) {
                startScreen = 0;
            }
        }
        else if (endScreen) {
            ost.stop();
            if (Keyboard::isKeyPressed(Keyboard::Key::Enter)) {
                playerHp = 6;
                enemyHp = 2;
                playerDead = 0;
                enemyDead = 0;
                gamePlaying = 0;
                endScreen = 0;
                ost.play();
            }
        }
        else if (gamePlaying && playerHp && !playerAttacking) {
            if (Keyboard::isKeyPressed(Keyboard::Key::D)) {
                playerRunning = 1;
                playerVelocity.x += playerMoveSpeed;
                player.setScale({ scale, scale });
                player.setOrigin({ frameWidth, 0 });
            }
            if (Keyboard::isKeyPressed(Keyboard::Key::A)) {
                playerRunning = 1;
                playerVelocity.x -= playerMoveSpeed;
                player.setScale({ -scale, scale });
                player.setOrigin({ 0, 0 });
            }
            if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                playerAttacking = 1;
                playerAttackX = 0;
            }
        }

        // enemy AI
        if (gamePlaying) {
            if (!enemyDead) {
                if (distance >= frameWidth) {
                    enemyRunning = 1;
                    enemyVelocity.x += enemyMoveSpeed;
                    enemy.setScale({ scale, scale });
                    enemy.setOrigin({ frameWidth, 0 });
                }
                else if (distance <= -frameWidth) {
                    enemyRunning = 1;
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
        }

        player.move(playerVelocity * dt);
        enemy.move(enemyVelocity * dt);
        enemyRun.setVolume(clamp(60.f - std::abs(distance) / 800 * 60.f, 0.f, 60.f));
        float heartOffsetX = player.getScale().x > 0 ? 144.f : 134.f;
        float heartOffsetY = -113.f;
        heart.setPosition({ player.getPosition().x - heartOffsetX, player.getPosition().y - heartOffsetY });

        // animate player
        if (playerAttacking) {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                if (playerAttackX == 0) playerAttack.play();
                if (++playerAttackX == 4 && enemyHp && fabs(distance) <= frameWidth) {
                    enemyHp--;
                    enemyDamageTimer = 0.f;
                }
                if (playerAttackX >= playerTotalAttack) playerAttacking = 0, playerAttackX = 0;
                player.setTextureRect(IntRect({ playerAttackX * frameWidth, playerAttackY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (!playerHp && !playerDead) {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                if (++playerDeathX >= playerTotalDeath) {
                    playerDead = 1;
                    endScreen = 1;
                    defeat.play();
                }
                player.setTextureRect(IntRect({ playerDeathX * frameWidth, playerDeathY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (playerDead) {
            player.setTextureRect(IntRect({ frameWidth * 3, frameHeight * 6 }, { frameWidth, frameHeight }));
        }
        else if (playerRunning) {
            playerFrameTimer += dt;
            if (playerFrameTimer >= animationSpeed) {
                playerFrameTimer = 0.f;
                playerRunX = (playerRunX + 1) % playerTotalRun;
                if (playerRunX == 0 || playerRunX == 4) {
                    playerRun.play();
                }
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
                if (enemyAttackX == 0) playerAttack.play();
                if (++enemyAttackX == 4 && playerHp && fabs(distance) <= frameWidth) {
                    playerHp--;
                    playerDamageTimer = 0.f;
                }
                if (enemyAttackX >= enemyTotalAttack) enemyAttacking = 0, enemyAttackX = 0;
                enemy.setTextureRect(IntRect({ enemyAttackX * frameWidth, enemyAttackY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (!enemyHp && !enemyDead) {
            enemyFrameTimer += dt;
            if (enemyFrameTimer >= animationSpeed) {
                enemyFrameTimer = 0.f;
                if (++enemyDeathX >= enemyTotalDeath) {
                    enemyDead = 1;
                    endScreen = 1;
                    victory.play();
                }
                enemy.setTextureRect(IntRect({ enemyDeathX * frameWidth, enemyDeathY * frameHeight }, { frameWidth, frameHeight }));
            }
        }
        else if (enemyDead) {
            enemy.setTextureRect(IntRect({ frameWidth * 3, frameHeight * 5 }, { frameWidth, frameHeight }));
        }
        else if (enemyRunning) {
            enemyFrameTimer += dt;
            if (enemyFrameTimer >= animationSpeed) {
                enemyFrameTimer = 0.f;
                enemyRunX = (enemyRunX + 1) % enemyTotalRun;
                if (enemyRunX == 0 || enemyRunX == 4) {
                    enemyRun.play();
                }
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
        window.draw(player);
        window.draw(enemy);
        if (startScreen) {
            window.draw(begin);
        }
        else if (endScreen) {
            if (!enemyHp) {
                window.draw(won);
                window.draw(won2);
            }
            else {
                window.draw(died);
                window.draw(died2);
            }
        }
        else {
            window.draw(instructions);
            window.draw(heart);
        }
        window.display();
    }
    return 0;
}

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>  // file handling (high score)
#include <vector>
#include <string>
#include <cstdlib>  


using namespace std;
using namespace sf;

const int tileSize = 25;
const float pacmanSpeed = 100.0f; 
const float ghostSpeed = 80.0f;    
const float powerUpDuration = 8.0f; 
bool powerUpActive = false;    
Clock powerUpTimer;            
bool isValidMove(const vector<string>& map, const Vector2f& position);

// Function to load the high score from file
int loadHighScore() {
    int highScore = 0;  // Initialize highScore to 0
    ifstream inputFile("highscore.txt");  
    if (inputFile.is_open()) {
        inputFile >> highScore;  
        inputFile.close();       
    }
    else {
        cout << "Unable to open highscore.txt. Initializing high score to 0." << endl;
    }
    return highScore;  
}

void saveHighScore(int highScore) {
    ofstream outputFile("highscore.txt");  
    if (outputFile.is_open()) {
        outputFile << highScore;  
        outputFile.close();  
        cout << "High score saved: " << highScore << endl;  
    }
    else {
        cout << "Error: Unable to save high score!" << endl;
    }
}

struct Ghost {
    Sprite ghostSprite;
    Vector2f position;
    Vector2f velocity;
    Clock moveClock;  
    float moveInterval;  
    float chaseDistance;  

    Ghost(const Vector2f& startPosition, const Texture& texture, float interval = 1.0f, float chaseDist = 100.0f)
        : moveInterval(interval), chaseDistance(chaseDist) {
        ghostSprite.setTexture(texture);
        ghostSprite.setScale(.045f, .045f);
        ghostSprite.setPosition(startPosition);
        position = startPosition;
        velocity = Vector2f(0.f, 0.f);
    }

    void runAwayFromPacMan(const Vector2f& pacManPosition, const vector<string>& map) {

        Vector2f possibleVelocities[4] = {
            Vector2f(-ghostSpeed, 0),  // Left
            Vector2f(ghostSpeed, 0),   // Right
            Vector2f(0, -ghostSpeed),  // Up
            Vector2f(0, ghostSpeed)    // Down
        };

        Vector2f bestVelocity = velocity;
        float maxDistance = distanceTo(pacManPosition, position);

        for (const auto& vel : possibleVelocities) {
            Vector2f newPos = position + vel;
            if (isValidMove(map, newPos)) {
                float dist = distanceTo(pacManPosition, newPos);
                if (dist > maxDistance) {
                    maxDistance = dist;
                    bestVelocity = vel;
                }
            }
        }

        velocity = bestVelocity; // Choose the direction that moves away from Pac-Man
    }

    void update(float dtSeconds, const vector<string>& map, const Vector2f& pacManPosition, bool powerUpActive, const Texture& blueGhostTexture, const Texture& normalGhostTexture) {
        if (powerUpActive) {
            ghostSprite.setTexture(blueGhostTexture); // Change ghost texture to blue ghost
            ghostSprite.setScale(.087f, .087f); // size of the blue ghost
            runAwayFromPacMan(pacManPosition, map);
        }
        else {
            ghostSprite.setTexture(normalGhostTexture);  // Reset to normal ghost texture
            ghostSprite.setScale(.045f, .045f); // Reset to original ghost size

            float distanceToPacMan = sqrt(pow(position.x - pacManPosition.x, 2) + pow(position.y - pacManPosition.y, 2));
            if (distanceToPacMan < chaseDistance) {
                chasePacMan(pacManPosition, map);
            }
            else {
                if (moveClock.getElapsedTime().asSeconds() >= moveInterval) {
                    moveRandomly(map);
                    moveClock.restart();
                }
            }
        }

        Vector2f newPosition = position + velocity * dtSeconds;

        if (isValidMove(map, newPosition)) {
            position = newPosition;
        }
        else {
            moveRandomly(map);
        }

        ghostSprite.setPosition(position);
    }



    void draw(RenderWindow& window) {
        window.draw(ghostSprite);
    }

    void moveRandomly(const vector<string>& map) {
        Vector2f possibleDirections[4] = {
            Vector2f(-ghostSpeed, 0),  // Left
            Vector2f(ghostSpeed, 0),   // Right
            Vector2f(0, -ghostSpeed),  // Up
            Vector2f(0, ghostSpeed)    // Down
        };

        vector<Vector2f> validDirections;
        for (const auto& dir : possibleDirections) {
            Vector2f newPos = position + dir;
            if (isValidMove(map, newPos)) {
                validDirections.push_back(dir);
            }
        }

        if (!validDirections.empty()) {
            velocity = validDirections[rand() % validDirections.size()];
        }
        else {
            velocity = -velocity;
        }
    }

    void chasePacMan(const Vector2f& pacManPosition, const vector<string>& map) {


        Vector2f possibleVelocities[4] = {
            Vector2f(-ghostSpeed, 0),  // Left
            Vector2f(ghostSpeed, 0),   // Right
            Vector2f(0, -ghostSpeed),  // Up
            Vector2f(0, ghostSpeed)    // Down
        };

        Vector2f bestVelocity = velocity;
        float minDistance = distanceTo(pacManPosition, position);

        for (const auto& vel : possibleVelocities) {
            Vector2f newPos = position + vel;
            if (isValidMove(map, newPos)) {
                float dist = distanceTo(pacManPosition, newPos);
                if (dist < minDistance) {
                    minDistance = dist;
                    bestVelocity = vel;
                }
            }
        }

        velocity = bestVelocity;
    }

    float distanceTo(const Vector2f& a, const Vector2f& b) const {
        return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }
};

struct PacMan {
    Texture pacmanTexture;
    Sprite pacmanSprite;
    Vector2f position;
    Vector2f velocity;
    int direction;

    PacMan(const Vector2f& startPosition, int pacmanDirection) {
        if (!pacmanTexture.loadFromFile("pacmanRight.png")) {  //right will be the default direction texture
            cerr << "Error loading default Pac-Man texture" << endl;
        }
        pacmanSprite.setTexture(pacmanTexture);
        pacmanSprite.setScale(.0178f, .0178f);
        pacmanSprite.setPosition(startPosition);
        position = startPosition;
        velocity = Vector2f(0.f, 0.f);
        direction = pacmanDirection;
    }

    void update(float dtSeconds, const vector<string>& map) {
        Vector2f nextPosition = position + (velocity * dtSeconds);
        if (isValidMove(map, nextPosition)) {
            position = nextPosition;
            pacmanSprite.setPosition(position);
        }
        else {
            velocity = { 0.f, 0.f };
        }
    }

    void draw(RenderWindow& window) {
        window.draw(pacmanSprite);
    }
};



bool isValidMove(const vector<string>& map, const Vector2f& position) {
    float tolerance = 2.0f;
    Vector2f corners[4] = {
        position + Vector2f(tolerance, tolerance),
        position + Vector2f(tileSize - tolerance, tolerance),
        position + Vector2f(tolerance, tileSize - tolerance),
        position + Vector2f(tileSize - tolerance, tileSize - tolerance)
    };

    for (int i = 0; i < 4; ++i) {
        int x = static_cast<int>(corners[i].x / tileSize);
        int y = static_cast<int>(corners[i].y / tileSize);

        if (y < 0 || y >= map.size() || x < 0 || x >= map[0].size()) {
            return false;
        }

        if (map[y][x] == '#') {
            return false;
        }
    }
    return true;
}

void handleKeyPress(PacMan& pacman, const Event::KeyEvent& keyEvent, int& pacmanDirection) {
    pacman.position.x = round(pacman.position.x / tileSize) * tileSize;
    pacman.position.y = round(pacman.position.y / tileSize) * tileSize;

    switch (keyEvent.code) {
    case Keyboard::Up:
        pacman.velocity = { 0, -pacmanSpeed };
        pacmanDirection = 1;
        if (!pacman.pacmanTexture.loadFromFile("pacmanUp.png")) {
            cerr << "Error loading Pac-Man up texture" << endl;
        }
        break;
    case Keyboard::Down:
        pacman.velocity = { 0, pacmanSpeed };
        pacmanDirection = 2;
        if (!pacman.pacmanTexture.loadFromFile("pacmanDown.png")) {
            cerr << "Error loading Pac-Man down texture" << endl;
        }
        break;
    case Keyboard::Left:
        pacman.velocity = { -pacmanSpeed, 0 };
        pacmanDirection = 3;
        if (!pacman.pacmanTexture.loadFromFile("pacmanLeft.png")) {
            cerr << "Error loading Pac-Man left texture" << endl;
        }
        break;
    case Keyboard::Right:
        pacman.velocity = { pacmanSpeed, 0 };
        pacmanDirection = 4;
        if (!pacman.pacmanTexture.loadFromFile("pacmanRight.png")) {
            cerr << "Error loading Pac-Man right texture" << endl;
        }
        break;
    default:
        break;
    }

    pacman.pacmanSprite.setTexture(pacman.pacmanTexture);
}


void drawMap(RenderWindow& window, const vector<string>& map, const Texture& pelletTexture, const Texture& powerPelletTexture, PacMan& pacman) {
    RectangleShape wall(Vector2f(tileSize, tileSize));
    wall.setFillColor(Color::Blue);

    Sprite pelletSprite(pelletTexture);
    pelletSprite.setScale(0.01, 0.01);
    Sprite powerPelletSprite(powerPelletTexture);
    powerPelletSprite.setScale(0.02, 0.02);

    for (size_t y = 0; y < map.size(); ++y) {
        for (size_t x = 0; x < map[y].size(); ++x) {
            char ch = map[y][x];
            Vector2f position(x * tileSize, y * tileSize);

            switch (ch) {
            case '#':
                wall.setPosition(position);
                window.draw(wall);
                break;
            case '.':
                pelletSprite.setPosition(position + Vector2f((tileSize / 2), tileSize / 2));
                window.draw(pelletSprite);
                break;
            case 'o':
                powerPelletSprite.setPosition(position + Vector2f(((tileSize / 4) + 1.5), ((tileSize + 1) / 4) + 1.3));
                window.draw(powerPelletSprite);
                break;
            case 'P':
                pacman.draw(window);
                break;
            default:
                break;
            }
        }
    }
}

void updateMap(vector<string>& map, const Vector2f& position, int& score, bool& powerUpActive, Clock& powerUpTimer) {
    int x = static_cast<int>((position.x + tileSize / 2) / tileSize);
    int y = static_cast<int>((position.y + tileSize / 2) / tileSize);

    if (y < 0 || y >= map.size() || x < 0 || x >= map[0].size()) {
        return;
    }

    if (map[y][x] == '.') {
        map[y][x] = ' ';
        score++;
    }
    else if (map[y][x] == 'o') {
        map[y][x] = ' ';
        score += 10;
        powerUpActive = true;
        powerUpTimer.restart();
    }
}

bool arePelletsLeft(const vector<string>& map) {
    for (const string& row : map) {
        for (char tile : row) {
            if (tile == '.' || tile == 'o') {
                return true;
            }
        }
    }
    return false;
}

void showGameOverScreen(RenderWindow& window, int score, int highScore) {
    RenderWindow gameOverWindow(VideoMode(300, 200), "Game Over");

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error loading font file" << endl;
        return;
    }

    Text gameOverText("Game Over!", font, 24);
    Text scoreText("Score: " + to_string(score), font, 24);
    Text highScoreText("High Score: " + to_string(highScore), font, 24);

    FloatRect gameOverBounds = gameOverText.getLocalBounds();
    gameOverText.setOrigin(gameOverBounds.width / 2, gameOverBounds.height / 2);
    gameOverText.setPosition(gameOverWindow.getSize().x / 2, gameOverWindow.getSize().y / 3);

    FloatRect scoreBounds = scoreText.getLocalBounds();
    scoreText.setOrigin(scoreBounds.width / 2, scoreBounds.height / 2);
    scoreText.setPosition(gameOverWindow.getSize().x / 2, gameOverWindow.getSize().y / 2);

    FloatRect highScoreBounds = highScoreText.getLocalBounds();
    highScoreText.setOrigin(highScoreBounds.width / 2, highScoreBounds.height / 2);
    highScoreText.setPosition(gameOverWindow.getSize().x / 2, gameOverWindow.getSize().y / 1.5);

    while (gameOverWindow.isOpen()) {
        Event event;
        while (gameOverWindow.pollEvent(event)) {
            if (event.type == Event::Closed)
                gameOverWindow.close();
        }

        gameOverWindow.clear();
        gameOverWindow.draw(gameOverText);
        gameOverWindow.draw(scoreText);
        gameOverWindow.draw(highScoreText);
        gameOverWindow.display();
    }
}

void checkCollisionWithGhosts(PacMan& pacman, vector<Ghost>& ghosts, bool powerUpActive, int& score, RenderWindow& window, const vector<string>& map, int& highScore) {
    for (auto& ghost : ghosts) {
        float distance = ghost.distanceTo(pacman.position, ghost.position);
        if ((distance < tileSize / 1.2f)) {
            if (powerUpActive) {
                ghost.position = Vector2f(tileSize * 9, tileSize * 7);  // Reset ghost to the box

                score += 50;  // Award points for eating a ghost
            }
            else {
                SoundBuffer death;
                if (!death.loadFromFile("pacman_death.wav")) {
                    return;
                }
                Sound deathMusic;
                deathMusic.setBuffer(death);
                deathMusic.play();

                // Update high score
                if (score > highScore) {
                    highScore = score;  // Update the in-game high score
                    saveHighScore(highScore);  // Save the new high score to the file
                }

                showGameOverScreen(window, score, highScore);  // Show Game Over with high score
                window.close();
            }
        }
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    SoundBuffer intro, chomp;
    if (!intro.loadFromFile("pacman_beginning.wav") ||
        !chomp.loadFromFile("pacman_chomp.wav")) {
        cout << "Error loading sound" << endl;
        return 1;
    }

    Sound introMusic, chompMusic;
    introMusic.setBuffer(intro);
    chompMusic.setBuffer(chomp);
    introMusic.play();

    int highScore = loadHighScore();  // Load the high score at the start

    vector<string> pacmanMap = {
        " ################### ",
        " #........#..o.....# ",
        " #o##.###.#.###.##o# ",
        " #.................# ",
        " #.##.#.#####.#.##.# ",
        " #....#...#...#....# ",
        " ####.### # ###.#### ",
        "    #.    0    .#    ",  // Map
        "#####.# ##o##.#.#####",
        "#o.  .. #   #  ..o. #",
        "#####.#.#####.#.#####",
        "    # #.......#.#    ",
        " #### # #.#.# #.#### ",
        " #........#........# ",
        " #.##.###.#.###.##.# ",
        " #o.#.....P.....#.o# ",
        " ##.#.#.#####.#.#.## ",
        " #.................# ",
        " ################### ",
    };

    RenderWindow window(VideoMode(tileSize * pacmanMap[0].size(), tileSize * pacmanMap.size() + 50), "Pac-Man");

    Texture pelletTexture, powerPelletTexture, ghostTexture, blueGhostTexture;
    if (!pelletTexture.loadFromFile("pallet.png") ||
        !powerPelletTexture.loadFromFile("pallet.png") ||
        !ghostTexture.loadFromFile("ghost.png") ||
        !blueGhostTexture.loadFromFile("pngimg.com - pacman_PNG35.png")) {
        cerr << "Error loading texture" << endl;
        return 1;
    }

    Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error loading font" << endl;
        return 1;
    }

    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);  // To set text size
    scoreText.setFillColor(Color::White);  // To set text color
    scoreText.setPosition(10.f, tileSize * pacmanMap.size() + 10);  // To position score at bottom of window

    int pacmanDirection = 1;
    PacMan pacman(Vector2f(tileSize * 9, tileSize * 15), pacmanDirection);  // Start Pac-Man at his designated position
    vector<Ghost> ghosts = {
        Ghost(Vector2f(tileSize * 9, tileSize * 7), ghostTexture, 1.0f, 150.0f),
        Ghost(Vector2f(tileSize * 10, tileSize * 7), ghostTexture, 1.0f, 120.0f),
        Ghost(Vector2f(tileSize * 11, tileSize * 7), ghostTexture, 1.0f, 100.0f),
        Ghost(Vector2f(tileSize * 12, tileSize * 7), ghostTexture, 1.0f, 130.0f)
    };

    Clock clock;
    int score = 0;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        Event event;
        while (window.pollEvent(event)) {
            chompMusic.play();
            if (event.type == Event::Closed)
                window.close();
            else if (event.type == Event::KeyPressed)
                handleKeyPress(pacman, event.key, pacmanDirection);
        }

        pacman.update(dt, pacmanMap);

        Vector2f pacManPosition = pacman.position;
        updateMap(pacmanMap, pacman.position, score, powerUpActive, powerUpTimer);

        if (powerUpActive && powerUpTimer.getElapsedTime().asSeconds() > powerUpDuration) {
            powerUpActive = false;
        }

        for (int i = 0; i < ghosts.size(); ++i) {
            ghosts[i].update(dt, pacmanMap, pacManPosition, powerUpActive, blueGhostTexture, ghostTexture);
        }

        checkCollisionWithGhosts(pacman, ghosts, powerUpActive, score, window, pacmanMap, highScore);

        // Update the score text
        scoreText.setString("Score: " + to_string(score - 1));

        window.clear();
        drawMap(window, pacmanMap, pelletTexture, powerPelletTexture, pacman);
        for (auto& ghost : ghosts) {
            ghost.draw(window);
        }
        window.draw(scoreText);  // Draw the score
        window.display();

        if (!arePelletsLeft(pacmanMap)) {
            if (score > highScore) {
                highScore = score;
                saveHighScore(highScore);
            }
            showGameOverScreen(window, score, highScore);
            window.close();
        }
    }
    return 0;
}

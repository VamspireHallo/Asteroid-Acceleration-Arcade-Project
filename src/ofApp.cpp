#include "ofApp.h" 

//--------------------------------------------------------------
void ofApp::setup() {
    // Loading background image
    background.load("BG/stars.png");

    // Loading fonts, different sizes for title and normal text
    titleFont.load("FONT/VideoPhreak.ttf", 50);
    textFont.load("FONT/VideoPhreak.ttf", 25);

    // Loading sound effects for movement and shooting
    // Looping enabled for consistent play
    thrustSound.load("SFX/thrust.wav");
    thrustSound.setMultiPlay(true); 
    thrustSound.setLoop(true);

    whirlSound.load("SFX/whirl.wav");
    whirlSound.setMultiPlay(true);
    whirlSound.setLoop(true);

    shootSound.load("SFX/shoot.wav");
    shootSound.setMultiPlay(true);
    shootSound.setLoop(true);

    // Loading collision sound effects
    playerHitSound.load("SFX/playerhit.wav");
    playerHitSound.setMultiPlay(true);

    asteroidHitSound.load("SFX/asteroidhit.wav");
    asteroidHitSound.setMultiPlay(true);
}

//--------------------------------------------------------------
void ofApp::update() {
    // Checks if the game is in the play state
    if (gameState != GAMEPLAY) { return; }

    // Count Down Timer 
    if (!timesUp) {
        // Decrease the timer by the elapsed time; 2 minutes to play
        timer -= ofGetLastFrameTime(); 

        // Timer Finished, Exit to the Game Over Screen
        if (timer <= 0) {
            timer = 0;
            timesUp = true;
            gameState = ENDSCREEN;
        }
    }

    // Handle player movement
    if (moveLeft) { player->rotate(-5.0f); }
    if (moveRight) { player->rotate(5.0f); }
    if (moveForward) { player->thrust(0.1f); }
    if (moveBackward) { player->thrust(-0.1f); }

    player->update();
    bulletSystem->update();

    // Handle shooting
    float currentTime = ofGetElapsedTimef();
    if (shooting && (currentTime - lastShotTime >= fireRate) && !player->invulnerable && !player->isExploding) {
        bulletSystem->emit(player->pos, player->rot, 8.0f, player->velocity);
        lastShotTime = currentTime;
        shootSound.play();
    }

    // Update large asteroids
    for (Asteroid& asteroid : asteroids) {
        asteroid.update();
        asteroid.updateExplosion();
    }

    // Update all small asteroids
    for (SmallAsteroid& smallAsteroid : smallAsteroids) {
        smallAsteroid.update();
        smallAsteroid.updateExplosion();
    }

    // Check for bullet collisions with large asteroids
    if (collisionSystem.BulletLargeAsteroidCollision(bulletSystem->particles, asteroids, smallAsteroids)) {
        score += 100;
        asteroidHitSound.play();
        asteroidsDestroyed++;
        spawnAsteroids();
    }

    // Check for bullet collisions with small asteroids
    if (collisionSystem.BulletSmallAsteroidCollision(bulletSystem->particles, smallAsteroids)) {
        score += 50;
        asteroidHitSound.play();
        asteroidsDestroyed++;
    }

    // Check player collision with large asteroids
    if (collisionSystem.PlayerLargeAsteroidCollision(*player, asteroids)) {
        score = std::max(0, score - 75);
        playerHitSound.play();
        playerDeaths++;
    }

    // Check player collision with small asteroids
    if (collisionSystem.PlayerSmallAsteroidCollision(*player, smallAsteroids)) {
        score = std::max(0, score - 75);
        playerHitSound.play();
        playerDeaths++;
    }

    // Stopping player sounds if it is hit
    if (player->isExploding) {
        if (thrustSound.isPlaying()) thrustSound.stop();
        if (whirlSound.isPlaying()) whirlSound.stop();
        if (shootSound.isPlaying()) shootSound.stop();
    }

    // Check collision between large asteroids
    collisionSystem.LargeAsteroidCollision(asteroids);

    // Check collision between large and small asteroids as well
    collisionSystem.LargeSmallAsteroidCollision(asteroids, smallAsteroids);

    // Check the game to see if at least 7 asteroids are in play
    if (asteroids.size() < 7 && smallAsteroids.size() <= 0) {
        spawnAsteroids();
    }

    // Remove large asteroids after explosions have finished
    asteroids.erase(
        std::remove_if(asteroids.begin(), asteroids.end(),
            [](const Asteroid& a) { return a.isExplosionFinished(); }),
        asteroids.end()
    );

    // Remove small asteroids after explosions have finished
    smallAsteroids.erase(
        std::remove_if(smallAsteroids.begin(), smallAsteroids.end(),
            [](const SmallAsteroid& a) { return a.isExplosionFinished(); }),
        smallAsteroids.end()
    );
}


//--------------------------------------------------------------
void ofApp::draw() {
    // Background Image
    background.draw(0, 0, ofGetWidth(), ofGetHeight());

    // Drawing Start Scree n
    if (gameState == START) {
        startScreen();
    }
    // Drawing Instructions Screen
    else if (gameState == INSTRUCTIONS) {
        instructionsScreen();
    }
    // Drawing Play Screen
    else if (gameState == GAMEPLAY) {
        player->draw();
        bulletSystem->draw();

        for (Asteroid& asteroid : asteroids) {
            asteroid.draw();
            asteroid.drawExplosion();
        }

        for (SmallAsteroid& smallAsteroid : smallAsteroids) {
            smallAsteroid.draw();
            smallAsteroid.drawExplosion();
        }

        // Setting up Score Counter Display
        ofSetColor(255);
        textFont.drawString("Score: " + ofToString(score), 5, 25);

        // Setting up Timer Display
        string timerText = "Time: " + ofToString(int(timer)); // Convert the float to an integer
        ofRectangle timerRect = textFont.getStringBoundingBox(timerText, 0, 0);
        float timerX = ofGetWidth() / 2 - timerRect.getWidth() / 2;
        textFont.drawString(timerText, timerX, 25);
    }
    // Drawing End Screen
    else if (gameState == ENDSCREEN) {
        endGame();
    }

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    // Enable gameplay movements and shooting
    if (gameState == GAMEPLAY) {
        if (key == OF_KEY_LEFT || key == 'a') {
            moveLeft = true;
            if (!whirlSound.isPlaying()) whirlSound.play();
        }
        if (key == OF_KEY_RIGHT || key == 'd') {
            moveRight = true;
            if (!whirlSound.isPlaying()) whirlSound.play();
        }
        if (key == OF_KEY_UP || key == 'w') {
            moveForward = true;
            if (!thrustSound.isPlaying()) thrustSound.play();
        }
        if (key == OF_KEY_DOWN || key == 's') {
            moveBackward = true;
            if (!thrustSound.isPlaying()) thrustSound.play();
        }
        if (key == ' ' && !player->invulnerable && !player->isExploding) {
            shooting = true;
            if (!shootSound.isPlaying()) shootSound.play();
        }
        // Player can exit gameplay by pressing 'r'
        if (key == 'r') {
            gameState = ENDSCREEN;
        }
    }
}


//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    // Disable gameplay movements and shooting
    if (gameState == GAMEPLAY) {
        if (key == OF_KEY_LEFT || key == 'a') {
            moveLeft = false;
            if (!moveRight) whirlSound.stop();  // Stop only if not still rotating
        }
        if (key == OF_KEY_RIGHT || key == 'd') {
            moveRight = false;
            if (!moveLeft) whirlSound.stop();
        }
        if (key == OF_KEY_UP || key == 'w') {
            moveForward = false;
            if (!moveBackward) thrustSound.stop();
        }
        if (key == OF_KEY_DOWN || key == 's') {
            moveBackward = false;
            if (!moveForward) thrustSound.stop();
        }
        if (key == ' ') {
            shooting = false;
            shootSound.stop();
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    if (gameState == START) {
        // Check hovering over "Start Game" button
        ofRectangle rect1 = textFont.getStringBoundingBox("START GAME", 0, 0);
        float x1 = ofGetWidth() / 2 - rect1.getWidth() / 2;
        float y1 = ofGetHeight() / 2;
        if (x >= x1 && x <= x1 + rect1.getWidth() && y >= y1 - rect1.getHeight() / 2 && y <= y1 + rect1.getHeight() / 2) {
            startGameHovered = true;
        }
        else {
            startGameHovered = false;
        }

        // Check hovering over "Instructions" button
        ofRectangle rect2 = textFont.getStringBoundingBox("INSTRUCTIONS", 0, 0);
        float x2 = ofGetWidth() / 2 - rect2.getWidth() / 2;
        float y2 = ofGetHeight() / 2 + 50;
        if (x >= x2 && x <= x2 + rect2.getWidth() && y >= y2 - rect2.getHeight() / 2 && y <= y2 + rect2.getHeight() / 2) {
            instructionsHovered = true;
        }
        else {
            instructionsHovered = false;
        }
    }

    // Checks both Instructions and End Screen
    if (gameState == INSTRUCTIONS || gameState == ENDSCREEN) {
        // Check hovering over "Exit Game" button
        ofRectangle exitRect = textFont.getStringBoundingBox("EXIT", 0, 0);
        float exitX = ofGetWidth() / 2 - exitRect.getWidth() / 2;
        float exitY = ofGetHeight() - 50;
        if (x >= exitX && x <= exitX + exitRect.getWidth() &&
            y >= exitY - exitRect.getHeight() && y <= exitY) {
            exitHovered = true;
        }
        else {
            exitHovered = false;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    if (gameState == START) {
        // Check if pressed "Start Game" button
        ofRectangle rect1 = textFont.getStringBoundingBox("START GAME", 0, 0);
        float x1 = ofGetWidth() / 2 - rect1.getWidth() / 2;
        float y1 = ofGetHeight() / 2;

        if (x >= x1 && x <= x1 + rect1.getWidth() && y >= y1 - rect1.getHeight() / 2 && y <= y1 + rect1.getHeight() / 2) {
            gameState = GAMEPLAY;
            startGame();  
        }

        // Check if pressed "Instructions" button
        ofRectangle rect2 = textFont.getStringBoundingBox("INSTRUCTIONS", 0, 0);
        float x2 = ofGetWidth() / 2 - rect2.getWidth() / 2;
        float y2 = ofGetHeight() / 2 + 50;
        if (x >= x2 && x <= x2 + rect2.getWidth() && y >= y2 - rect2.getHeight() / 2 && y <= y2 + rect2.getHeight() / 2) {
            gameState = INSTRUCTIONS;
        }
    }

    // Check if Exit Button was pressed in either Instructions or End Screen
    else if (gameState == INSTRUCTIONS || gameState == ENDSCREEN) {
        ofRectangle exitRect = textFont.getStringBoundingBox("EXIT", 0, 0);
        float exitX = ofGetWidth() / 2 - exitRect.getWidth() / 2;
        float exitY = ofGetHeight() - 50;

        if (x >= exitX && x <= exitX + exitRect.getWidth() &&
            y >= exitY - exitRect.getHeight() && y <= exitY) {
            gameState = START;
        }
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

// Asteroids Spawner to populate game while playing
void ofApp::spawnAsteroids() {
    // At least 7 asteroids must be in the game at any time
    int minAsteroids = 7;
    int numNewAsteroids = minAsteroids - asteroids.size();
    if (numNewAsteroids <= 0) {
        return;
    }

    // New asteroids added based on the number destroyed
    for (int i = 0; i < numNewAsteroids; i++) {
        glm::vec3 randomPos;
        float distanceToPlayer = 0;

        // Make sure asteroids spawn away from player
        do {
            randomPos = glm::vec3(ofRandomWidth(), ofRandomHeight(), 0);
            distanceToPlayer = glm::distance(randomPos, player->pos);
        } while (distanceToPlayer < minDistanceFromPlayer);

        int sizeFactor = 10 + (asteroidsDestroyed / 10);
        asteroids.push_back(Asteroid(randomPos, sizeFactor));
    }
}

void ofApp::startScreen() {
    // "ASTEROID ACCERATION" Title 
    ofSetColor(0, 255, 0);
    ofRectangle rect1 = titleFont.getStringBoundingBox("ASTEROID ACCELERATION", 0, 0);
    float x1 = ofGetWidth() / 2 - rect1.getWidth() / 2;
    float y1 = ofGetHeight() / 2 - 100;
    titleFont.drawString("ASTEROID ACCELERATION", x1, y1);

    // Start Game Button
    ofSetColor(255);
    ofRectangle rect2 = textFont.getStringBoundingBox("START GAME", 0, 0);
    float x2 = ofGetWidth() / 2 - rect2.getWidth() / 2;
    float y2 = ofGetHeight() / 2;

    // Change color if hovering over the "Start Game" button
    if (startGameHovered) {
        ofSetColor(0, 255, 0); // Green when hovered
    }
    else {
        ofSetColor(255); // White
    }
    textFont.drawString("START GAME", x2, y2);

    // Instructions Button
    ofRectangle rect3 = textFont.getStringBoundingBox("INSTRUCTIONS", 0, 0);
    float x3 = ofGetWidth() / 2 - rect3.getWidth() / 2;
    float y3 = ofGetHeight() / 2 + 50;

    // Change color if hovering over the "Instructions" button
    if (instructionsHovered) {
        ofSetColor(0, 255, 0); // Green when hovered
    }
    else {
        ofSetColor(255); // White
    }
    textFont.drawString("INSTRUCTIONS", x3, y3);

    ofSetColor(255);
}

void ofApp::instructionsScreen() {
    // Instructions Title
    ofSetColor(0, 255, 0);
    ofRectangle titleRect = titleFont.getStringBoundingBox("INSTRUCTIONS", 0, 0);
    float titleX = ofGetWidth() / 2 - titleRect.getWidth() / 2;
    float titleY = 100;
    titleFont.drawString("INSTRUCTIONS", titleX, titleY);

    // Game Details
    textFont.drawString("Left(A) / Right(D) Arrow Keys: Rotate Player", titleX - 100, ofGetHeight() / 2 - 100);
    textFont.drawString("Up(W) / Down(S) Arrow Keys: Move Player", titleX - 100, ofGetHeight() / 2 - 50);
    textFont.drawString("Spacebar: Shoot", titleX - 100, ofGetHeight() / 2 + 0);
    textFont.drawString("Objective: Shoot Asteroids To Score Points", titleX - 100, ofGetHeight() / 2 + 50);
    textFont.drawString("               Avoid Crashing & Losing Points", titleX - 100, ofGetHeight() / 2 + 100);
    textFont.drawString("               Max Your Score Before Time Runs Out!", titleX - 100, ofGetHeight() / 2 + 150);
    
    // Exit Button
    ofRectangle exitRect = textFont.getStringBoundingBox("EXIT", 0, 0);
    float exitX = ofGetWidth() / 2 - exitRect.getWidth() / 2;
    float exitY = ofGetHeight() - 50;

    if (exitHovered) {
        ofSetColor(0, 255, 0); // Green when hovered
    }
    else {
        ofSetColor(255); // White
    }
    textFont.drawString("EXIT", exitX, exitY);

    ofSetColor(255);
}

void ofApp::startGame() {
    // Delete old player and particle system if still active
    if (player) delete player;
    if (bulletSystem) delete bulletSystem;

    player = new Player();
    bulletSystem = new ParticleSystem();

    // Clear & reset all values for new game play
    asteroids.clear();
    smallAsteroids.clear();
    score = 0;
    playerDeaths = 0;
    asteroidsDestroyed = 0;
    timer = 120.0f;
    timesUp = false;

    // Spawn Asteroids
    for (int i = 0; i < 7; i++) {
        glm::vec3 randomPos;
        float distanceToPlayer = 0;
        // Make sure asteroids spawn away from player
        do {
            randomPos = glm::vec3(ofRandomWidth(), ofRandomHeight(), 0);
            distanceToPlayer = glm::distance(randomPos, player->pos);
        } while (distanceToPlayer < minDistanceFromPlayer);

        int randomSides = ofRandom(10, 20);
        asteroids.push_back(Asteroid(randomPos, randomSides));
    }
}

void ofApp::endGame() {
    // Clear all player actions, sounds, and flags
    player->reset();
    moveLeft = false;
    moveRight = false;
    moveForward = false;
    moveBackward = false;
    shooting = false;

    if (thrustSound.isPlaying()) thrustSound.stop();
    if (whirlSound.isPlaying()) whirlSound.stop();
    if (shootSound.isPlaying()) shootSound.stop();
    if (playerHitSound.isPlaying()) playerHitSound.stop();
    if (asteroidHitSound.isPlaying()) asteroidHitSound.stop();

    // Game Over Title
    ofSetColor(0, 255, 0);
    ofRectangle titleRect = titleFont.getStringBoundingBox("GAME OVER", 0, 0);
    float titleX = ofGetWidth() / 2 - titleRect.getWidth() / 2;
    float titleY = 100;
    titleFont.drawString("GAME OVER", titleX, titleY);

    // Final Score
    ofSetColor(255);
    string finalScore = "Final Score:";
    ofRectangle scoreRect = textFont.getStringBoundingBox(finalScore, 0, 0);
    float scoreX = ofGetWidth() / 2 - scoreRect.getWidth() / 2;
    float scoreY = ofGetHeight() / 2 - 100;
    textFont.drawString(finalScore, scoreX, scoreY);

    // Final Score Value
    ofSetColor(0, 255, 0);
    string scoreValue = ofToString(score);
    ofRectangle scoreValueRect = titleFont.getStringBoundingBox(scoreValue, 0, 0);
    float scoreValueX = ofGetWidth() / 2 - scoreValueRect.getWidth() / 2;
    float scoreValueY = scoreY + 70;
    titleFont.drawString(scoreValue, scoreValueX, scoreValueY);

    // Player Deaths
    ofSetColor(255);
    string deathsText = "Player Deaths:";
    ofRectangle deathsRect = textFont.getStringBoundingBox(deathsText, 0, 0);
    float deathsX = ofGetWidth() / 4 - deathsRect.getWidth() / 2;
    float deathsY = scoreValueY + 70;
    textFont.drawString(deathsText, deathsX, deathsY);

    // Player Deaths Value
    ofSetColor(0, 255, 0);
    string deathsValue = ofToString(playerDeaths);
    ofRectangle deathsValueRect = titleFont.getStringBoundingBox(deathsValue, 0, 0);
    float deathsValueX = ofGetWidth() / 4 - deathsValueRect.getWidth() / 2;
    float deathsValueY = deathsY + 70;
    titleFont.drawString(deathsValue, deathsValueX, deathsValueY);

    // Asteroids Destroyed
    ofSetColor(255);
    string asteroidsText = "Asteroids Destroyed:";
    ofRectangle asteroidsRect = textFont.getStringBoundingBox(asteroidsText, 0, 0);
    float asteroidsX = ofGetWidth() * 3 / 4 - asteroidsRect.getWidth() / 2;
    float asteroidsY = scoreValueY + 70;
    textFont.drawString(asteroidsText, asteroidsX, asteroidsY);

    // Asteroids Destroyed Value
    ofSetColor(0, 255, 0);
    string asteroidsValue = ofToString(asteroidsDestroyed);
    ofRectangle asteroidsValueRect = titleFont.getStringBoundingBox(asteroidsValue, 0, 0);
    float asteroidsValueX = ofGetWidth() * 3 / 4 - asteroidsValueRect.getWidth() / 2;
    float asteroidsValueY = asteroidsY + 70;
    titleFont.drawString(asteroidsValue, asteroidsValueX, asteroidsValueY);

    // Exit Button
    ofRectangle exitRect = textFont.getStringBoundingBox("EXIT", 0, 0);
    float exitX = ofGetWidth() / 2 - exitRect.getWidth() / 2;
    float exitY = ofGetHeight() - 50;

    if (exitHovered) {
        ofSetColor(0, 255, 0); // Green when hovered
    }
    else {
        ofSetColor(255); // White
    }
    textFont.drawString("EXIT", exitX, exitY);

    ofSetColor(255);
}

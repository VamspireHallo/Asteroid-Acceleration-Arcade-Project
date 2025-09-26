#pragma once

#include "ofMain.h"
#include "Shape.h"
#include "Player.h"
#include "Emitter.h"
#include "Asteroid.h"
#include "SmallAsteroid.h"
#include "CollisionSystem.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	void startScreen();
	void instructionsScreen();
	void startGame();
	void endGame();
	void spawnAsteroids();

	enum GameState {
		START,
		INSTRUCTIONS,
		GAMEPLAY,
		ENDSCREEN
	};
	GameState gameState = START;
	bool startGameHovered;
	bool instructionsHovered;
	bool exitHovered;
	float timer = 120.0f;
	bool timesUp = false;
	float minDistanceFromPlayer = 100.0f;

	Player* player = NULL;

	// Movement flags
	bool moveLeft = false;
	bool moveRight = false;
	bool moveForward = false;
	bool moveBackward = false;

	ParticleSystem* bulletSystem = NULL;
	bool shooting = false;
	float fireRate = 0.2f;
	float lastShotTime = 0.0f;
	float lastShootSoundTime = 0.0f;
	float shootSoundCooldown = 0.1f;

	std::vector<Asteroid> asteroids;
	std::vector<SmallAsteroid> smallAsteroids;

	int score = 0;
	int playerDeaths = 0;
	int asteroidsDestroyed = 0;

	CollisionSystem collisionSystem;

	ofSoundPlayer thrustSound;
	ofSoundPlayer whirlSound;
	ofSoundPlayer shootSound;
	ofSoundPlayer playerHitSound;
	ofSoundPlayer asteroidHitSound;

	ofImage background;

	ofTrueTypeFont titleFont;
	ofTrueTypeFont textFont;
};

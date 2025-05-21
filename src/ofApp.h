#pragma once

#include "ofMain.h"
#include "glm/gtx/rotate_vector.hpp"
#include <array>
#include <vector>
#include <iostream>
#include <algorithm>

class entity;

constexpr bool	withinDistance(glm::vec2 i1, glm::vec2 i2, float maxLen);
float			getAngleDeg(glm::vec2 a, glm::vec2 b);
bool			collisionCheck(entity a, entity b);

static constexpr glm::vec2 windowSize = {768, 768};

class entity {
	public:
		glm::vec2	pos;
		glm::vec2	dir;
		glm::vec2	size;
};

class ship : public entity {
	public:
		ofColor					colour;
		static constexpr float	maxTurnDeg = 2.0f;
		float 					health, maxHealth;
		float					speed;

		// Constructor for player
		ship() {
			pos = windowSize / 2;
			dir = glm::vec2(1, 0);
			size = glm::vec2(40, 20);
			colour = ofColor(255, 255, 255);
			health = 100;
			maxHealth = 100;
			speed = 3.5f;
		}

		// Constructor for enemies
		ship(const glm::vec2 & playerPos, int wave) {
			// find a random position that is not too close to the player
			do {
				pos = glm::vec2(ofRandom(100, windowSize.x - 100), ofRandom(100, windowSize.y - 100));
			} while (withinDistance(pos, playerPos, 150) == false);
			dir = glm::normalize(glm::vec2(ofRandom(-1, 1), ofRandom(-1, 1)));
			size = glm::vec2(35, 18);
			colour = ofColor(200, 30, 30, 255);
			health = 25 + 10 * wave;
			maxHealth = health;
			speed = 2.0f + 0.1f * wave;
		}

		void drawHealthBar() const {
			if (health != maxHealth) {
				const float barSize = 20.0f + maxHealth / 4;
				ofSetColor(40, 40, 40);
				ofDrawRectangle(-barSize / 2, -(size.x / 2 + 10), barSize, 5);
				const float healthRatio = std::max(0.0f, health / maxHealth);
				const ofColor gradient = ofColor(255 - healthRatio * 255, healthRatio * 255, 0);
				ofSetColor(gradient);
				ofDrawRectangle(-barSize / 2, -(size.x / 2 + 10), barSize * healthRatio, 5);
			}
		}

		void draw() const {
			ofPushMatrix(); {
				ofTranslate(pos); // move origin to center of ship
				drawHealthBar();
				ofRotateDeg(glm::degrees(atan2(dir.y, dir.x))); // rotate to face pointAt
				ofSetColor(colour);
				ofBeginShape(); {
					for (const glm::vec2& point : shipPoints) {
						ofVertex(point.x * size.x, point.y * size.y);
					}
				}
				ofEndShape(true);
				//ofDrawRectangle(-size.x / 2, -size.y / 2, size.x, size.y);
			} ofPopMatrix();
		}

		bool collidesWith(const ship & ship) const {
			return withinDistance(pos, ship.pos, (std::min(size.x, size.y) + std::min(ship.size.x, ship.size.y)) / 2) == false;
		}

	private:
		constexpr static std::array<glm::vec2, 5> shipPoints = {
			glm::vec2(-0.5, -0.5),
			glm::vec2( 0.25,  -0.5),
			glm::vec2( 0.5,  0),
			glm::vec2( 0.25,  0.5),
			glm::vec2(-0.5,  0.5),
		};
};

class keg : public entity {
	public:
		inline static const ofColor	colour = ofColor(255, 127, 0, 255);
		uint64_t					ticks = 0;
		bool						isActive = true;

		keg(const ship& fromShip) {
			const glm::vec2	backOfShip = fromShip.pos + fromShip.dir * (-fromShip.size.x / 2);
			pos = backOfShip;
			dir = { 0, 1 }; // normalised default direction
			size = { 16, 16 };
		}

		void draw() const {
			ofSetColor(colour);
			ofDrawRectangle(pos - size / 2, size.x, size.y);
		}

		void update(ship& player, std::vector<ship>& enemies) {
			ticks++;
			checkExplode(player);
			for (ship & enemy : enemies) {
				if (checkExplode(enemy)) {
					break;
				}
			}
			if (ticks > 1200) {
				size -= { 0.5f, 0.5f };
				if (size.x < 0 || size.y < 0) {
					isActive = false;
				}
			}
		}

		bool checkExplode(ship& victim) {
			if (isActive && ticks > 10 && collisionCheck(victim, *this)) {
				victim.health -= 25;
				isActive = false;
				ticks = 0;
			}
			return victim.health <= 0;
		}

		bool collidesWith(const ship& ship) const {
			return isActive && ticks > 100 && withinDistance(pos, ship.pos, (std::min(size.x, size.y) + std::min(ship.size.x, ship.size.y)) / 2) == false;
		}
};

enum GameState : int {
	PLAYING,
	ENDED,
};

struct gameStats {
	int wave = 0;
	int kegsLaunched = 0;
	int kills = 0;
};

struct fonts {
	ofTrueTypeFont	small;
	ofTrueTypeFont	medium;
};

struct sprites {
	ofTexture	keg; // need raw texture for higher texure quality at low res
};

struct input {
	bool							addKeg;
	static constexpr int			kegDelay = 5;
	uint64_t						nextKeg;
	std::unordered_map<int, bool>	presses;
};

class ofApp : public ofBaseApp{
	public:
		void reset();
		void setup();

		void update();
		void updatePlayer();
		void updateKegs();
		void updateEnemies();

		void drawBackground();
		void drawStats();
		void drawKegs();
		void draw();
	
		void audioOut(ofSoundBuffer &outBuffer);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		GameState			gameState = GameState::PLAYING;
		uint64_t			ticks = 0;

		float				maxKegs = 5, loadedKegs = maxKegs;
		std::vector<keg>	kegs;
		ship				player;
		std::vector<ship>	enemies;

		input				input;
		gameStats			stats;

		fonts				fonts;
		sprites				sprites;
		ofImage				background;
};

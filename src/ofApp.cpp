#include "ofApp.h"
#include "ofMain.h"

//--------------------------------------------------------------

void ofApp::reset() {
	gameState = GameState::PLAYING;
	ticks = 0;
	stats = { 0, 0, 0 };
	player = ship();
	kegs.clear();
	enemies.clear();
	input.addKeg = false;
	input.presses.clear();
}

void ofApp::setup() {
	ofSetFrameRate(60);
	ofEnableAlphaBlending();

	background.allocate(windowSize.x, windowSize.y, OF_IMAGE_COLOR);
	ofDisableArbTex();

	ofLoadImage(sprites.keg, "keg.png");
	sprites.keg.generateMipmap();
	sprites.keg.setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	fonts.small.load("verdana.ttf", 12);
	fonts.medium.load("verdana.ttf", 24);
	reset();
}

//--------------------------------------------------------------

void ofApp::updatePlayer() {
	float turnAngle = 0;
	if (input.presses['a']) {
		turnAngle -= ship::maxTurnDeg;
	}
	if (input.presses['d']) {
		turnAngle += ship::maxTurnDeg;
	}
	player.dir = glm::rotate(player.dir, glm::radians(turnAngle));
	player.pos += player.dir * player.speed;
	if (player.health <= 0) {
		gameState = GameState::ENDED;
	}
}

void ofApp::updateKegs() {
	if (input.addKeg) {
		stats.kegsLaunched++;
		kegs.emplace_back(player);
		input.addKeg = false;
	}

	for (keg& keg : kegs) {
		keg.update(player, enemies);
	}

	std::erase_if(kegs, [](const keg& keg) {
		return keg.isActive == false;
	});
}

void ofApp::updateEnemies() {
	if (ticks % std::max(120, 210 - stats.wave * 15) == 60) {
		enemies.emplace_back(player.pos, stats.wave);
	}
	for (ship & enemy : enemies) {
		enemy.dir = glm::rotate(enemy.dir, glm::radians(std::clamp(getAngleDeg(enemy.dir, player.pos - enemy.pos), -ship::maxTurnDeg, ship::maxTurnDeg)));
		enemy.pos += enemy.dir * enemy.speed;
		if (collisionCheck(player, enemy)) {
			player.health -= 25;
			enemy.health = 0;
		}
	}
	stats.kills += std::erase_if(enemies, [](const ship & enemy) {
		return enemy.health <= 0;
	});
}

void ofApp::update() {
	ticks++;
	loadedKegs = std::min(maxKegs, loadedKegs + 0.01f);
	if (gameState == GameState::ENDED) {
		return;
	}
	stats.wave = stats.kills / 10;
	updateKegs();
	updatePlayer();
	updateEnemies();
}

//--------------------------------------------------------------

void ofApp::drawBackground() {
	ofPixels& pixels = background.getPixels();
	for (int y = 0; y < windowSize.y; ++y) {
		for (int x = 0; x < windowSize.x; ++x) {
			int index = (y * windowSize.x + x) * 3;
			float noiseVal = ofNoise(x * 0.002, y * 0.002, ticks * 0.003);
			int darkness = noiseVal * 50;
			pixels[index + 0] = 0;
			pixels[index + 1] = 75 + darkness;
			pixels[index + 2] = 125 + darkness;
		}
	}
	ofSetColor(255);
	background.update();
	background.draw(0, 0);
}

void ofApp::drawStats() {
	ofSetColor(255);
	fonts.small.drawString("Wave: " + std::to_string(stats.wave + 1), 15, 30);
	fonts.small.drawString("Kegs launched: " + std::to_string(stats.kegsLaunched), 15, 55);
	fonts.small.drawString("Ships sunk: " + std::to_string(stats.kills), 15, 80);
}

static glm::vec2 getPos(glm::vec2 kegSize, int i) {
	const float padding = 10;
	const float spacing = 3;

	return glm::vec2(windowSize.x - (kegSize.x + spacing) * 5 - padding + i * (kegSize.x + spacing), windowSize.y - kegSize.y - padding);
}

void ofApp::drawKegs() {
	constexpr glm::vec2 kegSize(25, 35);
	float				frac = loadedKegs - std::floor(loadedKegs);
	int					i = 0;

	ofSetColor(keg::colour);
	for (; i < std::floor(loadedKegs); i++) {
		sprites.keg.draw(getPos(kegSize, i), kegSize.x, kegSize.y);
	}
	
	if (frac) {
		sprites.keg.drawSubsection(
			getPos(kegSize, i).x,				getPos(kegSize, i).y,
			kegSize.x * frac,					kegSize.y,
			0,									0,
			sprites.keg.getWidth() * frac,		sprites.keg.getHeight()
		);
	}

	ofSetColor(255);
}

void ofApp::draw() {
	drawBackground();
	for (const keg& keg : kegs) {
		keg.draw();
	}
	for (const ship& enemy : enemies) {
		enemy.draw();
	}
	player.draw();
	drawKegs();
	drawStats();

	if (gameState == GameState::ENDED) {
		ofSetColor(0, 0, 0);
		fonts.medium.drawString("You Lost :(", windowSize.x / 2 - 90, windowSize.y / 2);
		fonts.medium.drawString("Press [SPACE] to restart", windowSize.x / 2 - 190, windowSize.y / 2 + 40);
	}
}

//--------------------------------------------------------------

void ofApp::audioOut(ofSoundBuffer & outBuffer) {
}

//--------------------------------------------------------------

void ofApp::keyPressed(int key) {
	switch (key) {
	case 'a':
	case 'd':
		input.presses[key] = true;
		break;
	case OF_KEY_SPACE:
		if (gameState == GameState::ENDED) {
			reset();
		} else if (input.nextKeg <= ticks && loadedKegs >= 1) {
			loadedKegs--;
			input.addKeg = true;
			input.nextKeg = ticks + input.kegDelay;
		}
		break;
	}
}

//--------------------------------------------------------------

void ofApp::keyReleased(int key) {
	switch (key) {
	case 'a':
	case 'd':
		input.presses[key] = false;
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
}

void ofApp::mouseDragged(int x, int y, int button) {
}

void ofApp::mousePressed(int x, int y, int button) {
}

void ofApp::mouseReleased(int x, int y, int button) {
}

void ofApp::mouseEntered(int x, int y) {
}

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

#include "ofMain.h"

// returns true if the distance between the two points is greater than maxLen
bool withinDistance(glm::vec2 a, glm::vec2 b, float maxLen) {
	float dx = a.x - b.x;
	float dy = a.y - b.y;

	return (dx * dx + dy * dy > maxLen * maxLen);
}

//-----------------------------------------------------------------------------

// returns the difference between two angles in degrees
// range: -180 <-> 180
float getAngleDeg(glm::vec2 a, glm::vec2 b) {
	a = glm::normalize(a);
	b = glm::normalize(b);

	// Compute the signed angle in radians and convert to degrees
	return glm::degrees(atan2(a.x * b.y - a.y * b.x, glm::dot(a, b)));
}

//-----------------------------------------------------------------------------

static std::array<glm::vec2, 4> getCorners(glm::vec2 pos, glm::vec2 size, glm::vec2 dir) {
	glm::vec2 perp = glm::vec2(-dir.y, dir.x); // Perpendicular direction (right vector)

	glm::vec2 halfDir = dir * (size.y / 2.0f);
	glm::vec2 halfPerp = perp * (size.x / 2.0f);

	return {
		pos + halfDir + halfPerp,
		pos + halfDir - halfPerp,
		pos - halfDir - halfPerp,
		pos - halfDir + halfPerp
	};
}

// Project polygon onto axis and get min/max
static void projectPolygon(const std::array<glm::vec2, 4> & corners, glm::vec2 axis, float & min, float & max) {
	min = max = glm::dot(corners[0], axis);
	for (int i = 1; i < 4; ++i) {
		float projection = glm::dot(corners[i], axis);
		if (projection < min) min = projection;
		if (projection > max) max = projection;
	}
}

class entity {
public:
	glm::vec2 pos;
	glm::vec2 dir;
	glm::vec2 size;
	ofColor colour;
};

bool collisionCheck(entity a, entity b) {
	std::array<glm::vec2, 4> aCorners = getCorners(a.pos, a.size, glm::normalize(a.dir));
	std::array<glm::vec2, 4> bCorners = getCorners(b.pos, b.size, glm::normalize(b.dir));

	// Check 4 axes: 2 from A, 2 from B (each rect's edges)
	std::array<glm::vec2, 4> axes = {
		glm::normalize(aCorners[1] - aCorners[0]),
		glm::normalize(aCorners[3] - aCorners[0]),
		glm::normalize(bCorners[1] - bCorners[0]),
		glm::normalize(bCorners[3] - bCorners[0])
	};

	for (const auto& axis : axes) {
		float minA, maxA, minB, maxB;
		projectPolygon(aCorners, axis, minA, maxA);
		projectPolygon(bCorners, axis, minB, maxB);

		// If projections don't overlap, there's a separating axis -> no collision
		if (maxA < minB || maxB < minA)
			return false;
	}

	// No separating axis found -> collision
	return true;
}


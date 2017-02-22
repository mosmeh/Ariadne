#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>
#include <queue>
#include "Common.h"
#include "Effect.h"
#include "Maze.h"
#include "Script.h"

namespace Ariadne {

const double COLLISION_DETECTION_RADIUS = 0.3;
const double MAP_ZOOM = 2.0;
const double LOCAL_MAP_ZOOM = 2.0;
const unsigned int LOCAL_MAP_REGION = 50;

class Level {
public:
	Level() = default;
	Level(const FilePath& mazeFile);
	Size getSize() const;
	Point getStartPos() const;
	Point getGoalPos() const;
	Optional<int> getEventId(const Vec2& pos) const;
	bool intersects(const Vec2& point) const;
	bool pointingWall() const;
	Wall& getPointingWall();
	bool erasePointingWall();
	void resetWallColor();
	Point findNearestEmptyCell(const Vec2& pos) const;
	Optional<std::vector<Point> > findShortestPathFrom(const Vec2& pos) const;
	void update();
	void drawForward() const;
	void drawGlobalMap(const Vec2& pos) const;
	void drawLocalMap(const Vec2& pos) const;

private:
	Size size;
	Grid<bool> isWall;
	Point startPos, goalPos;
	//std::vector<RectF> eventRegions;
	std::vector<Point> eventCells;

	Array<Particle> particles;

	std::vector<Wall> walls;
	TransformedMesh groundMesh, ceilingMesh;
	Plane goalPlane;

	Texture mapTexture, borderedMapTexture;

	void updateMap();
	Optional<size_t> getPointingWallId() const;
};

}
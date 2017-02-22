#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>
#include "Common.h"
#include "Effect.h"

namespace Ariadne {

const double WALL_THICKNESS = 0.5;
const double WALL_HEIGHT = 5.0;
const double WALL_HALF_LENGTH = 2.0;
const double WALL_LENGTH = 2 * WALL_HALF_LENGTH;

enum class CellType {
	EMPTY = 0,
	NORMAL_WALL = 1,
	UNBREAKABLE_WALL = 2,
	START = 3,
	GOAL = 4,
	EVENT = 5
};

class CoordinateConverter {
public:
	CoordinateConverter() = delete;

	static Vec2 mirror(const Vec2& pos) {
		return {-pos.x, pos.y};
	}

	static Point mirror(const Point& pos) {
		return {-pos.x, pos.y};
	}

	static Vec2 mapToPlanar(const Point& pos) {
		return WALL_LENGTH * pos + WALL_HALF_LENGTH * Point(1, 1);
	}

	static Point planarToMap(const Vec2& pos) {
		return (pos / WALL_LENGTH).asPoint();
	}

	static Vec3 planarToSolid(const Vec2& pos, const double height) {
		return {-pos.x, height, pos.y};
	}
};

class Wall {
public:
	Wall(const Point& pos_, const Box& box_, const Box& rawBox_, const bool breakable_, const bool half_, const bool adjacentWallExists_[4]);

	bool isBreakable() const;
	bool isMarkable(const Direction direction) const;
	const Point& getPos() const;
	const Box& getBox() const;
	const Box& getRawBox() const;
	Direction getPointingDirection() const;
	void setColor(const Color& color_);
	void addMarking(const String& marking, Direction direction);
	double distanceFrom(const Vec3 pos) const;
	void update();
	void drawForward(const Texture& texture) const;

private:
	struct Marking {
		Marking(const Direction direction_, const Texture& texture_) :
			direction(direction_),
			texture(texture_) {}

		const Direction direction;
		const Texture texture;
		double alpha = 0;
	};

	Point pos;
	Box box, rawBox;
	bool breakable, half;
	bool adjacentWallExists[4];
	Color color = Palette::White;

	TransformedMesh transformedMesh;
	Plane faces[4];

	std::vector<Marking> markings;
	Effect effect;
};

class MazeParser {
public:
	MazeParser() = delete;

	static std::tuple<Size, Grid<bool>, std::vector<Wall>, Point, Point, std::vector<Point> > parse(const FilePath& filePath);
};

}

#pragma once
#include <Siv3D.hpp>
#include "Common.h"

class Level {
public:
	Level() = default;
	Level(const String& filename);
	Size getSize() const;
	void draw() const;
	void drawForward() const;
	bool intersects(Vec2 point) const;

private:
	std::vector<Box> walls;
	int rows, columns;
};

enum class CellType {
	EMPTY = 0,
	WALL = 1
};
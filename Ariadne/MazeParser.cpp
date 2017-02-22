#include "Maze.h"

namespace Ariadne {

std::tuple<Size, Grid<bool>, std::vector<Wall>, Point, Point, std::vector<Point> > MazeParser::parse(const FilePath& filePath) {
	const CSVReader csvReader(filePath);
	const size_t rows = csvReader.rows;
	const size_t columns = csvReader.columns(0);
	Grid<CellType> grid(columns, rows);
	Grid<bool> isWall(columns, rows);
	for (const auto y : step(rows)) {
		if (csvReader.columns(y) != columns) {
			// invalid
		}
		for (const auto x : step(columns)) {
			grid.at(y, x) = CellType(csvReader.get<int>(y, x));
			isWall.at(y, x) = grid.at(y, x) == CellType::NORMAL_WALL
						   || grid.at(y, x) == CellType::UNBREAKABLE_WALL;
		}
	}

	Point startPos, goalPos;
	std::vector<Wall> walls;
	std::vector<Point> eventCells;
	for (const auto y : step(rows)) {
		for (const auto x : step(columns)) {
			const auto type = grid.at(y, x);
			switch (type) {
			case CellType::NORMAL_WALL:
			case CellType::UNBREAKABLE_WALL:
			{
				const bool wallAtWest = x > 0 && isWall.at(y, x - 1),
						   wallAtEast = x < columns - 1 && isWall.at(y, x + 1),
						   wallAtNorth = y > 0 && isWall.at(y - 1, x),
						   wallAtSouth = y < rows - 1 && isWall.at(y + 1, x);
				const bool breakable = type == CellType::NORMAL_WALL
					&& ((wallAtWest || wallAtEast) ^ (wallAtNorth || wallAtSouth));
				const bool adjacentWallExists[4] = {
					wallAtNorth, wallAtSouth, wallAtEast, wallAtWest
				};

				if (wallAtWest || wallAtEast) {
					bool half = true;
					double offset = WALL_HALF_LENGTH;
					if (wallAtWest) {
						offset = 0;
						if (wallAtEast) {
							half = false;
						}
					}

					double length = (half ? 1 : 2) * WALL_HALF_LENGTH;

					Box rawBox(-(x * WALL_LENGTH + offset + length / 2), WALL_HEIGHT / 2, (2 * y + 1) * WALL_HALF_LENGTH, length, WALL_HEIGHT, WALL_THICKNESS);

					if ((wallAtNorth ^ wallAtSouth) && (wallAtWest ^ wallAtEast)) {
						if (wallAtEast) {
							offset -= WALL_THICKNESS / 2;
						}
						length += WALL_THICKNESS / 2;
					}

					Box box(-(x * WALL_LENGTH + offset + length / 2), WALL_HEIGHT / 2, (2 * y + 1) * WALL_HALF_LENGTH, length, WALL_HEIGHT, WALL_THICKNESS);
					walls.emplace_back(Point(x, y), box, rawBox, breakable, half, adjacentWallExists);
				}

				if (wallAtNorth || wallAtSouth) {
					bool half = true;
					double offset = WALL_HALF_LENGTH;
					if (wallAtNorth) {
						offset = 0;
						if (wallAtSouth) {
							half = false;
						}
					}

					double length = (half ? 1 : 2) * WALL_HALF_LENGTH;

					Box rawBox(-(2 * static_cast<int>(x) + 1) * WALL_HALF_LENGTH, WALL_HEIGHT / 2, y * WALL_LENGTH + offset + length / 2, WALL_THICKNESS, WALL_HEIGHT, length);

					if ((wallAtWest ^ wallAtEast) && (wallAtNorth ^ wallAtSouth)) {
						if (wallAtSouth) {
							offset += WALL_THICKNESS / 2;
						}
						length -= WALL_THICKNESS / 2;
					}

					Box box(-(2 * static_cast<int>(x) + 1) * WALL_HALF_LENGTH, WALL_HEIGHT / 2, y * WALL_LENGTH + offset + length / 2, WALL_THICKNESS, WALL_HEIGHT, length);
					walls.emplace_back(Point(x, y), box, rawBox, breakable, half, adjacentWallExists);
				}
			}
			break;
			case CellType::START:
				startPos = {static_cast<int>(x), static_cast<int>(y)};
				break;
			case CellType::GOAL:
				goalPos = {static_cast<int>(x), static_cast<int>(y)};
				break;
			case CellType::EVENT:
				eventCells.emplace_back(x, y);
				break;
			}
		}
	}

	return {Size(columns, rows), isWall, walls, startPos, goalPos, eventCells};
}

}
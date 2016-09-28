#include "Level.h"

Level::Level(const String& filename) {
	auto csvReader = CSVReader(filename);
	rows = csvReader.rows;
	columns = csvReader.columns(0);
	auto grid = Grid<CellType>(rows, columns);
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < columns; ++x) {
			grid.at(x, y) = CellType(csvReader.get<int>(y, x));
		}
	}

	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < columns; ++x) {
			if (grid.at(x, y) == CellType::WALL) {
				bool wallAtLeft = x > 0 && grid.at(x - 1, y) == CellType::WALL;
				bool wallAtRight = x < columns - 1 && grid.at(x + 1, y) == CellType::WALL;

				if (wallAtLeft || wallAtRight) {
					double length = WALL_HALF_LENGTH;
					int offset = 1;
					if (wallAtLeft) {
						offset = 0;
						if (wallAtRight) {
							length = 2 * WALL_HALF_LENGTH;
						}
					}
					walls.push_back(Box((2 * x + offset) * WALL_HALF_LENGTH + length / 2, WALL_HEIGHT / 2, (2 * y + 1) * WALL_HALF_LENGTH, length, WALL_HEIGHT, WALL_THICKNESS));
				}

				bool wallAtUp = y > 0 && grid.at(x, y - 1) == CellType::WALL;
				bool wallAtDown = y < rows - 1 && grid.at(x, y + 1) == CellType::WALL;

				if (wallAtUp || wallAtDown) {
					double length = WALL_HALF_LENGTH;
					int offset = 1;
					if (wallAtUp) {
						offset = 0;
						if (wallAtDown) {
							length = 2 * WALL_HALF_LENGTH;
						}
					}
					walls.push_back(Box((2 * x + 1) * WALL_HALF_LENGTH, WALL_HEIGHT / 2, (2 * y + offset) * WALL_HALF_LENGTH + length / 2, WALL_THICKNESS, WALL_HEIGHT, length));
				}
			}
		}
	}
}

Size Level::getSize() const {
	return Size(columns, rows);
}

void Level::draw() const {
	assert(TextureAsset::IsRegistered(L"wall"));
	for (auto& w : walls) {
		w.draw(TextureAsset(L"wall"));
	}
}

void Level::drawForward() const {
	assert(TextureAsset::IsRegistered(L"wall"));
	for (auto& w : walls) {
		w.drawForward(TextureAsset(L"wall"));
	}
}

bool Level::intersects(Vec2 point) const {
	auto player = Sphere(point.x, WALL_HEIGHT / 2, point.y, 0.3);
	for (auto& w : walls) {
		if (w.intersects(player)) {
			return true;
		}
	}
	return false;
}
#include "Level.h"

namespace Ariadne {

Level::Level(const FilePath& mazeFile)  {
	std::tie(size, isWall, walls, startPos, goalPos, eventCells) = MazeParser::parse(mazeFile);

	groundMesh = Mesh(MeshData::Plane(size * WALL_LENGTH, {20, 20})).translated(Vec3(-size.x, 0, size.y) * WALL_HALF_LENGTH);
	ceilingMesh = groundMesh.translated({0, WALL_HEIGHT, 0});
	goalPlane = Plane(CoordinateConverter::planarToSolid(
		CoordinateConverter::mapToPlanar(goalPos), EPS), WALL_LENGTH);

	const auto c = goalPlane.center;
	const auto h = WALL_HALF_LENGTH;
	for (const auto i : step(30)) {
		particles.emplace_back(RandomVec3({c.x - h, c.x + h}, {0, 4}, {c.z - h, c.z + h}), 0.1, Palette::Yellow);
	}

	updateMap();
}

Size Level::getSize() const {
	return size;
}

Point Level::getStartPos() const {
	return startPos;
}

Point Level::getGoalPos() const {
	return goalPos;
}

Optional<int> Level::getEventId(const Vec2& pos) const {
	if (CoordinateConverter::planarToMap(pos) == goalPos) {
		return GOAL_ID;
	}

	const auto c = findNearestEmptyCell(pos);
	for (const auto i : step(eventCells.size())) {
		if (eventCells.at(i) == c) {
			return static_cast<int>(i);
		}
	}
	return none;
}

bool Level::intersects(const Vec2& pos) const {
	const Sphere player(CoordinateConverter::planarToSolid(pos, WALL_HEIGHT / 2), COLLISION_DETECTION_RADIUS);
	for (const auto& w : walls) {
		if (w.getBox().intersects(player)) {
			return true;
		}
	}
	return false;
}

bool Level::pointingWall() const {
	const auto ray = Graphics3D::ToRay(Window::Center());
	return std::any_of(walls.begin(), walls.end(), [&ray](const Wall& wall) {
		return ray.intersects(wall.getBox());
	});
}

Wall& Level::getPointingWall() {
	return walls.at(getPointingWallId().value());
}

bool Level::erasePointingWall() {
	const auto id = getPointingWallId();
	if (!id.has_value() || !walls.at(id.value()).isBreakable()) {
		return false;
	}

	const auto wallIter = walls.begin() + id.value();
	isWall.at(wallIter->getPos()) = false;
	walls.erase(wallIter);
	updateMap();
	return true;
}

Optional<size_t> Level::getPointingWallId() const {
	if (!pointingWall()) {
		return none;
	}

	const auto ray = Graphics3D::ToRay(Window::Center());
	std::vector<double> distances(walls.size());
	std::transform(walls.begin(), walls.end(), distances.begin(), [&ray](const Wall& wall) {
		const auto intersection = ray.intersectsAt(wall.getBox());
		if (intersection.has_value()) {
			return ray.origin.distanceFrom(intersection.value());
		} else {
			return Infinity<double>();
		}
	});

	return std::distance(distances.begin(), std::min_element(distances.begin(), distances.end()));
}

void Level::resetWallColor() {
	for (auto& wall : walls) {
		wall.setColor(Palette::White);
	}
}

Optional<std::vector<Point> > Level::findShortestPathFrom(const Vec2& pos) const {
	const auto start = findNearestEmptyCell(pos);

	struct Cell {
		Point from = Point(-1, -1);
		int cost = -1;
	};

	Grid<Cell> cells(size);
	cells[start].cost = 0;

	std::queue<Point> queue;
	std::unordered_map<Point, bool> queued;
	queue.push(start);
	queued[start] = true;
	while (!queue.empty()) {
		auto currentPos = queue.front();
		queue.pop();
		queued[currentPos] = false;

		for (const auto& d : NEIGHBORHOOD) {
			const auto to = currentPos + d;
			if (isWall.inBounds(static_cast<size_t>(to.y), static_cast<size_t>(to.x)) && !isWall.at(to)) {
				int cost = cells[currentPos].cost + 1;
				if (cells[to].cost < 0 || cost < cells[to].cost) {
					cells[to].from = currentPos;
					cells[to].cost = cost;
					if (!queued[to]) {
						queue.push(to);
						queued[to] = true;
					}
				}
			}
		}
	}

	if (cells[goalPos].from == Point(-1, -1)) {
		return none;
	}

	Point currentPos = goalPos;
	std::vector<Point> path;
	while (currentPos != start) {
		path.emplace_back(currentPos);
		currentPos = cells[currentPos].from;
	}
	std::reverse(path.begin(), path.end());
	return path;
}

void Level::update() {
	const double height = 4;
	for (auto& p : particles) {
		p.pos.y += 0.01f;
		if (p.pos.y > height) {
			p.pos.y = 0;
		}
		p.scaling.x = p.scaling.y = static_cast<float>(0.5 * (1 - 2 * abs(height / 2 - p.pos.y) / height));
	}

	for (auto& w : walls) {
		w.update();
	}
}

void Level::drawForward() const {
	groundMesh.drawForward(TextureAsset(L"ground"));
	ceilingMesh.drawForward(TextureAsset(L"wall"));
	goalPlane.draw(TextureAsset(L"ground"), Palette::Yellow);

	for (const auto& w : walls) {
		w.drawForward(TextureAsset(L"wall"));
	}

	const auto blendState = Graphics3D::GetBlendStateForward();
	const auto depthState = Graphics3D::GetDepthStateForward();
	Graphics3D::SetBlendStateForward(BlendState::Additive);
	Graphics3D::SetDepthStateForward(DepthState::TestOnly);

	Graphics3D::DrawParticlesForward(particles, TextureAsset(L"particle"));

	Graphics3D::SetBlendStateForward(blendState);
	Graphics3D::SetDepthStateForward(depthState);
}

void Level::updateMap() {
	Image img((size * WALL_LENGTH * MAP_ZOOM).asPoint(), AlphaF(0));
	for (const auto& w : walls) {
		const auto& box = w.getRawBox();
		if (box.w > box.d) {
			// horizontal
			Line(MAP_ZOOM * Vec2(-box.x - box.w / 2, box.z), MAP_ZOOM * Vec2(-box.x + box.w / 2, box.z)).overwrite(img, Palette::White);
		} else {
			// vertical
			Line(MAP_ZOOM * Vec2(-box.x, box.z - box.d / 2), MAP_ZOOM * Vec2(-box.x, box.z + box.d / 2)).overwrite(img, Palette::White);
		}
	}

	mapTexture = Texture(img);
	borderedMapTexture = Texture(img.bordered(LOCAL_MAP_REGION, AlphaF(0)));
}

void Level::drawGlobalMap(const Vec2& pos) const {
	if (mapTexture) {
		mapTexture.draw();
		Circle(MAP_ZOOM * pos, 3).draw(Palette::Red);
	}
}

void Level::drawLocalMap(const Vec2& pos) const {
	if (mapTexture) {
		const Vec2 leftTopPos = MAP_ZOOM * pos - Vec2::One * LOCAL_MAP_REGION / 2;
		const RectF region(Vec2::One * LOCAL_MAP_REGION + leftTopPos, Vec2::One * LOCAL_MAP_REGION);
		borderedMapTexture.uv(region).scale(LOCAL_MAP_ZOOM).draw(Vec2::Zero);
		Circle(Vec2::One * LOCAL_MAP_REGION / 2 * LOCAL_MAP_ZOOM, 3).draw(Palette::Red);
	}
}

Point Level::findNearestEmptyCell(const Vec2& pos) const {
	const auto point = CoordinateConverter::planarToMap(pos);
	if (!isWall[point]) {
		return point;
	}
	std::vector<double> distances(8);
	std::transform(NEIGHBORHOOD, NEIGHBORHOOD + 8, distances.begin(), [this, point, pos](const Point& d) {
		const auto neighbor = point + d;
		if (isWall[neighbor]) {
			return Infinity<double>();
		} else {
			return CoordinateConverter::mapToPlanar(neighbor).distanceFrom(pos);
		}
	});
	return point + NEIGHBORHOOD[std::distance(distances.begin(), std::min_element(distances.begin(), distances.end()))];
}

}

#include "Maze.h"

namespace Ariadne {

Wall::Wall(const Point& pos_, const Box& box_, const Box& rawBox_, const bool breakable_, const bool half_, const bool adjacentWallExists_[4]) :
	pos(pos_),
	box(box_),
	rawBox(rawBox_),
	breakable(breakable_),
	half(half_) {
	std::copy(adjacentWallExists_, adjacentWallExists_ + 4, adjacentWallExists);

	const auto a = (Vec2(-box.x, box.z) - Vec2(box_.w, box_.d) / 2) / WALL_HEIGHT;
	const auto b = (Vec2(-box.x, box.z) + Vec2(box_.w, box_.d) / 2) / WALL_HEIGHT;
	const Float2 uv[24] = {
		{0, 0}, {0, 0}, {0, 0}, {0, 0},
		{-b.x, 0}, {-a.x, 0}, {-b.x, 1}, {-a.x, 1},
		{a.y, 0}, {b.y, 0}, {a.y, 1}, {b.y, 1},
		{a.x, 0}, {b.x, 0}, {a.x, 1}, {b.x, 1},
		{-b.y, 0}, {-a.y, 0}, {-b.y, 1}, {-a.y, 1},
		{0, 0}, {0, 0}, {0, 0}, {0, 0},
	};
	transformedMesh = Mesh(MeshData::Box6(box_.size, uv)).translated(box_.center);

	faces[0] = Plane(box.x, WALL_HEIGHT / 2, box.z - box.d / 2 - EPS, box.w, WALL_HEIGHT, Quaternion::Pitch(-Math::Pi / 2));
	faces[1] = Plane(box.x, WALL_HEIGHT / 2, box.z + box.d / 2 + EPS, box.w, WALL_HEIGHT, Quaternion::RollPitchYaw(0, -Math::Pi / 2, Math::Pi));

	faces[2] = Plane(box.x - box.w / 2 - EPS, WALL_HEIGHT / 2, box.z, box.d, WALL_HEIGHT, Quaternion::RollPitchYaw(Math::Pi / 2, -Math::Pi / 2, 0));
	faces[3] = Plane(box.x + box.w / 2 + EPS, WALL_HEIGHT / 2, box.z, box.d, WALL_HEIGHT, Quaternion::RollPitchYaw(Math::Pi / 2, -Math::Pi / 2, Math::Pi));
}

bool Wall::isBreakable() const {
	return breakable;
}

bool Wall::isMarkable(const Direction direction) const {
	if (adjacentWallExists[static_cast<size_t>(direction)]) {
		return false;
	}
	if (std::any_of(markings.begin(), markings.end(), [direction](const Marking& m) {
		return m.direction == direction; })) {
		return false;
	}
	if (box.w > box.d) {
		return direction == Direction::NORTH
			|| direction == Direction::SOUTH;
	} else {
		return direction == Direction::WEST
			|| direction == Direction::EAST;
	}
}

const Point& Wall::getPos() const {
	return pos;
}

const Box& Wall::getBox() const {
	return box;
}

const Box& Wall::getRawBox() const {
	return rawBox;
}

Direction Wall::getPointingDirection() const {
	const auto ray = Graphics3D::ToRay(Window::Center());
	std::vector<double> distances(4);
	std::transform(faces, faces + 4, distances.begin(), [&ray](const Plane& face) {
		const auto intersection = ray.intersectsAt(face);
		if (intersection.has_value()) {
			return intersection.value().distanceFrom(ray.origin);
		} else {
			return Infinity<double>();
		}
	});
	return Direction(std::distance(distances.begin(), std::min_element(distances.begin(), distances.end())));
}

void Wall::setColor(const Color& color_) {
	color = color_;
}

void Wall::addMarking(const String& mark, Direction direction) {
	Image img((faces[static_cast<size_t>(direction)].size * 50).asPoint(), AlphaF(0));
	int size = 128;
	while (Font(size).region(mark).w > img.width - 5 && size > 1) {
		--size;
	}
	Font(size).overwriteCenter(img, mark, Palette::White);
	markings.emplace_back(direction, Texture(img, TextureDesc::For3D));

	const size_t i = markings.size() - 1;
	effect.add<LambdaEffect>([this, i](const double t) {
		if (t > 1) {
			return false;
		}
		markings.at(i).alpha = t;
		return true;
	});
}

double Wall::distanceFrom(const Vec3 pos_) const {
	return box.center.distanceFrom(pos_);
}

void Wall::update() {
	effect.update();
}

void Wall::drawForward(const Texture& texture) const {
	transformedMesh.drawForward(texture, color);
	for (const auto& m : markings) {
		if (m.texture) {
			Color markColor;
			if (effect.hasEffects()) {
				markColor = AlphaF(m.alpha);
			} else {
				markColor = color;
			}
			faces[static_cast<size_t>(m.direction)].drawForward(m.texture, markColor);
		}
	}
}

}
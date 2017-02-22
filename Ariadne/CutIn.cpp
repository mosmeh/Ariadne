#include "Effect.h"

namespace Ariadne {

CutIn::CutIn(const TextureRegion& texture_, const double easeLength, const double stayLength, const Point& pointA_, const Point& pointB_) :
	texture(texture_),
	ease(easeLength),
	stay(stayLength),
	pointA(pointA_),
	pointB(pointB_),
	pointC(Point::Zero),
	easeOut(false) {}

CutIn::CutIn(const TextureRegion& texture_, const double easeLength, const double stayLength, const Point& pointA_, const Point& pointB_, const Point& pointC_) :
	texture(texture_),
	ease(easeLength),
	stay(stayLength),
	pointA(pointA_),
	pointB(pointB_),
	pointC(pointC_),
	easeOut(true) {}


bool CutIn::update(double t) {
	double alpha;
	Point pos;
	if (t < ease) {
		pos = (pointA + (pointB - pointA) * std::sin(Math::HalfPi * t / ease)).asPoint();
		alpha = std::sin(Math::HalfPi * t / ease);
	} else if (t < ease + stay) {
		pos = pointB;
		alpha = 1;
	} else if (easeOut && t < 2 * ease + stay) {
		pos = (pointB + (pointC - pointB) * (1 - std::sin(Math::HalfPi * (t - stay) / ease))).asPoint();
		alpha = std::sin(Math::HalfPi * (t - stay) / ease);
	} else {
		return false;
	}
	texture.drawAt(pos, AlphaF(alpha));

	return true;
}

}
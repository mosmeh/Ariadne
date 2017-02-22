#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>

namespace Ariadne {

class CutIn : public IEffect {
public:
	CutIn(const TextureRegion& texture_, const double easeDuration, const double stayDuration, const Point& pointA_, const Point& pointB_);
	CutIn(const TextureRegion& texture_, const double easeDuration, const double stayDuration, const Point& pointA_, const Point& pointB_, const Point& pointC_);
	bool update(double t) override;

private:
	const TextureRegion texture;
	const double ease, stay;
	const bool easeOut;
	const Point pointA, pointB, pointC;
};

class LambdaEffect : public IEffect {
public:
	LambdaEffect(const std::function<bool(double)>& updateFunc_) : updateFunc(updateFunc_) {};
	bool update(double t) {
		return updateFunc(t);
	}

private:
	const std::function<bool(double)> updateFunc;
};

class DelayedEffect : public IEffect {
public:
	DelayedEffect(const double delay_, const std::function<void(void)>& func_) : delay(delay_), func(func_) {}
	bool update(double t) {
		if (t > delay) {
			func();
			return false;
		}
		return true;
	}

private:
	const double delay;
	const std::function<void(void)> func;
};

}
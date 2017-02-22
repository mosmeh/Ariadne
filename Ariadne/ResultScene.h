#pragma once
#include <Siv3D.hpp>
#include "Common.h"

namespace Ariadne {

class ResultScene : public MyApp::Scene {
public:
	ResultScene();
	void init();
	void update();
	void draw() const;

private:
	const Font font;

	String format(const size_t rank, const Milliseconds& ms) const;
};

}
#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>
#include "Common.h"
#include "Button.h"

namespace Ariadne {

class TitleScene : public MyApp::Scene {
public:
	TitleScene();
	~TitleScene();
	void init();
	void update();
	void draw() const;

private:
	std::vector<Button> buttons;
	const Font font;
};

}
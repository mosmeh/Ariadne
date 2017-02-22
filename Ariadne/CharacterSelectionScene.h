#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>
#include "Common.h"
#include "Button.h"
#include "TextBox.h"
#include "Effect.h"

namespace Ariadne {

class CharacterSelectionScene : public MyApp::Scene {
public:
	CharacterSelectionScene();
	~CharacterSelectionScene();
	void init();
	void update();
	void draw() const;

private:
	const int BUTTON_COLUMNS = 3;

	Font font;

	Button startButton, backButton;
	std::vector<Button> characterButtons;
	TextBox textBox;
	Effect effect;

	size_t numMouseHoveringButtons = 0;

	Point calcPos(const Character character) const;
};

}
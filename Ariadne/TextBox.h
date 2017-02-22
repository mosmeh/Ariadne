#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>
#include "Common.h"

namespace Ariadne {

class TextBox {
public:
	TextBox(const Font& bodyFont_, const Font& nameFont_, const size_t bodyHeight_, const size_t nameHeight_, const Color& textColor_, const Color& backgroundColor_, const Padding& padding_ = Padding(0));

	void open(const String& text, const String& name_ = L"");
	void close();
	bool isOpened() const;
	bool cursorReachedEnd() const;
	void displayAllText();
	void update();
	void draw() const;

private:
	const Font bodyFont, nameFont;
	const Color textColor, backgroundColor;
	const Padding padding;
	String text;
	String wrappedText;
	String displayedText;
	String name;
	size_t cursorPos;
	const size_t bodyHeight, nameHeight;
	bool opened = false;
};

}

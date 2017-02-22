#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>
#include "Button.h"
#include "Common.h"

namespace Ariadne {

class InnerMessageBox {
public:
	enum class Style {
		OK,
		YES_NO
	};

	InnerMessageBox(const Font& font_,  const Padding& padding);
	void setTextColor(const Color& color);
	void setBackgroundColor(const Color& color);
	void setIcon(const TextureRegion& texture);
	void clearIcon();
	void show(const String& text_, const Style style_ = Style::OK);
	bool isShowed() const;
	bool hasCommand() const;
	Optional<MessageBoxCommand> getCommand() const;
	void reset();
	void update();
	void draw() const;

private:
	const Size buttonSize = {100, 50};

	const Font font;
	const Padding padding;
	String text;
	String wrappedText;
	Style style;
	Color textColor = Palette::White, backgroundColor = Palette::Black;
	TextureRegion icon;

	Rect rect;
	std::vector<Button> buttons;
	bool showed = false;

	Optional<MessageBoxCommand> selectedCommand;

	void addButton(const String& label, const MessageBoxCommand command);
};

}
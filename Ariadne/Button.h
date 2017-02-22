#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>

namespace Ariadne {

class Button {
public:
	enum class TextAlign {
		BOTTOM_CENTER,
		BOTTOM_LEFT,
		BOTTOM_RIGHT,
		MIDDLE_CENTER,
		MIDDLE_LEFT,
		MIDDLE_RIGHT,
		TOP_CENTER,
		TOP_LEFT,
		TOP_RIGHT
	};

	Button() = default;

	void setPos(const Point& pos);
	void setSize(const Size& size);
	void setFont(const Font& font_);
	void setText(const String& text_);
	void setTextColor(const Color& color);
	void setTextAlign(const TextAlign textAlign_);
	void setPadding(const Padding& padding_);
	void setTexture(const TextureRegion& texture_);
	void setEnabled(const bool enabled_);
	void setVisible(const bool visible_);
	void addOnClickedHandler(const std::function<void(void)>& handler);
	void addOnMouseEnterHandler(const std::function<void(void)>& handler);
	void addOnMouseLeaveHandler(const std::function<void(void)>& handler);
	bool isClicked() const;
	void click();
	void invokeHandlers();
	void draw() const;

private:
	Rect rect;

	Font font;
	String text;
	Color textColor = Palette::Black;
	TextAlign textAlign = TextAlign::MIDDLE_CENTER;
	Padding padding;

	TextureRegion texture;

	std::vector<std::function<void(void)> > onClickedHandlers, onMouseEnterHandlers, onMouseLeaveHandlers;
	bool enabled = true, visible = true;
	bool mouseHovers = false;
};

}
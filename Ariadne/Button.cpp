#include "Button.h"

namespace Ariadne {

void Button::setPos(const Point& pos) {
	rect.pos = pos;
}

void Button::setSize(const Size& size) {
	rect.size = size;
}

void Button::setFont(const Font& font_) {
	font = font_;
}

void Button::setText(const String& text_) {
	text = text_;
}

void Button::setTextColor(const Color& color) {
	textColor = color;
}

void Button::setTextAlign(const TextAlign textAlign_) {
	textAlign = textAlign_;
}

void Button::setPadding(const Padding& padding_) {
	padding = padding_;
}

void Button::setTexture(const TextureRegion& texture_) {
	texture = texture_;
}

void Button::addOnClickedHandler(const std::function<void(void)>& handler) {
	onClickedHandlers.push_back(handler);
}

void Button::setEnabled(const bool enabled_) {
	enabled = enabled_;
}
void Button::setVisible(const bool visible_) {
	visible = visible_;
}

void Button::addOnMouseEnterHandler(const std::function<void(void)>& handler) {
	onMouseEnterHandlers.push_back(handler);
}

void Button::addOnMouseLeaveHandler(const std::function<void(void)>& handler) {
	onMouseLeaveHandlers.push_back(handler);
}

bool Button::isClicked() const {
	return enabled && visible && rect.leftClicked;
}

void Button::click() {
	if (enabled && visible) {
		for (const auto& handler : onClickedHandlers) {
			handler();
		}
	}
}

void Button::invokeHandlers() {
	if (enabled && visible) {
		if (rect.leftClicked) {
			for (const auto& handler : onClickedHandlers) {
				handler();
			}
		}
		if (!mouseHovers && rect.mouseOver) {
			for (const auto& handler : onMouseEnterHandlers) {
				handler();
			}
		} else if (mouseHovers && !rect.mouseOver) {
			for (const auto& handler : onMouseLeaveHandlers) {
				handler();
			}
		}
	}

	mouseHovers = rect.mouseOver;
}

void Button::draw() const {
	if (!visible) {
		return;
	}

	if (enabled && visible) {
		if (rect.mouseOver) {
			if (texture.texture) {
				rect(texture).draw();
			} else {
				rect.draw();
			}
		} else {
			if (texture.texture) {
				rect(texture).draw(Palette::Lightgrey);
			} else {
				rect.draw(Palette::Lightgrey);
			}
		}
	} else {
		rect(texture).draw(Palette::Gray);
	}

	if (!text.isEmpty && font) {
		const auto region = font(text).region().size + Size(padding.right, padding.bottom);
		Point pos;
		switch (textAlign) {
		case TextAlign::BOTTOM_CENTER:
			pos = {(rect.w - region.x) / 2, rect.h - region.y};
			break;
		case TextAlign::BOTTOM_LEFT:
			pos = {padding.left, rect.h - region.y};
			break;
		case TextAlign::BOTTOM_RIGHT:
			pos = {rect.w - region.x, rect.h - region.y};
			break;
		case TextAlign::MIDDLE_CENTER:
			pos = {(rect.w - region.x) / 2, (rect.h - region.y) / 2};
			break;
		case TextAlign::MIDDLE_LEFT:
			pos = {padding.left, (rect.h - region.y) / 2};
			break;
		case TextAlign::MIDDLE_RIGHT:
			pos = {rect.w - region.x, (rect.h - region.y) / 2};
			break;
		case TextAlign::TOP_CENTER:
			pos = {(rect.w - region.x) / 2, padding.top};
			break;
		case TextAlign::TOP_LEFT:
			pos = {padding.left, padding.top};
			break;
		case TextAlign::TOP_RIGHT:
			pos = {rect.w - region.x, padding.top};
			break;
		}
		font(text).draw(pos + rect.pos, textColor);
	}
}

}
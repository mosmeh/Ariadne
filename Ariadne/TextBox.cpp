#include "TextBox.h"

namespace Ariadne {

TextBox::TextBox(const Font& bodyFont_, const Font& nameFont_, const size_t bodyHeight_, const size_t nameHeight_, const Color& bodyColor_, const Color& backgroundColor_, const Padding& padding_) :
	bodyFont(bodyFont_),
	nameFont(nameFont_),
	bodyHeight(bodyHeight_),
	nameHeight(nameHeight_),
	textColor(bodyColor_),
	backgroundColor(backgroundColor_),
	padding(padding_) {}

void TextBox::open(const String& message, const String& name_) {
	text = message;
	name = name_;
	wrappedText = TextWrapper::wrap(message, bodyFont, Window::Width() - padding.left - padding.right);
	cursorPos = 0;
	displayedText = L"";
	opened = true;
}

void TextBox::close() {
	opened = false;
}

bool TextBox::isOpened() const {
	return opened;
}

bool TextBox::cursorReachedEnd() const {
	return cursorPos >= wrappedText.length;
}

void TextBox::displayAllText() {
	displayedText = wrappedText;
	cursorPos = wrappedText.length;
}

void TextBox::update() {
	if (cursorPos < wrappedText.length) {
		displayedText.append({wrappedText[cursorPos++]});
	}
}

void TextBox::draw() const {
	if (opened) {
		if (name == L"") {
			Rect(Point(0, Window::Height() - bodyHeight), Size(Window::Width(), bodyHeight)).draw(backgroundColor);
		} else {
			Rect(Point(0, Window::Height() - bodyHeight - nameHeight), Size(Window::Width(), bodyHeight + nameHeight)).draw(backgroundColor);
			nameFont(name).draw(static_cast<double>(padding.left), static_cast<double>(Window::Height() - bodyHeight - nameFont.height / 2), textColor);
		}
		bodyFont(displayedText).draw(static_cast<double>(padding.left), static_cast<double>(padding.top + Window::Height() - bodyHeight), textColor);
	}
}

}
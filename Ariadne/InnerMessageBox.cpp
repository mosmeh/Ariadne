#include "InnerMessageBox.h"

namespace Ariadne {

InnerMessageBox::InnerMessageBox(const Font& font_, const Padding& padding_) : font(font_), padding(padding_) {}

void InnerMessageBox::setTextColor(const Color& color) {
	textColor = color;
}

void InnerMessageBox::setBackgroundColor(const Color& color) {
	backgroundColor = color;
}

void InnerMessageBox::setIcon(const TextureRegion& texture) {
	icon = texture;
}

void InnerMessageBox::clearIcon() {
	icon = TextureRegion();
}

void InnerMessageBox::show(const String& text_, const Style style_) {
	text = text_;
	style = style_;
	showed = true;
	selectedCommand = none;

	wrappedText = TextWrapper::wrap(text, font, Window::Width() - padding.left - padding.right);
	const auto region = font(wrappedText).region();

	rect.setSize(Window::Width() - padding.left - padding.right + (icon.texture.isEmpty() ? 0 : 60), region.h + buttonSize.y + padding.top + padding.bottom + 30);
	rect.setCenter(Window::Center());

	buttons.clear();
	buttons.shrink_to_fit();
	switch (style) {
	case Style::OK:
		addButton(L"OK", MessageBoxCommand::Ok);
		break;
	case Style::YES_NO:
		addButton(L"‚Í‚¢", MessageBoxCommand::Yes);
		addButton(L"‚¢‚¢‚¦", MessageBoxCommand::No);
		break;
	}
}

void InnerMessageBox::addButton(const String& label, const MessageBoxCommand command) {
	Button button;
	button.setSize(buttonSize);
	button.setFont(font);
	button.setText(label);
	button.addOnClickedHandler([this, command] {
		selectedCommand = command;
		showed = false;
	});
	buttons.push_back(button);

	for (const auto i : step(buttons.size())) {
		buttons.at(i).setPos({
			Window::Center().x + (buttonSize.x + 10) * (i - static_cast<double>(buttons.size()) / 2),
			rect.y + rect.h - buttonSize.y - padding.bottom
		});
	}
}

bool InnerMessageBox::isShowed() const {
	return showed;
}

bool InnerMessageBox::hasCommand() const {
	return selectedCommand.has_value();
}

Optional<MessageBoxCommand> InnerMessageBox::getCommand() const {
	return selectedCommand;
}

void InnerMessageBox::reset() {
	selectedCommand = none;
	clearIcon();
}

void InnerMessageBox::update() {
	if (showed) {
		for (auto& button : buttons) {
			button.invokeHandlers();
		}
		if (Input::AnyKeyClicked()) {
			switch (style) {
			case Style::OK:
				if (Input::KeyEnter.clicked || Input::KeyEscape.clicked) {
					selectedCommand = MessageBoxCommand::Ok;
				}
				break;
			case Style::YES_NO:
				if (Input::KeyEnter.clicked || Input::KeyY.clicked) {
					selectedCommand = MessageBoxCommand::Yes;
				} else if (Input::KeyEscape.clicked || Input::KeyN.clicked) {
					selectedCommand = MessageBoxCommand::No;
				}
				break;
			default:
				break;
			}
		}
		if (selectedCommand.has_value()) {
			showed = false;
		}
	}
}

void InnerMessageBox::draw() const {
	if (showed) {
		rect.draw(backgroundColor);
		const auto textPos = Window::Center() - Vec2(0, buttonSize.y / 2);
		font(wrappedText).drawCenter(textPos, textColor);
		if (!icon.texture.isEmpty()) {
			icon.resize(100, 100).draw(padding.left, (Window::Height() - 100) / 2);
		}
		for (const auto& button : buttons) {
			button.draw();
		}
	}
}

}
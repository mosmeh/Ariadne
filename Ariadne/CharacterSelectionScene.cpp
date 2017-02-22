#include "CharacterSelectionScene.h"

namespace Ariadne {

CharacterSelectionScene::CharacterSelectionScene() :
	font(14),
	textBox(Font(14), Font(16, Typeface::Bold), 200, 40, Palette::White, Color(0, 122, 204, 200), Padding(30, 200)) {}

CharacterSelectionScene::~CharacterSelectionScene() {
	SoundAsset(L"character-selection-bgm").stop();
}

void CharacterSelectionScene::init() {
	Graphics::SetBackground(BACKGROUND_COLOR);

	SoundAsset(L"character-selection-bgm").play();

	effect.setSpeed(3);

	const Size size(150, 50);
	startButton.setPos(Window::Size() - size - 50 * Point::One);
	startButton.setSize(size);
	startButton.setFont(Font(14));
	startButton.setText(m_data->textAssets.at(L"start"));
	startButton.addOnClickedHandler([this] {
		SoundAsset(L"start").play();
		changeScene(SceneType::MAZE);
	});

	backButton.setPos(Window::Size() - Point(size.x * 2 + 5, size.y) - 50 * Point::One);
	backButton.setSize(size);
	backButton.setFont(Font(14));
	backButton.setText(m_data->textAssets.at(L"back"));
	backButton.addOnClickedHandler([this] {
		SoundAsset(L"cancel").play();
		changeScene(SceneType::TITLE);
	});

	const Size buttonSize(100, 100);
	for (const auto character : m_data->availableCharacters) {
		const size_t i = characterButtons.size();
		Button button;
		button.setPadding({0, 10});
		button.setPos(Point(Window::Width() - 50 - BUTTON_COLUMNS * buttonSize.x, 50 * 2) + buttonSize.y * Point(i % BUTTON_COLUMNS, i / BUTTON_COLUMNS));
		button.setSize(buttonSize);
		if (m_data->isAccompanying(character)) {
			button.setTexture(CharacterAsset::get(character).checkedFace);
		} else {
			button.setTexture(CharacterAsset::get(character).face);
		}
		button.addOnClickedHandler([this, character, i] {
			auto& accChars = m_data->accompanying;
			const auto charIter = std::find(accChars.begin(), accChars.end(), character);
			auto& button = characterButtons.at(i);
			effect.clear();
			std::unordered_map<Character, Point> posBefore;
			for (const auto& ch : accChars) {
				posBefore[ch] = calcPos(ch);
			}
			if (charIter == accChars.end()) {
				if (accChars.size() < MAX_ACCOMPANYING_CHARACTERS) {
					SoundAsset(L"select").playMulti();
					m_data->accompanying.emplace_back(character);
					std::sort(m_data->accompanying.begin(), m_data->accompanying.end());
					const auto pos = calcPos(character);
					effect.add<LambdaEffect>([this, pos, character](const double t) {
						if (t > 1) {
							return false;
						}
						CharacterAsset::get(character).standing.scale(0.7).drawAt(pos, AlphaF(t));
						return true;
					});
					button.setTexture(CharacterAsset::get(character).checkedFace);
				} else {
					SoundAsset(L"error").playMulti();
				}
			} else {
				SoundAsset(L"select").playMulti();
				const auto pos = calcPos(character);
				effect.add<LambdaEffect>([this, pos, character] (const double t) {
					if (t > 1) {
						return false;
					}
					CharacterAsset::get(character).standing.scale(0.7).drawAt(pos, AlphaF(1 - t));
					return true;
				});
				posBefore.erase(posBefore.find(character));
				accChars.erase(charIter);
				button.setTexture(CharacterAsset::get(character).face);
			}

			for (const auto p : posBefore) {
				effect.add<LambdaEffect>([this, p, character](const double t) {
					if (t > 1) {
						return false;
					}
					CharacterAsset::get(p.first).standing.scale(0.7).drawAt(p.second + (calcPos(p.first) - p.second) * Easing::Sine(t));
					return true;
				});
			}
		});
		button.addOnMouseEnterHandler([this, character] {
			++numMouseHoveringButtons;
			const auto& asset = CharacterAsset::get(character);
			textBox.open(asset.description, asset.displayName);
		});
		button.addOnMouseLeaveHandler([this, character] {
			--numMouseHoveringButtons;
			if (numMouseHoveringButtons <= 0) {
				textBox.close();
			}
		});

		characterButtons.push_back(button);
	}
}

void CharacterSelectionScene::update() {
	for (auto& b : characterButtons) {
		b.invokeHandlers();
	}
	startButton.invokeHandlers();
	backButton.invokeHandlers();
	textBox.update();
	effect.update();
}

void CharacterSelectionScene::draw() const {
	font(L"{} / {}"_fmt, m_data->accompanying.size(), MAX_ACCOMPANYING_CHARACTERS).drawCenter(Window::Width() - 200, 50);
	if (!effect.hasEffects()) {
		for (const auto i : step(m_data->accompanying.size())) {
			const auto character = m_data->accompanying.at(i);
			CharacterAsset::get(character).standing.scale(0.7).drawAt(calcPos(character));
		}
	}
	for (const auto& b : characterButtons) {
		b.draw();
	}
	startButton.draw();
	backButton.draw();
	textBox.draw();
}

Point CharacterSelectionScene::calcPos(const Character character) const {
	auto& accChars = m_data->accompanying;
	const auto i = std::distance(accChars.begin(), std::find(accChars.begin(), accChars.end(), character));
	const auto interval = (Window::Width() - 450) / (m_data->accompanying.size() + 1);
	return {50 + interval * (i + 1), Window::Height() / 2};
}

}
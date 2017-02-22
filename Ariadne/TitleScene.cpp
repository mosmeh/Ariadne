#include "TitleScene.h"

namespace Ariadne {

TitleScene::TitleScene() : font(60) {}

TitleScene::~TitleScene() {
	SoundAsset(L"title-bgm").stop();
}

void TitleScene::init() {
	SoundAsset(L"title-bgm").play();

	const Size buttonSize(200, 50);
	if (m_data->showedTutorial || FileSystem::Exists(SCORES_CSV)) {
		for (const auto& p : m_data->levels) {
			const auto name = p.first;
			const auto scriptPath = p.second;

			Button button;
			button.setSize(buttonSize);
			button.setPos(Window::Center() + Point(-buttonSize.x / 2, 60 + (buttonSize.y + 5) * buttons.size()));
			button.setFont(Font(14));
			button.setText(name);
			button.addOnClickedHandler([this, name, scriptPath] {
				SoundAsset(L"level-select").play();
				m_data->levelName = name;
				m_data->scriptPath = scriptPath;
				m_data->accompanying = {Character::AQUA, Character::ASTREA, Character::ADNE};
				changeScene(SceneType::CHARACTER_SELECTION);
			});
			buttons.push_back(button);
		}
	} else {
		Button button;
		button.setSize(buttonSize);
		button.setPos(Window::Center() + Point(-buttonSize.x / 2, 80));
		button.setFont(Font(14));
		button.setText(m_data->textAssets.at(L"start"));
		button.addOnClickedHandler([this] {
			SoundAsset(L"level-select").play();
			m_data->levelName = L"Tutorial";
			m_data->scriptPath = L"/501"; // L"levels/tutorial.script"
			m_data->accompanying = {Character::AQUA};
			m_data->showedTutorial = true;
			changeScene(SceneType::MAZE);
		});
		buttons.push_back(button);
	}
}

void TitleScene::update() {
	for (auto& button : buttons) {
		button.invokeHandlers();
	}
}

void TitleScene::draw() const {
	TextureAsset(L"background").draw();
	font(m_data->textAssets.at(L"title")).drawCenter(Window::Center() - Point(0, Window::Height() * 0.3));
	for (const auto& button : buttons) {
		button.draw();
	}
}

}
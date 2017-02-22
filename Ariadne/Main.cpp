#define NOMINMAX
#include <Windows.h>
#include <Siv3D.hpp>
#include <HamFramework.hpp>
#include "Common.h"
#include "Character.h"
#include "TitleScene.h"
#include "CharacterSelectionScene.h"
#include "MazeScene.h"
#include "ResultScene.h"

void Main() {
	using namespace Ariadne;

	CharacterAsset::load(L"/402"); // L"character-assets.json"
	TextureAsset::Register(L"background", L"/200"); // L"textures/background.png"
	TextureAsset::Register(L"wall", L"/203", TextureDesc::For3D); // L"textures/wall.jpg"
	TextureAsset::Register(L"ground", L"/202", TextureDesc::For3D); // L"textures/ground.jpg"
	TextureAsset::Register(L"particle", L"/204", TextureDesc::For3D); // L"textures/particle.png"
	for (const auto i : step(9)) {
		TextureAsset::Register(L"explosion" + ToString(i), L"/21" + ToString(i), TextureDesc::For3D); // L"textures/explosion/" + ToString(i) + L".png"
	}
	SoundAsset::Register(L"title-bgm", L"sounds/title-bgm.mp3");
	SoundAsset(L"title-bgm").setLoop(true);
	SoundAsset::Register(L"character-selection-bgm", L"sounds/character-selection-bgm.mp3");
	SoundAsset(L"character-selection-bgm").setLoop(true);
	SoundAsset::Register(L"select", L"sounds/button16.mp3");
	SoundAsset::Register(L"level-select", L"sounds/decision7.mp3");
	SoundAsset::Register(L"start", L"sounds/decision25.mp3");
	SoundAsset::Register(L"error", L"sounds/button56.mp3");
	SoundAsset::Register(L"cancel", L"sounds/cancel2.mp3");

	MyApp manager(SceneManagerOption::None);
	manager.add<TitleScene>(SceneType::TITLE);
	manager.add<CharacterSelectionScene>(SceneType::CHARACTER_SELECTION);
	manager.add<MazeScene>(SceneType::MAZE);
	manager.add<ResultScene>(SceneType::RESULT);

	Window::Resize(1280, 720);

	// disable IME
	System::Update();
	ImmAssociateContext(GetActiveWindow(), NULL);

	manager.init(SceneType::TITLE);

	while (System::Update()) {
		if (!manager.updateAndDraw()) {
			break;
		}
	}
}
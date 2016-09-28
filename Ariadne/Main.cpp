#include <Siv3D.hpp>
#include <HamFramework.hpp>
#include "Common.h"
#include "MazeScene.h"

void Main() {
	auto manager = MyApp();
	manager.add<MazeScene>(L"Game");

	while (System::Update()) {
		if (!manager.updateAndDraw()) {
			break;
		}
	}
}
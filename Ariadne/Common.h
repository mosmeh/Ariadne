#pragma once
#include <Siv3D.hpp>
#include <HamFramework.hpp>

struct GameData {

};

using MyApp = SceneManager<String, GameData>;

const double PLAYER_SPEED = 0.1;
const double WALL_THICKNESS = 0.5;
const double WALL_HEIGHT = 5.0;
const double WALL_HALF_LENGTH = 2.0;
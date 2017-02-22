#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>
#include <HamFramework.hpp>
#include "Character.h"

namespace Ariadne {

static const size_t MAX_ACCOMPANYING_CHARACTERS = 3;
static const FilePath INIT_SCRIPT = L"/403"; // L"init.script"
static const FilePath SCORES_CSV = L"scores.csv";

static const double EPS = 1e-4;

enum class Direction : size_t {
	NORTH = 0, SOUTH = 1, EAST = 2, WEST = 3
};

static const Point NEIGHBORHOOD[8] = {
	Point::Up,
	Point::Up + Point::Right,
	Point::Right,
	Point::Right + Point::Down,
	Point::Down,
	Point::Down + Point::Left,
	Point::Left,
	Point::Left + Point::Up
};

enum class SceneType {
	TITLE,
	CHARACTER_SELECTION,
	MAZE,
	RESULT
};

struct CommonData {
public:
	CommonData() {
		{
			JSONReader json(L"/401"); // L"texts.json"
			for (const auto& p : json.root().getObject()) {
				textAssets[p.first] = p.second.getString();
			}
			Window::SetTitle(textAssets.at(L"title"));
		}
		{
			JSONReader json(L"/400"); // L"levels.json"
			for (const auto& p : json.root().getArray()) {
				const auto name = p[L"name"].getString();
				const auto path = p[L"script"].getString();
				levels.emplace_back(name, path);
			}
		}

		if (FileSystem::Exists(SCORES_CSV)) {
			CSVReader csv(SCORES_CSV);
			for (const auto i : step(csv.rows)) {
				const auto name = csv.get<String>(i, 0);
				for (size_t j = 1; j < csv.columns(i); ++j) {
					scores[name].emplace_back(csv.get<long long>(i, j));
				}
			}
		}
	}

	bool isAccompanying(const Character character) const {
		return std::find(accompanying.begin(), accompanying.end(), character) != accompanying.end();
	}

	std::unordered_map<String, String> textAssets;

	std::vector<std::pair<String, FilePath> > levels;
	String levelName;
	FilePath scriptPath;

	std::unordered_map<String, std::vector<Milliseconds> > scores;

	bool showedTutorial = false;

	std::vector<Character> availableCharacters = {Character::AQUA, Character::AGLAIA, Character::ANASTASIA, Character::ASTREA, Character::ANEMONE, Character::ADNE};
	std::vector<Character> accompanying = {Character::AQUA, Character::AGLAIA, Character::ASTREA};
	Milliseconds elapsed;
};

using MyApp = SceneManager<SceneType, CommonData>;

class TextWrapper {
public:
	TextWrapper() = delete;

	static String wrap(const String& text, const Font& font, const size_t width) {
		String wrappedText = L"";
		size_t idx = 0;
		while (idx < text.length) {
			String newLine = L"";
			while (idx < text.length &&
				   static_cast<size_t>(font(newLine + text[idx]).region().w) < width) {
				newLine.append({text[idx++]});
			}
			wrappedText.append(newLine);
			wrappedText.append(L"\n");
		}

		return wrappedText;
	}
};

}
#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>

namespace Ariadne {

static const Color BACKGROUND_COLOR = Color(40, 50, 60);

enum class Character {
	AQUA,
	AGLAIA,
	ANASTASIA,
	ASTREA,
	ANEMONE,
	ADNE,
	NONE
};

static const size_t NUM_CHARACTERS = static_cast<size_t>(Character::NONE);
static const std::unordered_map<String, Character> NAME_TO_CHARACTER = {
	{L"aqua", Character::AQUA},
	{L"aglaia", Character::AGLAIA},
	{L"anastasia", Character::ANASTASIA},
	{L"astrea", Character::ASTREA},
	{L"anemone", Character::ANEMONE},
	{L"adne", Character::ADNE},
	{L"none", Character::NONE}
};

class CharacterParser {
public:
	CharacterParser() = delete;

	static Character parse(const String& name) {
		return NAME_TO_CHARACTER.at(name.lower());
	}
};

class CharacterAsset {
public:
	struct CharacterAssetSet {
		String displayName, description, confirm, invoking;
		Texture standing, face, checkedFace;
	};

	CharacterAsset(const CharacterAsset&) = delete;
    CharacterAsset& operator=(const CharacterAsset&) = delete;
    CharacterAsset(CharacterAsset&&) = delete;
    CharacterAsset& operator=(CharacterAsset&&) = delete;

	static void load(const FilePath& filePath) {
		CharacterAssetSet noneAsset;
		noneAsset.displayName = L"none";
		getInstance().assetSets[Character::NONE] = noneAsset;

		JSONReader json(filePath);
		for (const auto& p : NAME_TO_CHARACTER) {
			String name;
			Character character;
			std::tie(name, character) = p;

			if (json.root().contains(name)) {
				const auto& c = json[name];
				CharacterAssetSet assetSet;
				assetSet.displayName = c[L"displayName"].getString();
				assetSet.description = c[L"description"].getString();
				assetSet.confirm = c[L"confirm"].getString();
				assetSet.invoking = c[L"invoking"].getString();
				assetSet.standing = Texture(c[L"standingImage"].getString());

				const auto faceImg = Image(c[L"faceImage"].getString()).scaled(100, 100);
				assetSet.face = Texture(faceImg);

				Image img(100, 100, BACKGROUND_COLOR);
				faceImg.write(img);
				getInstance().checkImage.write(img);
				assetSet.checkedFace = Texture(img);

				getInstance().assetSets[character] = assetSet;
			}
		}
	}

	static const CharacterAssetSet& get(const Character character) {
        return getInstance().assetSets.at(character);
	}

private:
	CharacterAsset() : checkImage(Image(L"/201" /*L"textures/check.png"*/).scaled(100, 100)) {}
	~CharacterAsset() = default;

	std::unordered_map<Character, CharacterAssetSet> assetSets;
	Image checkImage;

	static CharacterAsset& getInstance() {
        static CharacterAsset instance;
		return instance;
	}
};

}
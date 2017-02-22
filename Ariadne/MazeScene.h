#pragma once
#include <Siv3D.hpp>
#include <queue>
#include "Common.h"
#include "Level.h"
#include "Character.h"
#include "Ability.h"
#include "Effect.h"
#include "Button.h"
#include "TextBox.h"
#include "InnerMessageBox.h"

namespace Ariadne {

const double PLAYER_SPEED = 0.15;
const double CAMERA_HEIGHT = 2.0;
const double MOUSE_SENSITIVITY = 0.003;
const double WALL_INTERATION_LIMIT_RADIUS = 10;
const double AGLAIA_ABILITY_DURATION_SEC = 30;

class Fake3DArrow {
public:
	Fake3DArrow(const double length_, const double thickness_, const Vec2& headSize_, const double shadow_, const double roll_) :
		length(length_),
		thickness(thickness_),
		headSize(headSize_),
		shadow(shadow_),
		roll(roll_) {

		setTheta(0);
	}

	void setPos(const Point& pos_) {
		pos = pos_;

		polygon.moveBy(pos - polygon.centroid());
	}

	void setTheta(const double theta_) {
		theta = theta_;

		polygon = Arrow(Vec2::Zero, length * Vec2::Up.rotated(theta), thickness, headSize).scaled(1.0, roll);
		setPos(pos);
	}

	void draw(const Color color = Palette::White, const Color shadowColor = Palette::Gray) const {
		for (int i = static_cast<int>(shadow * (1 - roll)); i >= 0; --i) {
			polygon.movedBy(0, i).draw(shadowColor);
		}
		polygon.draw(color);
	}

private:
	const double length, thickness, shadow, roll;
	const Vec2 headSize;
	s3d::Polygon polygon;
	Point pos;
	double theta;
};

class MazeScene : public MyApp::Scene {
public:
	MazeScene();
	~MazeScene();
	void init();
	void update();
	void draw() const;

private:
	struct SpotLight {
		Float3 position;
		float attenuation;
		Float3 diffuseColor;
		float exponent;
		Float3 direction;
		float cutoff;
	};

	enum class Mode {
		NORMAL,
		FREEZED,
		ABILITY_SELECTION,
		WALL_SELECTION
	};

	const VertexShader vertexShader;
	const PixelShader pixelShader;
	ConstantBuffer<SpotLight> spotLightCB;
	float spotLightMaxExponent = 32.0f;

	Stopwatch stopWatch;

	Level level;
	bool levelLoaded = false;
	Script script;
	Camera camera;
	Vec2 pos;
	double cameraTheta = 90_deg, cameraPhi = 0;
	int walkingFrames = 0;

	Mode mode = Mode::NORMAL;
	Character selectedCharacter = Character::NONE;

	bool textBoxWaitingInputs = false;
	bool cursorJumped = false;
	size_t numMouseHoveringButtons = 0;

	int lastEventId = BEGIN_ID;

	Effect effect, aglaiaAbilityEffect;
	std::unordered_map<Character, Ability> abilities;
	Array<Particle> particles;
	String particleTexture;
	unsigned int anastasiaCount = 1;
	Fake3DArrow arrow;

	bool drawMap = false;

	std::queue<Command> queuedCommands;
	std::unordered_map<size_t, std::pair<Rect, String> > displayedTextures;
	std::vector<String> playingSounds;
	TextBox textBox;
	String convName;
	std::vector<Button> characterButtons;
	Button guideButton, titleButton;
	InnerMessageBox msgBox;

	Optional<unsigned int> inputNumber() const;
	void invokeAbility(const Character character);
	void confirmAbilityInvocation(const Character character);
	void animateArrow(const Vec2& target);
	void loadMaze(const FilePath& mazeFile);
	void loadScript(const FilePath& scriptFile);
	void queueEventScript(const int id);
	void executeQueuedScript();
	void executeCommand(const Command& command);

	/*String modeToString(const Mode m) {
		switch (m) {
		case Mode::NORMAL:
			return L"NORMAL";
			break;
		case Mode::ABILITY_SELECTION:
			return L"ABILITY_SELECTION";
			break;
		case Mode::FREEZED:
			return L"FREEZED";
			break;
		case Mode::WALL_SELECTION:
			return L"WALL_SELECTION";
			break;
		default:
			return L"unknown";
			break;
		}
	}*/
};

}

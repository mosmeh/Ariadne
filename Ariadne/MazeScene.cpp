#include "MazeScene.h"

namespace Ariadne {

MazeScene::MazeScene() :
	vertexShader(VertexShader(L"/404")), // L"spotlight.vs"
	pixelShader(PixelShader(L"/405")), // L"spotlight.ps"
	arrow(200, 50, {100, 100}, 30, 0.5),
	textBox(Font(14), Font(16, Typeface::Bold), 200, 40, Palette::White, Color(0, 122, 204, 200), Padding(30, 200)),
	msgBox(Font(14), {20, 180}) {

	spotLightCB->attenuation = 0.01f;
	spotLightCB->diffuseColor = Float3(1, 0.9, 0.3);
	spotLightCB->cutoff = HalfPiF;

	abilities.emplace(Character::AQUA, Ability([this] {
		effect.add<LambdaEffect>([this](const double t) {
			const double explosionDuration = 0.5;
			if (t > explosionDuration) {
				particles.clear();
				level.erasePointingWall();
				mode = Mode::NORMAL;
				return false;
			}
			mode = Mode::FREEZED;
			if (particles.empty()) {
				level.resetWallColor();
				const auto p = CoordinateConverter::planarToSolid(pos, CAMERA_HEIGHT);
				const auto w = level.getPointingWall().getBox().center;
				particles.emplace_back(p + 0.6 * (w - p), 4.5);
			}
			particleTexture = L"explosion" + ToString(static_cast<int>(9 * t / explosionDuration));
			return true;
		});
	}, [this]() {
		if (level.pointingWall()) {
			const auto& wall = level.getPointingWall();
			if (!wall.isBreakable()) {
				return Ability::AssertionResult::CANNOT_INTERACT;
			} else if (wall.distanceFrom(camera.pos) > WALL_INTERATION_LIMIT_RADIUS) {
				return Ability::AssertionResult::TOO_FAR;
			} else {
				return Ability::AssertionResult::CAN_INVOKE;
			}
		} else {
			return Ability::AssertionResult::NOT_POINTING;
		}
	}));

	abilities.emplace(Character::AGLAIA, Ability([this] {
		aglaiaAbilityEffect.clear();
		aglaiaAbilityEffect.add<LambdaEffect>([this](const double t) {
			const double speed = aglaiaAbilityEffect.getSpeed();
			if (t < speed) {
				spotLightCB->exponent = static_cast<float>(spotLightMaxExponent * (speed - t));
			} else if (t < speed * AGLAIA_ABILITY_DURATION_SEC) {
				spotLightCB->exponent = static_cast<float>(spotLightMaxExponent * t / (speed * AGLAIA_ABILITY_DURATION_SEC));
			} else {
				return false;
			}
			return true;
		});
	}));

	{
		Ability ability([this] {
			auto& wall = level.getPointingWall();
			wall.addMarking(ToString(anastasiaCount++), wall.getPointingDirection());
		}, [this]() {
			if (level.pointingWall()) {
				const auto& wall = level.getPointingWall();
				if (!wall.isMarkable(wall.getPointingDirection())) {
					return Ability::AssertionResult::CANNOT_INTERACT;
				} else if (wall.distanceFrom(camera.pos) > WALL_INTERATION_LIMIT_RADIUS) {
					return Ability::AssertionResult::TOO_FAR;
				} else {
					return Ability::AssertionResult::CAN_INVOKE;
				}
			} else {
				return Ability::AssertionResult::NOT_POINTING;
			}
		});
		ability.setLimit(9);
		abilities.emplace(Character::ANASTASIA, ability);
	}

	abilities.emplace(Character::ASTREA, Ability([this] {
		animateArrow(CoordinateConverter::mapToPlanar(level.getGoalPos()));
	}));

	abilities.emplace(Character::ANEMONE, Ability([this] {
		const auto path = level.findShortestPathFrom(pos);
		if (path.has_value()) {
			const auto target = CoordinateConverter::mapToPlanar(path.value().front());
			const auto direction = CoordinateConverter::mirror(target - pos).normalized();
			const auto angle = std::acos(Vec2::Up.dot(direction));
			const auto currentPhi = cameraPhi;
			const auto targetPhi = -HalfPi - (direction.x < 0 ? angle : (2 * Pi - angle));
			effect.add<LambdaEffect>([this, currentPhi, targetPhi](const double t) {
				if (t > 1) {
					cameraPhi = targetPhi;
					mode = Mode::NORMAL;
					return false;
				}
				cameraPhi = currentPhi + (targetPhi - currentPhi) * Easing::Sine(t);
				mode = Mode::FREEZED;
				return true;
			});
		} else {
			notifyError();
		}
	}));

	{
		Ability ability([this] {
			drawMap = true;
		});
		ability.setLimit(1);
		abilities.emplace(Character::ADNE, ability);
	}

	arrow.setPos(Window::Center());
	msgBox.setBackgroundColor({0, 122, 204, 200});
};

MazeScene::~MazeScene() {
	System::SetExitEvent(WindowEvent::CloseButton | WindowEvent::EscapeKey);
	Cursor::SetStyle(CursorStyle::Default);
	for (const auto& s : playingSounds) {
		SoundAsset(s).stop();
	}
}

void MazeScene::init() {
	/*if (CommandLine::Get().size() > 1) {
		m_data->accompanying = m_data->availableCharacters;
	}*/

	System::SetExitEvent(WindowEvent::Manual);

	Graphics::SetBackground(Palette::Black);
	Graphics3D::SetLightForward(0, Light::None());
	Graphics3D::SetAmbientLightForward(Palette::Black);

	aglaiaAbilityEffect.setSpeed(10 / AGLAIA_ABILITY_DURATION_SEC);

	const Size buttonSize(200, 50);
	const Size characterButtonSize(100, 100);

	guideButton.setVisible(false);
	guideButton.setText(m_data->textAssets.at(L"guide"));
	guideButton.setPos(Window::Size() - Point((buttonSize.x + 5) * 2, buttonSize.y) - 50 * Point::One);
	guideButton.setSize(buttonSize);
	guideButton.setFont(Font(14));
	guideButton.addOnClickedHandler([this] {
		SoundAsset(L"select").play();
		msgBox.show(m_data->textAssets.at(L"guide_string"));
		mode = Mode::FREEZED;
	});

	titleButton.setVisible(false);
	titleButton.setText(m_data->textAssets.at(L"back_to_title"));
	titleButton.setPos(Window::Size() - buttonSize - 50 * Point::One);
	titleButton.setSize(buttonSize);
	titleButton.setFont(Font(14));
	titleButton.addOnClickedHandler([this] {
		SoundAsset(L"select").play();
		msgBox.show(m_data->textAssets.at(L"confirm_back_to_title"), InnerMessageBox::Style::YES_NO);
		mode = Mode::FREEZED;
	});

	int xPos = Window::Width() - static_cast<int>(m_data->accompanying.size()) * characterButtonSize.x;
	for (const auto character : m_data->accompanying) {
		Button button;
		button.setPos({xPos, 0});
		button.setSize(characterButtonSize);
		button.setFont(Font(20, Typeface::Bold));
		button.setTextColor(Palette::White);
		button.setTextAlign(Button::TextAlign::BOTTOM_RIGHT);
		button.setPadding({0, 5, 0});
		button.setTexture(CharacterAsset::get(character).face);
		button.addOnClickedHandler([this, character] {
			SoundAsset(L"select").play();
			if (mode == Mode::ABILITY_SELECTION) {
				selectedCharacter = character;
				if (character == Character::AQUA || character == Character::ANASTASIA) {
					mode = Mode::WALL_SELECTION;
					textBox.open(m_data->textAssets.at(L"select_wall"));
				} else {
					confirmAbilityInvocation(character);
				}
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
		xPos += characterButtonSize.x;
	}

	Cursor::SetStyle(CursorStyle::None);

	loadScript(INIT_SCRIPT);
	loadScript(m_data->scriptPath);
}

void MazeScene::update() {
	if (System::GetPreviousEvent() & WindowEvent::EscapeKey) {
		System::Exit();
	} else if (System::GetPreviousEvent() & WindowEvent::CloseButton) {
		if (MessageBox::Show(m_data->textAssets.at(L"confirm_close"), MessageBoxStyle::YesNo, 1) == MessageBoxCommand::Yes) {
			System::Exit();
		}
	}

	textBox.update();
	msgBox.update();

	if (Input::MouseR.released) {
		switch (mode) {
		case Mode::NORMAL:
			selectedCharacter = Character::NONE;
			textBoxWaitingInputs = false;
			mode = Mode::ABILITY_SELECTION;
			break;
		case Mode::WALL_SELECTION:
		case Mode::ABILITY_SELECTION:
			mode = Mode::NORMAL;
			break;
		}
	}
	const auto showButtons = mode == Mode::ABILITY_SELECTION && !textBox.isOpened();
	guideButton.setVisible(showButtons);
	titleButton.setVisible(showButtons);
	
	if (mode != Mode::ABILITY_SELECTION) {
		numMouseHoveringButtons = 0;
	}

	if (mode == Mode::FREEZED && msgBox.hasCommand()) {
		const auto command = msgBox.getCommand().value();
		switch (command) {
		case MessageBoxCommand::Yes:
			SoundAsset(L"select").play();
			if (selectedCharacter == Character::NONE) {
				changeScene(SceneType::TITLE);
			} else {
				invokeAbility(selectedCharacter);
			}
			break;
		case MessageBoxCommand::Ok:
			SoundAsset(L"select").play();
			mode = Mode::NORMAL;
			break;
		case MessageBoxCommand::No:
		default:
			SoundAsset(L"cancel").play();
			mode = Mode::NORMAL;
			break;
		}
		selectedCharacter = Character::NONE;
		msgBox.reset();
	}

	if (mode == Mode::NORMAL || mode == Mode::ABILITY_SELECTION) {
		const auto numKey = inputNumber();
		if (numKey.has_value()) {
			const auto n = numKey.value();
			if (n > 0 && n <= characterButtons.size()) {
				mode = Mode::ABILITY_SELECTION;
				characterButtons.at(n - 1).click();
			}
		}
	}

	if (mode == Mode::WALL_SELECTION) {
		if (Input::MouseL.clicked || Input::KeyEnter.clicked) {
			const auto result = abilities.at(selectedCharacter).getAssertionResult();
			if (result != Ability::AssertionResult::NOT_POINTING) {
				mode = Mode::FREEZED;
				Cursor::SetPos(Window::Center());
				cursorJumped = true;
			}
			switch (result) {
			case Ability::AssertionResult::CAN_INVOKE:
				SoundAsset(L"select").play();
				confirmAbilityInvocation(selectedCharacter);
				break;
			case Ability::AssertionResult::CANNOT_INTERACT:
				SoundAsset(L"error").play();
				msgBox.show(m_data->textAssets.at(L"cannot_interact"));
				break;
			case Ability::AssertionResult::TOO_FAR:
				SoundAsset(L"error").play();
				msgBox.show(m_data->textAssets.at(L"too_far"));
				break;
			}
		}
	}

	if (mode != Mode::FREEZED) {
		level.resetWallColor();
		if (Input::KeyBackspace.clicked) {
			mode = Mode::NORMAL;
		}
	}

	if (mode == Mode::FREEZED || mode == Mode::ABILITY_SELECTION) {
		Cursor::SetStyle(CursorStyle::Default);
	} else {
		Cursor::SetStyle(CursorStyle::None);

		if (cursorJumped) {
			cursorJumped = false;
		} else {
			const auto delta = MOUSE_SENSITIVITY * Mouse::DeltaF();
			cameraTheta += delta.y;
			cameraPhi -= delta.x;
		}

		cameraTheta += (Input::KeyDown.pressed - Input::KeyUp.pressed) * 3_deg;
		cameraPhi += (Input::KeyLeft.pressed - Input::KeyRight.pressed) * 3_deg;

		cameraTheta = std::min(179_deg, std::max(1_deg, cameraTheta));

		if (mode == Mode::WALL_SELECTION && level.pointingWall()) {
			auto& wall = level.getPointingWall();
			const auto ability = abilities.at(selectedCharacter);
			Color color;
			switch (ability.getAssertionResult()) {
			case Ability::AssertionResult::CAN_INVOKE:
				color = Palette::Aqua;
				break;
			case Ability::AssertionResult::CANNOT_INTERACT:
				color = Palette::Red;
				break;
			case Ability::AssertionResult::TOO_FAR:
				color = Palette::Forestgreen;
				break;
			}
			wall.setColor(color);
		}

		if (Window::GetState().cursorOnClient) {
			const double RADIUS = 100;
			if (Mouse::PosF().distanceFrom(Window::Center()) > RADIUS) {
				const auto pointingDirection = (Mouse::PosF() - Window::Center()).normalized();
				Cursor::SetPos((Window::Center() - RADIUS * pointingDirection).asPoint());
				cursorJumped = true;
			}
		}

		if (mode == Mode::NORMAL) {
			const auto tempPos = pos + PLAYER_SPEED * CoordinateConverter::mirror(((Input::KeyW.pressed - Input::KeyS.pressed) * Vec2::UnitX + (Input::KeyA.pressed - Input::KeyD.pressed) * Vec2::UnitY).normalized().rotated(cameraPhi));
			if (!level.intersects(tempPos)) {
				pos = tempPos;
			}
			if (Input::KeyW.pressed || Input::KeyA.pressed || Input::KeyS.pressed || Input::KeyD.pressed) {
				++walkingFrames;
			} else {
				walkingFrames = 0;
			}

			if (levelLoaded) {
				const auto id = level.getEventId(pos);
				if (id.has_value()) {
					if (id.value() != lastEventId) {
						if (id.value() == GOAL_ID) {
							stopWatch.pause();
						}
						queueEventScript(id.value());
						lastEventId = id.value();
					}
				}
			}
		}
	}

	camera.pos = CoordinateConverter::planarToSolid(pos, CAMERA_HEIGHT + 0.1 * sin(walkingFrames * 0.2));
	const auto direction = Vec3(Spherical(1, cameraTheta, cameraPhi));
	camera.lookat = camera.pos + direction;
	Graphics3D::SetCamera(camera);

	spotLightCB->position = camera.pos;
	spotLightCB->direction = direction;

	if (textBoxWaitingInputs) {
		if (Input::MouseL.clicked || Input::KeyEnter.clicked || Input::KeySpace.clicked) {
			if (textBox.cursorReachedEnd()) {
				textBoxWaitingInputs = false;
			} else {
				textBox.displayAllText();
			}
		}
	} else {
		executeQueuedScript();
		if (!textBoxWaitingInputs && queuedCommands.empty()
			&& mode != Mode::ABILITY_SELECTION && mode != Mode::WALL_SELECTION) {
			if (!effect.hasEffects()) {
				textBox.close();
			}
			switch (lastEventId) {
			case BEGIN_ID:
				stopWatch.start();
				break;
			case GOAL_ID:
				if (m_data->levelName.lower() == L"tutorial") {
					changeScene(SceneType::TITLE);
				} else {
					m_data->elapsed = stopWatch.elapsed();
					changeScene(SceneType::RESULT);
				}
				break;
			}
		}
	}

	guideButton.invokeHandlers();
	titleButton.invokeHandlers();

	const auto& characters = m_data->accompanying;
	for (const auto i : step(characters.size())) {
		auto& button = characterButtons.at(i);
		const auto limit = abilities.at(characters.at(i)).getLimit();
		button.setText(limit >= 0 ? ToString(limit) : L"Åá");
		button.setEnabled(limit != 0 && mode != Mode::FREEZED);
		button.invokeHandlers();
	}

	effect.update();
	aglaiaAbilityEffect.update();
	level.update();
}

void MazeScene::draw() const {
	Graphics3D::SetConstantForward(ShaderStage::Pixel, 1, spotLightCB);
	Graphics3D::BeginVSForward(vertexShader);
	Graphics3D::BeginPSForward(pixelShader);

	level.drawForward();
	if (!particles.empty()) {
		Graphics3D::DrawParticlesForward(particles, TextureAsset(particleTexture));
	}

	Graphics3D::EndVSForward();
	Graphics3D::EndPSForward();

	if (drawMap) {
		level.drawLocalMap(pos);
	}
	guideButton.draw();
	titleButton.draw();
	for (const auto& b : characterButtons) {
		b.draw();
	}
	for (const auto& tq : displayedTextures) {
		Rect rect;
		String name;
		std::tie(rect, name) = tq.second;
		rect(TextureAsset(name)).draw();
	}
	textBox.draw();
	msgBox.draw();
}

Optional<unsigned int> MazeScene::inputNumber() const {
	if (Input::AnyKeyClicked()) {
#define KEY(n) if (Input::Key##n.clicked) return n;
		KEY(0);
		KEY(1);
		KEY(2);
		KEY(3);
		KEY(4);
		KEY(5);
		KEY(6);
		KEY(7);
		KEY(8);
		KEY(9);
#undef KEY
	}

	return none;
}

void MazeScene::invokeAbility(const Character character) {
	for (auto& button : characterButtons) {
		button.setVisible(false);
	}
	textBox.open(CharacterAsset::get(character).invoking);

	const auto center = Window::Center();
	const double ease = 0.5, stay = 0.5;
	effect.add<CutIn>(CharacterAsset::get(character).standing.scale(0.7),
					  ease, stay, center + Point(300, 0), center, center - Point(300, 0));
	const double cutInDuration = stay + ease * 2;
	effect.add<DelayedEffect>(cutInDuration, [this, character]() {
		abilities.at(character).invoke();
		mode = Mode::NORMAL;
		textBoxWaitingInputs = false;
		for (auto& button : characterButtons) {
			button.setVisible(true);
		}
	});
}

void MazeScene::confirmAbilityInvocation(const Character character) {
	const auto& asset = CharacterAsset::get(character);
	msgBox.setIcon(asset.face);
	msgBox.show(asset.confirm, InnerMessageBox::Style::YES_NO);
	mode = Mode::FREEZED;
	cursorJumped = true;
	Cursor::SetPos(Window::Center());
}

void MazeScene::animateArrow(const Vec2& target) {
	const double INIT_SPEED = 0.3;
	const int ROTATIONS = 3;
	const double ROTATION_DURATION = 1.5, STAY_DURATION = 0.5;

	const auto direction = CoordinateConverter::mirror(target - pos).normalized();
	const auto angle = std::acos(Vec2::Up.dot(direction));
	const auto theta = HalfPi + cameraPhi + (direction.x < 0 ? angle : (2 * Pi - angle));

	const double phase = theta - 2 / Pi * ROTATION_DURATION * INIT_SPEED;
	const double correctedSpeed = theta + 2 * Pi * ROTATIONS - phase;
	effect.add<LambdaEffect>([this, ROTATION_DURATION, STAY_DURATION, correctedSpeed, phase](const double t) {
		if (t < ROTATION_DURATION) {
			arrow.setTheta(correctedSpeed * std::sin(HalfPi * t / ROTATION_DURATION) + phase);
		}
		if (t < ROTATION_DURATION + STAY_DURATION) {
			arrow.draw();
			mode = Mode::FREEZED;
			return true;
		}
		mode = Mode::NORMAL;
		return false;
	});
}

void MazeScene::loadMaze(const FilePath& mazeFile) {
	level = Level(mazeFile);
	pos = CoordinateConverter::mapToPlanar(level.getStartPos());
	spotLightCB->exponent = spotLightMaxExponent;
	spotLightCB->cutoff = HalfPiF;

	levelLoaded = true;
}

void MazeScene::loadScript(const FilePath& scriptFile) {
	script = ScriptParser::parse(scriptFile).value();
	for (const auto& c : script.at(BEGIN_ID)) {
		queuedCommands.push(c);
	}
}

void MazeScene::queueEventScript(const int id) {
	std::queue<Command> queue;
	queuedCommands.swap(queue);

	if (script.find(id) != script.end()) {
		for (const auto& c : script.at(id)) {
			queuedCommands.push(c);
		}
	}
	textBoxWaitingInputs = false;
}

void MazeScene::executeQueuedScript() {
	while (!queuedCommands.empty()) {
		const auto cmd = queuedCommands.front();
		queuedCommands.pop();
		executeCommand(cmd);
		if (cmd.type == CommandType::MESSAGE) {
			break;
		}
	}
}

void MazeScene::executeCommand(const Command& command) {
	const auto& args = command.arguments;
	switch (command.type) {
	case CommandType::SOURCE:
		loadScript(args.front());
		break;
	case CommandType::LOAD_MAZE:
		loadMaze(args.front());
		break;
	case CommandType::REGISTER_TEXTURE:
	{
		const String& name = args[0];
		const String& path = args[1];
		const String& desc = args[2];
		if (TextureAsset::IsRegistered(name)) {
			TextureAsset::Unregister(name);
		}
		if (desc == L"3d") {
			TextureAsset::Register(name, path, TextureDesc::For3D);
		} else {
			TextureAsset::Register(name, path);
		}
		break;
	}
	case CommandType::UNREGISTER_TEXTURE:
	{
		const String& name = args.front();
		if (name.lower() == L"all") {
			TextureAsset::UnregisterAll();
		} else {
			TextureAsset::Unregister(name);
		}
		break;
	}
	case CommandType::REGISTER_SOUND:
	{
		const String& name = args[0];
		const String& path = args[1];
		if (SoundAsset::IsRegistered(name)) {
			SoundAsset::Unregister(name);
		}
		SoundAsset::Register(name, path);
		break;
	}
	case CommandType::UNREGISTER_SOUND:
	{
		const String& name = args.front();
		if (name.lower() == L"all") {
			SoundAsset::UnregisterAll();
		} else {
			SoundAsset::Unregister(name);
		}
		break;
	}
	case CommandType::PLAY_SOUND:
	{
		const String& name = args[0];
		const String& loop = args[1].lower();
		SoundAsset(name).setLoop(loop == L"loop");
		SoundAsset(name).play();
		playingSounds.emplace_back(name);
		break;
	}
	case CommandType::STOP_SOUND:
		SoundAsset(args.front()).stop();
		break;
	case CommandType::DIRECT:
	{
		int d = -1;
		const String direction = args.front().lower();
		if (direction == L"west") {
			d = 0;
		} else if (direction == L"south") {
			d = 1;
		} else if (direction == L"east") {
			d = 2;
		} else if (direction == L"north") {
			d = 3;
		}
		cameraPhi = d * HalfPi;

		break;
	}
	case CommandType::FREEZE:
		mode = Mode::FREEZED;
		break;
	case CommandType::UNFREEZE:
		mode = Mode::NORMAL;
		break;
	case CommandType::MESSAGE:
		textBoxWaitingInputs = true;
		textBox.open(args.front(), convName);
		break;
	case CommandType::NAME:
	{
		const String& name = args.front();
		if (name.lower() == L"clear") {
			convName = L"";
		} else {
			convName = name;
		}
		break;
	}
	case CommandType::IMAGE:
	{
		const size_t id = FromString<size_t>(args[0]);
		const String& name = args[1];
		const auto& tex = TextureAsset(name);

		Size size;
		if (args.size() - 1 == 6) {
			// width and height is specified
			const Vec2 r(FromString<double>(args[4]), FromString<double>(args[5]));
			size = (r * Window::Size()).asPoint();
		} else if (args.size() - 1 == 5) {
			// width is specified
			const auto width = FromString<double>(args[4]) * Window::Width();
			size = (tex.size * width / tex.width).asPoint();
		} else if (args.size() - 1 == 4) {
			// position is specified but size is not
			size = tex.size;
		} else {
			// background image
			size = Window::Size();
		}

		Rect rect(size);
		if (args.size() - 1 >= 4) {
			// position is specified
			const Vec2 r(FromString<double>(args[2]), FromString<double>(args[3]));
			rect.setCenter((r * Window::Size()).asPoint());
		} else {
			// background image
			rect.pos = {0, 0};
		}
		displayedTextures[id] = {rect, name};
		break;
	}
	case CommandType::HIDE_IMAGE:
	{
		if (args.front().lower() == L"all") {
			displayedTextures.clear();
		} else {
			const size_t id = FromString<size_t>(args[0]);
			displayedTextures.erase(id);
		}
		break;
	}
	case CommandType::MESSAGE_BOX:
	{
		msgBox.show(args.front());
		mode = Mode::FREEZED;
		break;
	}
	case CommandType::MEET:
	{
		const Character character = CharacterParser::parse(args[0]);
		m_data->availableCharacters.push_back(character);
		break;
	}
	case CommandType::LIMIT:
	{
		const Character character = CharacterParser::parse(args[0]);
		const auto limit = FromString<int>(args[1]);
		abilities.at(character).setLimit(limit);
		break;
	}
	case CommandType::DARK:
		spotLightMaxExponent = FromString<float>(args.front());
		spotLightCB->exponent = spotLightMaxExponent;
		break;
	case CommandType::BLACKOUT:
		spotLightCB->cutoff = 0;
		mode = Mode::FREEZED;
		break;
	case CommandType::MOVE:
	{
		const int x = FromString<int>(args[0]);
		const int y = FromString<int>(args[1]);
		pos = CoordinateConverter::mapToPlanar({x, y});
		break;
	}
	}
}

}
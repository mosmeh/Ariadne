#pragma once
#define NO_MATH_USING
#include <Siv3D.hpp>

namespace Ariadne {

static const int BEGIN_ID = -1;
static const int GOAL_ID = -2;

enum class CommandType {
	UNKNOWN,
	SOURCE,				// source SCRIPT
	LOAD_MAZE,			// loadmaze MAZE
	REGISTER_TEXTURE,	// regtex NAME PATH [3d]
	UNREGISTER_TEXTURE, // unregtex NAME
	REGISTER_SOUND,		// regosound NAME PATH
	UNREGISTER_SOUND,	// unregsound NAME
	PLAY_SOUND,			// playsound NAME [loop]
	STOP_SOUND,			// stopsound NAME
	DIRECT,				// direct {NORTH, EAST, SOUTH, WEST}
	FREEZE,				// freeze
	UNFREEZE,			// unfreeze
	MESSAGE,			// msg TEXT
	NAME,				// name {NAME, CLEAR}
	IMAGE,				// img ID NAME [x] [y] [width] [height]
	HIDE_IMAGE,			// hideimg {ID, ALL}
	MESSAGE_BOX,		// msgbox TEXT [{ok, yesno}]
	MEET,				// meet CHARACTER
	LIMIT,				// limit LIMIT>=-1
	DARK,				// dark DARKNESS>=0
	BLACKOUT,			// blackout
	MOVE				// move X Y
};

struct Command {
	Command(const CommandType type_, const std::vector<String>& arguments_) :
		type(type_),
		arguments(arguments_) {}

	Command(const CommandType type_) : type(type_) {}

	CommandType type;
	std::vector<String> arguments;
};

using Script = std::unordered_map<int, std::vector<Command> >;

class ScriptParser {
public:
	ScriptParser() = delete;

	static Optional<Script> parse(const FilePath& filePath);

private:
	static std::pair<CommandType, size_t> stringToCommandTypeAndNumArguments(const String& commandName);
};

}
#include "Script.h"

namespace Ariadne {

Optional<Ariadne::Script> ScriptParser::parse(const FilePath& filePath) {
	TextReader reader(filePath);

	Script result;

	int id;
	std::vector<Command> commands;

	String line;
	while (reader.readLine(line)) {
		const auto hashPos = line.indexOf(L'#');
		if (hashPos != String::npos) {
			line = line.erase(hashPos, String::npos);
		}

		const auto tokens = line.trim().split(L' ');
		if (tokens.size() > 0) {
			if (tokens.front().front() == L'@') {
				if (tokens.size() != 1) {
					return none;
				}

				if (!commands.empty()) {
					result[id] = commands;
					commands.clear();
					commands.shrink_to_fit();
				}

				const String eventName = tokens.front().substr(1, String::npos).lower();
				if (eventName == L"begin") {
					id = BEGIN_ID;
				} else if (eventName == L"goal") {
					id = GOAL_ID;
				} else {
					id = FromString<int>(eventName);
				}
			} else {
				CommandType type;
				size_t numArgs;
				std::tie(type, numArgs) = stringToCommandTypeAndNumArguments(tokens.front());
				if (type == CommandType::UNKNOWN) {
					return none;
				}

				std::vector<String> args;
				for (size_t i = 1; i < std::min(tokens.size(), numArgs); ++i) {
					args.push_back(tokens.at(i));
				}
				String lastArg = L"";
				for (size_t i = numArgs; i < tokens.size(); ++i) {
					lastArg.append(tokens.at(i));
					if (i < tokens.size() - 1) {
						lastArg.append(L" ");
					}
				}
				if (type == CommandType::MESSAGE || type == CommandType::MESSAGE_BOX) {
					args.push_back(lastArg.replace(L"\\n", L"\n"));
				} else {
					args.push_back(lastArg);
				}

				commands.emplace_back(type, args);
			}
		}
	}
	result[id] = commands;

	return Optional<Ariadne::Script>(result);
}

std::pair<CommandType, size_t> ScriptParser::stringToCommandTypeAndNumArguments(const String& commandName) {
	const String loweredName = commandName.lower();
	if (loweredName == L"source") {
		return {CommandType::SOURCE, 1};
	} else if (loweredName == L"loadmaze") {
		return {CommandType::LOAD_MAZE, 1};
	} else if (loweredName == L"regtex" || loweredName == L"registertexture") {
		return {CommandType::REGISTER_TEXTURE, 3};
	} else if (loweredName == L"unregtex" || loweredName == L"unregistertexture") {
		return {CommandType::UNREGISTER_TEXTURE, 1};
	} else if (loweredName == L"regsound" || loweredName == L"registersound") {
		return {CommandType::REGISTER_SOUND, 2};
	} else if (loweredName == L"unregsound" || loweredName == L"unregistersound") {
		return {CommandType::UNREGISTER_SOUND, 1};
	} else if (loweredName == L"playsound") {
		return {CommandType::PLAY_SOUND, 2};
	} else if (loweredName == L"stopsound") {
		return {CommandType::STOP_SOUND, 1};
	} else if (loweredName == L"direct") {
		return {CommandType::DIRECT, 1};
	} else if (loweredName == L"freeze") {
		return {CommandType::FREEZE, 0};
	} else if (loweredName == L"unfreeze") {
		return {CommandType::UNFREEZE, 0};
	} else if (loweredName == L"msg" || loweredName == L"message") {
		return {CommandType::MESSAGE, 1};
	} else if (loweredName == L"name") {
		return {CommandType::NAME, 1};
	} else if (loweredName == L"img" || loweredName == L"image") {
		return {CommandType::IMAGE, 6};
	} else if (loweredName == L"hideimg" || loweredName == L"hideimage") {
		return {CommandType::HIDE_IMAGE, 1};
	} else if (loweredName == L"msgbox" || loweredName == L"messagebox") {
		return {CommandType::MESSAGE_BOX, 1};
	} else if (loweredName == L"meet") {
		return {CommandType::MEET, 1};
	} else if (loweredName == L"limit") {
		return {CommandType::LIMIT, 2};
	} else if (loweredName == L"dark") {
		return {CommandType::DARK, 1};
	} else if (loweredName == L"blackout") {
		return {CommandType::BLACKOUT, 0};
	} else if (loweredName == L"move") {
		return {CommandType::MOVE, 2};
	}

	return {CommandType::UNKNOWN, 0};
}

}

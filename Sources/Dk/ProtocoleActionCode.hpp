#ifndef PROTOCOLE_ACTION_CODE_H
#define PROTOCOLE_ACTION_CODE_H

#include <string.h>

namespace Protocole {
	static const size_t BIN_PING = 1346981447;
	static const size_t BIN_INFO = 1229866575;
	static const size_t BIN_GAZO = 1195465295; 
	static const size_t BIN_MCMD = 1296256324;
	static const size_t BIN_SIZE = 1397316165;
	static const size_t BIN_READ = 1380270404;
	static const size_t BIN_QUIT = 1364543828;
	
	static const std::string CMD_CHANNEL = "CHANNEL";
	static const std::string CMD_HEIGHT = "HEIGHT";
	static const std::string CMD_WIDTH = "WIDTH";
	static const std::string CMD_FPS = "FPS";
}

#endif
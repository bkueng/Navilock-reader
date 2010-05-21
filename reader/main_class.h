/*
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef MAIN_CLASS_H_
#define MAIN_CLASS_H_

#include "global.h"

#include <map>
#include <queue>


enum EParseState {
	Parse_not_done=0,
	Parse_print_help,
	Parse_unknown_command,
	Parse_print_version,
	Parse_ok
	
};

enum ETask {
	Task_get_tracks=0,
	Task_print_track_info,
	Task_delete_tracks,
	Task_set_distance,
	Task_read_addr
};
#define TASK_COUNT 5

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CMain
 * main class with the main task and command line parser
/*////////////////////////////////////////////////////////////////////////////////////////////////


class CMain {
public:
	CMain();
	~CMain();
	
	/* parse the command line parameters */
	void init(int argc, char *argv[]);
	
	void exec();
	
private:
	void parseCommandLine(int argc, char *argv[]);
	void printHelp();
	void wrongUsage(const char* fmt, ...);
	void printVersion();
	
	bool pushTask(ETask task);
	
	void processArgs();
	
	EParseState m_parse_state;
	map<string, string> m_arg_variables;
	queue<string> m_formats;
	bool m_tasks[TASK_COUNT];
};



#endif /* MAIN_CLASS_H_ */

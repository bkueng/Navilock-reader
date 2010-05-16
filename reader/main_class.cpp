/*
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version 2
 *	as published by the Free Software Foundation.
 */

#include "main_class.h"
#include "serial.h"
#include "navilock.h"
#include "persistence.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CMain
/*////////////////////////////////////////////////////////////////////////////////////////////////


CMain::CMain() : m_parse_state(Parse_not_done) {
	memset(m_tasks, 0, sizeof(m_tasks));
}

CMain::~CMain() {
}



void CMain::init(int argc, char *argv[]) {
	parseCommandLine(argc, argv);
	
}

void CMain::parseCommandLine(int argc, char *argv[]) {
	vector<string> args;
	for(int i=0; i<argc; ++i) args.push_back(argv[i]);
	args.push_back(""); //to avoid buffer overflow
	
	m_arg_variables.clear();
	
	for(int i=1; i<argc; ++i) {
		string& arg=args[i];
		if(arg=="--help" || arg=="-h") {
			m_parse_state=Parse_print_help;
		} else if(arg=="--get-tracks" || arg=="-t") {
			pushTask(Task_get_tracks);
		} else if(arg=="--info" || arg=="-i") {
			pushTask(Task_print_track_info);			
		} else if(arg=="--device" || arg=="-d") {
			m_arg_variables["device"]=args[i+1];
			++i;
		} else if(arg=="--output" || arg=="-o") {
			m_arg_variables["output"]=args[i+1];
			++i;
		} else if(arg=="--format" || arg=="-f") {
			m_arg_variables["format"]=toLower(args[i+1]);
			++i;
		} else if(arg=="--verbose" || arg=="-v") {
			m_arg_variables["verbose"]="1";
		} else {
			m_parse_state=Parse_unknown_command;
			m_arg_variables["unknown_command"]=arg;
			i=argc; //abort
		}
	}
}

bool CMain::pushTask(ETask task) {
	if(m_parse_state==Parse_not_done|| m_parse_state==Parse_ok) {
		m_parse_state=Parse_ok;
		m_tasks[task]=true;
		return(true);
	}
	return(false);
}


void CMain::exec() {
	switch(m_parse_state) {
	case Parse_not_done:
	case Parse_print_help:
		printHelp();
		break;
	case Parse_unknown_command:
		wrongUsage("Unknown command: %s", m_arg_variables["unknown_command"].c_str());
		break;
	case Parse_ok:
		processArgs();
		break;
	}
}

void CMain::printHelp() {
	printf("Usage:\n"
		" "APP_NAME" [-v] -d <device> [-t [-o <path>] [-f <format>]] [-i]\n"
		"  -d, --device <device>           set the device to read from/write to\n"
		"                                  <device>: e.g. /dev/ttyUSB1\n"
		"  -t, --get-tracks                read the tracks and save them to file\n"
		"  -o, --output                    output path for writing tracks\n"
		"                                  each track will be saved to a file\n"
		"                                  in the form \'trace_YYYY-MM-DD-HH.MM.SS.gpx\'\n"
		"                                  if not set, stdout will be used\n"
		"  -f, --format <format>           file output format\n"
		"                                  supported are gpx and txt\n"
		"                                  default is txt\n"
		"  -i, --info                      print track information on device\n"
		"  -v, --verbose                   print debug messages\n"
		"  -h, --help                      print this message\n"
		);
}

void CMain::wrongUsage(const char* fmt, ...) {
	
	printHelp();
	
	printf("\n ");
	
	va_list args;
	va_start (args, fmt);
	vprintf(fmt, args);
	va_end (args);
	
	printf("\n");
	
}


void CMain::processArgs() {
	
	if(m_arg_variables["verbose"]=="1") CLog::getInstance().setConsoleLevel(DEBUG);
	
	
	string& device=m_arg_variables["device"];
	if(device=="") {
		//todo: can we make this optional -> try to find appropriate device
		
		wrongUsage("No device submitted");
		return;
	}
	
	//open the device
	CSerial serial;
	try {
		serial.open(device.c_str());
		serial.initConnection(B115200);
	} catch(const Exception& e) {
		LOG(ERROR, "Failed to open the serial device %s", device.c_str());
		return;
	}
	
	LOG(DEBUG, "Serial device %s opened", device.c_str());
	
	CNavilock navilock(serial);
	
	
	if(m_tasks[Task_print_track_info]) {
		navilock.readTrackInfos();
		printf("Found %u Tracks\n", navilock.tracks().size());
		
		if(navilock.tracks().size()>0) {
			printf(" #    Points       POI      Addr          Start         Duration\n");
			printf("----------------------------------------------------------------------\n");
		}
		for(size_t i=0; i<navilock.tracks().size(); ++i) {
			const ETrack& track=navilock.tracks()[i];
			printf("%2u  %8u  %8u  %8u   %s %s  %s\n"
					, i, track.point_count-(uint)track.poi_count, (uint)track.poi_count, track.start_addr, track.start_date.toStr().c_str(),
					track.start_time.toStr().c_str(),
					deltaTimeToStr(getDeltaTimeSec(track.start_date, track.start_time,
							track.end_date, track.end_time)).c_str());
			
		}
	}
	
	if(m_tasks[Task_get_tracks]) {
		CPersistence* persistence=NULL;
		string file_ext="";
		if(m_arg_variables["format"]=="txt") {
			file_ext=".txt";
			persistence=new CPersistenceTxt();
		} else if(m_arg_variables["format"]=="gravity" || m_arg_variables["format"]=="grav") {
			file_ext=".grav";
			persistence=new CPersistenceGravity();
		} else { //gpx
			file_ext=".gpx";
			persistence=new CPersistenceGpx();
		}
		
		navilock.readTracks();
		string& folder=m_arg_variables["output"];
		if(folder.length()>0 && folder[folder.length()-1]!=FOLDER_SEPARATOR) folder+=FOLDER_SEPARATOR;
		
		FILE* hFile=stdout;
		
		for(size_t i=0; i<navilock.tracks().size(); ++i) {
			const ETrack& track=navilock.tracks()[i];
			if(track.bGot_info) {
				if(folder.length()>0) {
					string file=folder+"trace_"+track.start_date.toStr("%04u-%02i-%02i")
							+"-"+track.start_time.toStr("%02i.%02i.%02i")+file_ext;
					hFile=fopen(file.c_str(), "w");
					ASSERT_THROW_e(hFile, EFILE_ERROR, "Failed to open the file %s", file.c_str());
				}
				persistence->write(hFile, track);
				if(folder.length()>0) {
					fclose(hFile);
				}
			}
		}
		
		delete(persistence);
	}
	
}











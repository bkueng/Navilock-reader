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

#include "main_class.h"
#include "serial.h"
#include "navilock.h"
#include "persistence.h"
#include "version.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CMain
/*////////////////////////////////////////////////////////////////////////////////////////////////


CMain::CMain() :m_parameters(NULL), m_cl_parse_result(Parse_none_found) {
	
}

CMain::~CMain() {
	SAVE_DEL(m_parameters);
}



void CMain::init(int argc, char *argv[]) {
	parseCommandLine(argc, argv);
	
}

void CMain::parseCommandLine(int argc, char *argv[]) {
	
	SAVE_DEL(m_parameters);
	m_parameters=new CCommandLineParser(argc, argv);
	
	//init known arguments
	m_parameters->addSwitch("help", 'h');
	m_parameters->addSwitch("version");
	m_parameters->addSwitch("verbose", 'v');
	m_parameters->addParam("device", 'd');
	m_parameters->addParam("set-distance");
	
	m_parameters->addTask("get-tracks", 't');
	m_parameters->addParam("output", 'o', "", "get-tracks");
	m_parameters->addSwitch("new-only", 'n', "get-tracks");
	m_parameters->addParam("format", 'f', "txt", "get-tracks");
	
	m_parameters->addTask("reset", 'r');
	
	m_parameters->addTask("info", 'i');
	
	m_parameters->addTask("read-address", 'a');
	m_parameters->addParam("offset", 'o', "0", "read-address");
	m_parameters->addParam("size", 's', "0", "read-address");
	
	m_cl_parse_result=m_parameters->parse();
	
}

void CMain::printHelp() {
	printf("Usage:\n"
		" "APP_NAME" [-v] -d <device> [-t [-o <path> [-n]] [-f <format>]] [-i]\n"
		" "APP_NAME" [-v] -d <device> [--set-distance <distance>] [-r]\n"
		" "APP_NAME" [-v] -d <device> [-a -o <offset> -s <size>]\n"
		" "APP_NAME" --version\n"
		"  -d, --device <device>           set the device to read from/write to\n"
		"                                  <device>: e.g. /dev/ttyUSB1\n"
		"  -t, --get-tracks                read the tracks and save them to file\n"
		"    -o, --output                  output path for writing tracks\n"
		"                                  each track will be saved to a file\n"
		"                                  in the form \'trace_YYYY-MM-DD-HH.MM.SS.gpx\'\n"
		"                                  if not set, stdout will be used\n"
		"    -n, --new-only                read only tracks that don't already exist in\n"
		"                                  the output folder\n"
		"    -f, --format <format>         file output format\n"
		"                                  supported are gpx and txt\n"
		"                                  default is txt\n"
		"  -r, --reset                     delete all tracks\n"
		"  --set-distance <distance>       set the total km count to <distance>\n"
		"  -a, --read-address              read flash memory and output hex values\n"
		"    -o, --offset                  address offset\n"
		"    -s, --size                    size to read in bytes\n"
		"  -i, --info                      print track information on device\n"
		"  -v, --verbose                   print debug messages\n"
		"  -h, --help                      print this message\n"
		"  --version                       print the version\n"
		);
}


void CMain::exec() {
	
	ASSERT_THROW(m_parameters, ENOT_INITIALIZED);
	
	switch(m_cl_parse_result) {
	case Parse_none_found:
		printHelp();
		break;
	case Parse_unknown_command:
		wrongUsage("Unknown command: %s", m_parameters->getUnknownCommand().c_str());
		break;
	case Parse_success:
		if(m_parameters->getSwitch("help")) {
			printHelp();
		} else if(m_parameters->getSwitch("version")) {
			printVersion();
		} else {
			processArgs();
		}
		break;
	}
}

void CMain::printVersion() {
	printf("%s\n", getAppVersion().toStr().c_str());
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
	
	if(m_parameters->getSwitch("verbose")) CLog::getInstance().setConsoleLevel(DEBUG);
	
	
	string device;
	m_parameters->getParam("device", device);
	if(device.length()==0) {
		//todo: can we make this optional -> try to find appropriate device
		
		wrongUsage("No device submitted");
		return;
	}
	
	//open the device
	CSerial serial;
	serial.open(device.c_str());
	serial.initConnection(B115200);
	
	LOG(DEBUG, "Serial device %s opened", device.c_str());
	
	CNavilock navilock(serial);
	
	if(m_parameters->setTask("info")->bGiven) {
		navilock.readTrackInfos();
		printf("Found %u Tracks on device %s\n", (uint)navilock.tracks().size(), device.c_str());
		
		if(navilock.tracks().size()>0) {
			printf(" #    Points       POI      Addr          Start         Duration\n");
			printf("----------------------------------------------------------------------\n");
		}
		for(size_t i=0; i<navilock.tracks().size(); ++i) {
			const ETrack& track=navilock.tracks()[i];
			printf("%2u  %8u  %8u  %8u   %s %s  %s\n"
					, (uint)i, track.point_count-(uint)track.poi_count, (uint)track.poi_count, track.start_addr, track.start_date.toStr().c_str(),
					track.start_time.toStr().c_str(),
					deltaTimeToStr(getDeltaTimeSec(track.start_date, track.start_time,
							track.end_date, track.end_time)).c_str());
			
		}
		
		printf("Total distance: %.1lfkm\n", navilock.totalDistance());
		
		uint last_addr=POINT_START_ADDR, last_point_count=0;
		if(navilock.tracks().size()>0) {
			const ETrack& last_track=*(navilock.tracks().end()-1);
			last_addr=last_track.start_addr;
			last_point_count=last_track.point_count;
		}
		uint tot_points=(DEVICE_MEM_SIZE-POINT_START_ADDR)/POINT_DATA_LEN;
		uint point_free_count=(DEVICE_MEM_SIZE-last_addr)/POINT_DATA_LEN-last_point_count;
		printf("Free memory: %.1f%% = %u points (~ %s)\n", (float)point_free_count/(float)tot_points*100.0f,
				point_free_count, deltaTimeToStr(((TIME_BETWEEN_POINTS*point_free_count)/60)*60).c_str());
		
	}
	
	if(m_parameters->setTask("get-tracks")->bGiven) {
		CPersistence* persistence;
		string file_ext="";
		string folder="";
		m_parameters->getParam("output", folder);
		if(folder.length()>0 && folder.substr(folder.length()-1,1)!=PATH_SEP) folder+=PATH_SEP;
		bool bNew_only=m_parameters->getSwitch("new-only");
		
		navilock.readTrackInfos();
		string format;
		m_parameters->getParam("format", format);
		
		do {
			toLower(format);
			if(format=="txt") {
				file_ext=".txt";
				persistence=new CPersistenceTxt();
			} else if(format=="gravity" || format=="grav") {
				file_ext=".grav";
				persistence=new CPersistenceGravity();
			} else if(format=="gpx") {
				file_ext=".gpx";
				persistence=new CPersistenceGpx();
			} else {
				persistence=NULL;
				LOG(ERROR, "Unknown output format: %s", format.c_str());
			}
			
			if(persistence) {
				FILE* hFile=stdout;
				
				for(size_t i=0; i<navilock.tracks().size(); ++i) {
					const ETrack& track=navilock.tracks()[i];
					string date_time=track.start_date.toStr("%04u-%02i-%02i")
									+"-"+track.start_time.toStr("%02i.%02i.%02i");
					if(folder.length()==0 || !bNew_only || (bNew_only && findFile(folder, date_time, file_ext)=="")) {
						navilock.readTrack(i);
						
						if(track.bGot_info) {
							if(folder.length()>0) {
								string file=folder+"trace_"+date_time+file_ext;
								hFile=fopen(file.c_str(), "w");
								ASSERT_THROW_e(hFile, EFILE_ERROR, "Failed to open the file %s", file.c_str());
							}
							persistence->write(hFile, track);
							if(folder.length()>0) {
								fclose(hFile);
							}
						}
					}
				}
				
				delete(persistence);
			}
		} while(m_parameters->getParam("format", format));
	}
	
	
	if(m_parameters->setTask("reset")->bGiven) {
		navilock.deleteTracks();
	}
	
	m_parameters->setTask("");
	string distance;
	if(m_parameters->getParam("set-distance", distance)) {
		double dist;
		ASSERT_THROW_s(sscanf(distance.c_str(), "%lf", &dist)==1, "Failed to parse the distance %s", distance.c_str());
		ASSERT_THROW_s(dist>=0.0 && dist<99999.9, "Distance out of bounds (0 <= dist < 99999.9)");
		navilock.setTotalDistance(dist);
	}
	
	
	if(m_parameters->setTask("read-address")->bGiven) {
		uint offset, count;
		int ret=0;
		string soffset, scount;
		m_parameters->getParam("offset", soffset);
		m_parameters->getParam("size", scount);
		
		if(sscanf(soffset.c_str(), "%u", &offset)!=1) offset=0;
		if(sscanf(scount.c_str(), "%u", &count)!=1 || count==0) count=16;
		char buffer[50];
		bool bFirst=true;
		for(uint addr=offset; addr<offset+count && ret!=-1; addr+=ret) {
			
			ret=navilock.readAddr(addr, buffer, sizeof(buffer));
			
			if(bFirst && ret>0) {
				printf("        ");
				for(int i=0; i<ret; ++i) printf("%2i ", i);
				printf("\n       ");
				for(int i=0; i<ret; ++i) printf("---");
				printf("\n");
				bFirst=false;
			}
			
			printf("%6i: ", addr);
			for(int i=0; i<ret; ++i) printf("%02X ", (int)(unsigned char)buffer[i]);
			printf("\n");
		}
		
	}
}











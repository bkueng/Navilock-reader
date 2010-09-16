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

#ifndef NAVILOCK_H_
#define NAVILOCK_H_

#include "serial.h"
#include "global.h"

typedef unsigned char uchar;

#define EARTH_SEMI_MAJOR_AXIS_A 	6378137.0  			//[m] from WGS-84 reference ellipsoid
#define EARTH_SEMI_MAJOR_AXIS_B 	6356752.314  		//[m]

#define DEVICE_MEM_SIZE 			(2*1024*1024) 		//[bytes], 2MB
#define POINT_DATA_LEN 				16 					//[bytes] how much space one point uses
#define POINT_START_ADDR 			3584 				//address with the first point
#define TIME_BETWEEN_POINTS 		5 					//[s] approximate delta time between two saved points


struct ETime {
	char hour;
	char min;
	char sec;
	
	int toSec() { return(((int)hour*60+(int)min)*60+(int)sec); }
	
	string toStr(const char* format="%02i:%02i:%02i") const {
		char b[15];
		sprintf(b, format, (int)hour, (int)min, (int)sec); 
		return(b);
	}
};

struct EDate {
	ushort year;
	char month; //[1-12]
	char day; //[1- depends on month]
	
	static char dayCountFromMonth(char month, ushort year);
	static bool isLeapYear(ushort year);
	
	//days must be smaller then one month (28)!
	void increaseDay(int days=1);
	
	string toStr(const char* format="%04u-%02i-%02i") const {
		char b[15];
		sprintf(b, format, (uint)year, (int)month, (int)day);
		return(b);
	}
};

//not very efficent if several years are between
int getDeltaTimeSec(const EDate& start_date, const ETime& start_time, const EDate& end_date, const ETime& end_time);

string deltaTimeToStr(int sec);


struct EDegree {
	double degree;
	
	/* TODO: check if negative values are always right */
	void set(int degrees, int min, float sec) { 
		if(degrees >= 1000) { //negative
			degree=-((double)degrees-1000+((double)sec/60.0+(double)min)/60.0); 
		} else {
			degree=(double)degrees+((double)sec/60.0+(double)min)/60.0; 
		}
	}
	
	/* format is 47.6546543456 (in degrees) */
	string toStr() {
		char b[20];
		sprintf(b, "%.10lf", degree);
		return(b);
	}
};


struct EPoint;


struct E3dPoint {
	/*
	 * x axis points to (lat=0, lon=90)
	 * y axix points to (lat=0, lon=0)
	 * z axis equals earth axis
	 */
	E3dPoint(double x=0.0, double y=0.0, double z=0.0) : x(x), y(y), z(z) {}
	E3dPoint(const EPoint& p);
	E3dPoint(const E3dPoint& p) : x(p.x), y(p.y), z(p.z) {}
	
	//thats the earth radius
	double EllipsoidDistWGS84(double latitude) const;
	
	double x, y, z; //[m]
};

double pointDistance(const E3dPoint& p1, const E3dPoint& p2);
double pointDistanceSquare(const E3dPoint& p1, const E3dPoint& p2);



struct EPoint {
	EDegree latitude;
	EDegree longitude;
	
	ETime time;
	
	int altitude; //[m]
	uchar speed; //*1.8=km/h
	
	char type; //0: normal point, 1: POI
	
	//additional infos, these are calculated
	E3dPoint point3d;
	double dist; //[m] from track start to and with this point
	double delta_dist; //[m] 
	
	//point_before->calcAdditional() must be called before
	void calcAdditional(const EPoint* point_before);
	
};


struct ETrack {
	ETrack() : bGot_info(false), points(NULL), tot_distance(-1.0) {}
	~ETrack() { if(points) delete[](points); }
	
	bool bGot_info;
	
	EPoint* points;
	uint point_count; //all points, including POI count
	uchar poi_count; //points of interest
	uint start_addr;
	
	EDate start_date;
	ETime start_time;
	
	EDate end_date;
	ETime end_time;
	
	//additional infos, calculated
	double tot_distance;
	uchar max_speed;
	int min_altitude;
	int max_altitude;
	int elevation; //[m] in total
	int descent; //[m] in total
	int time_zero_speed; //[s] total time with speed==0 -> end time - start time - time_zero_speed = driving speed
	
	//points must be loaded, calls calcAdditional for every point
	void calcAdditional();
	
	//[s]
	int tripDuration() const { return(getDeltaTimeSec(start_date, start_time, end_date, end_time)); }
};

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CNavilock
 * is responsible for getting the information from the device using a CDataPoint device
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CNavilock {
public:
	/* devicee must be initialized and ready to be read/written from */
	CNavilock(CDataPoint& device);
	~CNavilock();
	
	/* the tracks (date, time) can be read without getting the track points itself */
	void readTrackInfos();
	
	void readTrack(size_t idx);
	/*read all tracks with the points, also reads the infos if they are not already loaded */
	void readTracks();
	
	/* resets the flash and removes all tracks */
	void deleteTracks();
	
	/* distance is in km */
	void setTotalDistance(double new_distance);
	double totalDistance();
	
	/* read bytes from internal flash memory. how many bytes are read is defined by the device
	 * returns the count of read bytes or -1 on error */
	int readAddr(uint addr, char* buffer, int buffer_size);
	
	const vector<ETrack>& tracks() const { return(m_tracks); }
private:
	/* returns true if track exists and buffer has data, false otherwise */
	bool readTrackInfo(ushort track, char* buffer, int buffer_size);
	
	
	
	CDataPoint& m_device;
	
	vector<ETrack> m_tracks;
	bool m_bRead_track_infos;
	
};


#endif /* NAVILOCK_H_ */

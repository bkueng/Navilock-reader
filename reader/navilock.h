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

#define SPEED_FACTOR				1.8					//calculate km/h from device speed

//constants to calculate energy
#define PLANET_G					9.81				//[m/s^2] acceleration of the earth
#define AIR_DENSITY					1.25				//[kg/m^3]
#define AIR_DRAG_COEFFICIENT		1.0					//how aerodynamic the ciclyst is (race bicycle: 0.88)
#define AREA_COEFFICIENT			0.28				//this value * body height = area
#define WHEEL_DRAG_COEFFICIENT		0.005				//asphalt would be 0.0035 (also depends on wheel pressure)
#define BIKE_EFFICIENCY				0.98				//energy loss in bearing, chain, ...
#define MUSCLES_EFFICIENCY			0.25				//human muscles efficency

#define CALORIES_PER_JOULE			0.239



struct ETime {
	char hour;
	char min;
	char sec;
	
	int toSec() const { return(((int)hour*60+(int)min)*60+(int)sec); }
	
	int diff(const ETime& time_after) const {
		int ret=time_after.toSec()-toSec();
		if(ret<0) ret+=24*60*60;
		return(ret);
	}
	
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
	
	void set(int degrees, int min, float sec) { degree=(double)degrees+((double)sec/60.0+(double)min)/60.0; }
	
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
	//information from device
	EDegree latitude;
	EDegree longitude;
	
	ETime time;
	
	int altitude; //[m]
	
	uchar speed; //*1.8=km/h
	float getSpeed() const { return((float)speed*SPEED_FACTOR); } //[km/h]
	float getSpeedMS() const { return((float)speed*SPEED_FACTOR/3.6); } //[m/s]
	
	char type; //0: normal point, 1: POI
	
	
	//additional infos, these are calculated
	E3dPoint point3d;
	double dist; //[m] from track start to and with this point
	double delta_dist; //[m]
	int delta_time; //[s]
	float faltitude; //[m], smoothed altitude, average of 3 neighbour points
	
	//energy
	float delta_energy; //[J] how much energy from the ciclyst was needed to get form point before to this point
						// a negative value means, the ciclyst braked
						// if greater 0, bike energy loss is included but without human muscles loss
	
	//point_before->calcAdditional() must be called before
	void calcAdditional(const EPoint* point_before, float bicycle_plus_body_weight, float body_height);
	
};


struct ETrack {
	ETrack(float bicycle_plus_body_weight=-1.0, float body_height=-1.0)
	: bGot_info(false), points(NULL), bicycle_plus_body_weight(bicycle_plus_body_weight)
	  , body_height(body_height), tot_distance(-1.0) {}
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
	
	//ciclyst info
	float bicycle_plus_body_weight;
	float body_height;
	
	//additional infos, calculated
	double tot_distance;
	uchar max_speed;
	float getMaxSpeed() const { return((float)max_speed*SPEED_FACTOR); } //[km/h]
	int min_altitude;
	int max_altitude;
	int elevation; //[m] in total
	int descent; //[m] in total
	int time_zero_speed; //[s] total time with speed==0 -> end time - start time - time_zero_speed = driving speed
	
	float used_energy; //[J] how much energy was needed by the cyclist (not how much energy the cyclist burned!)
	float getCyclistEnergyConsumption() const { return(used_energy/MUSCLES_EFFICIENCY); } //[J], how much energy the body used
	float lost_energy; //[J] 'lost' braking energy
	float power; //[W] average power generation (points with speed==0 not included)
	
	//points must be loaded, calls calcAdditional for every point
	void calcAdditional();
	
	//[s]
	int tripDuration() const { return(getDeltaTimeSec(start_date, start_time, end_date, end_time)); }
	
	bool hasEnergyCalculations() const { return(bicycle_plus_body_weight>0 && body_height>0); }
};

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CNavilock
 * is responsible for getting the information from the device using a CDataPoint device
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CNavilock {
public:
	/* devicee must be initialized and ready to be read/written from */
	CNavilock(CDataPoint& device, float bicycle_plus_body_weight=-1.0, float body_height=-1.0);
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
	
	float m_bicycle_plus_body_weight; //[kg]
	float m_body_height; //[m]
	
	
	CDataPoint& m_device;
	
	vector<ETrack> m_tracks;
	bool m_bRead_track_infos;
	
};


#endif /* NAVILOCK_H_ */

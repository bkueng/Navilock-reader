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

#include "navilock.h"

#include <cstring>
#include <sstream>
#include <unistd.h>

#include <unistd.h>

#define IO_BUFFER_SIZE 128

#define READ_UINT(buffer, offset) (((unsigned char)buffer[offset]) | (((unsigned char)buffer[offset+1])<<8) \
		| (((unsigned char)buffer[offset+2])<<16) | (((unsigned char)buffer[offset+3])<<24))

#define READ_USHORT(buffer, offset) (((unsigned char)buffer[offset]) | (((unsigned char)buffer[offset+1])<<8))




char EDate::dayCountFromMonth(char month, ushort year) {
	switch(month) {
	case 1: return(31);
	case 2: if(isLeapYear(year)) return(29); return(28);
	case 3: return(31);
	case 4: return(30);
	case 5: return(31);
	case 6: return(30);
	case 7: return(31);
	case 8: return(31);
	case 9: return(30);
	case 10: return(31);
	case 11: return(30);
	case 12: return(31);
	}
	return(30);
}

bool EDate::isLeapYear(ushort year) {
	if(year%4==0) {
		if(year%100==0 && year%400!=0) return(false);
		return(true);
	}
	return(false);
}

void EDate::increaseDay(int days) {
	char day_count=dayCountFromMonth(month, year);
	if((day+=days) > day_count) {
		day-=day_count;
		if(++month>12) {
			++year;
			month=1;
		}
	}
}

int getDeltaTimeSec(const EDate& start_date, const ETime& start_time, const EDate& end_date, const ETime& end_time) {
	
	int ret=(((int)end_time.hour-(int)start_time.hour)*60+((int)end_time.min-(int)start_time.min))*60
			+((int)end_time.sec-(int)start_time.sec);
	
	int a=(int)start_date.year*13+(int)start_date.month;
	int b=(int)end_date.year*13+(int)end_date.month;
	for(int i=a; i<b; ++i) {
		ret+=(int)EDate::dayCountFromMonth(i%13, i/13)*3600*24;
	}
	ret+=((int)end_date.day-(int)start_date.day)*3600*24;
	
	return(ret);
}


string deltaTimeToStr(int sec) {
	stringstream s;
	int a;
	if((a=sec/(3600*24))>0) s<<a<<"d ";
	if((a=(sec/3600)%24)>0) s<<a<<"h ";
	if((a=(sec/60)%60)>0) s<<a<<"m ";
	if((a=(sec)%60)>0) s<<a<<"s ";
	
	string ret=s.str();
	if(ret.length()==0) return("0 s");
	return(ret.substr(0, ret.length()-1));
}




E3dPoint::E3dPoint(const EPoint& p) {
	double N=EllipsoidDistWGS84(p.latitude.degree);
	double cos_lat=cos(p.latitude.degree/180.0*M_PI);
	y=(N+(double)p.altitude)*cos_lat*cos(p.longitude.degree/180.0*M_PI);
	x=(N+(double)p.altitude)*cos_lat*sin(p.longitude.degree/180.0*M_PI);
	double bovera=EARTH_SEMI_MINOR_AXIS_B/EARTH_SEMI_MAJOR_AXIS_A;
	z=(bovera*bovera*N+(double)p.altitude)*sin(p.latitude.degree/180.0*M_PI);
	
}

double E3dPoint::EllipsoidDistWGS84(double latitude) const {
	double sin_lat=sin(latitude/180.0*M_PI);
	double bovera=EARTH_SEMI_MINOR_AXIS_B/EARTH_SEMI_MAJOR_AXIS_A;
	return(EARTH_SEMI_MAJOR_AXIS_A / sqrt(1.0-sin_lat*sin_lat*(1.0-bovera*bovera)));
}


double pointDistance(const E3dPoint& p1, const E3dPoint& p2) {
	return(sqrt(pointDistanceSquare(p1, p2)));
}

double pointDistanceSquare(const E3dPoint& p1, const E3dPoint& p2) {
	return((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) + (p1.z-p2.z)*(p1.z-p2.z));
}



void EPoint::calcAdditional(const EPoint* point_before) {
	
	point3d=E3dPoint(*this);
	if(point_before) {
		delta_dist=pointDistance(point3d, point_before->point3d);
		dist=delta_dist+point_before->dist;
	} else {
		delta_dist=dist=0.0;
	}
	
}

void ETrack::calcAdditional() {
	if(bGot_info && tot_distance==-1.0) {
		tot_distance=0.0;
		max_speed=0;
		min_altitude=99999;
		time_zero_speed=max_altitude=elevation=descent=0;
		EPoint* last_point=NULL;
		
		uint i=0;
		while(i<point_count && points[i].altitude == 0) { //skip the first points with height 0
			points[i].calcAdditional(last_point);
			++i;
		}
		if(i==point_count) i=0;
		for(; i<point_count; ++i) {
			points[i].calcAdditional(last_point);
			tot_distance+=points[i].delta_dist;
			if(points[i].speed>max_speed) max_speed=points[i].speed;
			if(points[i].altitude>max_altitude) max_altitude=points[i].altitude;
			if(points[i].altitude<min_altitude) min_altitude=points[i].altitude;
			if(last_point) {
				if(points[i].speed==0) {
					int dsec=points[i].time.toSec()-last_point->time.toSec();
					if(dsec < 0) dsec+=24*60*60;
					time_zero_speed+=dsec;
				}
				if(last_point->altitude!=0 && points[i].altitude!=0) {
					if(last_point->altitude < points[i].altitude) {
						elevation+=points[i].altitude-last_point->altitude;
					} else {
						descent+=last_point->altitude-points[i].altitude;
					}
				}
			}
			last_point=points+i;
		}
	}
}

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CNavilock
/*////////////////////////////////////////////////////////////////////////////////////////////////


CNavilock::CNavilock(CDataPoint& device) : m_device(device), m_bRead_track_infos(false) {
	
}

CNavilock::~CNavilock() {
	
}


bool CNavilock::readTrackInfo(ushort track, char* buffer, int buffer_size) {
	if(buffer_size<24) return(false);
	
	char request[]= { 0x54, 0x46, 0x00, 0x00, 0x00, 0x00 };
	
	request[4]=track>>8;
	request[5]=(track & 0xFF);
	m_device.write(request, sizeof(request));
	int ret=m_device.read(buffer, buffer_size);
	ASSERT_THROW_e(ret==24, EDEVICE, "Expected Answer of 24 bytes but got %i", ret);
	
	static const unsigned char no_track[]= { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
									0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
									0xFF, 0xFF, 0xFF, 0xFF
									};
	return(memcmp(buffer, no_track, sizeof(no_track))!=0);
}

void CNavilock::readTrackInfos() {
	if(!m_bRead_track_infos) {
		
		ushort track_id=0;
		char buffer[IO_BUFFER_SIZE];
		ETrack track;
		
		while(readTrackInfo(track_id, buffer, IO_BUFFER_SIZE)) {
			
			/* parse buffer */
			//poi count
			track.poi_count=(uchar)buffer[12];
			//point count
			track.point_count=READ_UINT(buffer, 0) + (uint)track.poi_count;
			//address
			track.start_addr=READ_UINT(buffer, 4);
			
			//year
			track.start_date.year=READ_USHORT(buffer, 8);
			//start date/time
			track.start_date.month=buffer[14];
			track.start_date.day=buffer[15];
			track.start_time.hour=buffer[16];
			track.start_time.min=buffer[17];
			track.start_time.sec=buffer[18];
			//end date/time
			track.end_date.month=buffer[19];
			track.end_date.day=buffer[20];
			track.end_time.hour=buffer[21];
			track.end_time.min=buffer[22];
			track.end_time.sec=buffer[23];
			track.end_date.year=READ_USHORT(buffer, 10);
			
			
			LOG(DEBUG, "Track %i: Point count: %u, POI count: %u, start addr %u, start: %s %s end: %s %s"
					, track_id, track.point_count, (uint)track.poi_count, track.start_addr, track.start_date.toStr().c_str(),
					track.start_time.toStr().c_str(), track.end_date.toStr().c_str(), track.end_time.toStr().c_str());
			
			/*
			for(int i=0; i<24; ++i) printf("%02X ", (int)(unsigned char)buffer[i]);
			printf("\n");
			//*/
			
			m_tracks.push_back(track);
			
			++track_id;
		}
		m_bRead_track_infos=true;
	}
}


void CNavilock::readTrack(size_t idx) {
	readTrackInfos();
	ASSERT_THROW(idx<m_tracks.size(), EINVALID_PARAMETER);
	
	//read the track
	ETrack& track=m_tracks[idx];
	char buffer[IO_BUFFER_SIZE];
	uint val;
	
	if(!track.bGot_info) {
		LOG(DEBUG, "Reading Track %u with %u Points", idx, track.point_count); 
		
		if(track.point_count>0) {
			track.points=new EPoint[track.point_count];
			int ret;
			
			for(uint i=0; i<track.point_count; ++i) {
				ASSERT_THROW_e((ret=readAddr(track.start_addr+i*0x10, buffer, IO_BUFFER_SIZE))==POINT_DATA_LEN, EDEVICE
						, "Failed to read addr %u from track %i (returned with %i, expected %i)", track.start_addr+i, idx, ret
						, POINT_DATA_LEN);
				/* parse buffer */
				EPoint& point=track.points[i];
				//latitude
				val=READ_UINT(buffer, 0);
				point.latitude.set((int)(val/100000), (int)(val/1000)%100, (float)(val%1000)/10.0);
				//longitude
				val=READ_UINT(buffer, 4);
				point.longitude.set((int)(val/100000), (int)(val/1000)%100, (float)(val%1000)/10.0);
				//type
				point.type=buffer[8];
				//speed
				point.speed=(uchar)buffer[9];
				//time
				point.time.hour=buffer[10];
				point.time.min=buffer[11];
				point.time.sec=buffer[12];
				//altitude
				point.altitude=(short)READ_USHORT(buffer, 14);
				
				
				LOG(DEBUG, "  Point %2u: lat: %s lon: %s type: %i, time: %s, alt: %im, speed: %3.1fkm/h", 
						i, point.latitude.toStr().c_str(), point.longitude.toStr().c_str(),
						(int)point.type, point.time.toStr().c_str(), point.altitude, (float)point.speed*1.8);
				
				/*
				for(int i=0; i<16; ++i) printf("%02X ", (int)(unsigned char)buffer[i]);
				printf("\n");
				//*/
			}
		}
		track.bGot_info=true;
		track.calcAdditional();
	}
}


int CNavilock::readAddr(uint addr, char* buffer, int buffer_size) {
	
	char request[]= { 0x54, 0x50, 0x00, 0x00, 0x00, 0x00 };
	
	request[2]=(addr>>24) & 0xFF;
	request[3]=(addr>>16) & 0xFF;
	request[4]=(addr>>8) & 0xFF;
	request[5]=(addr & 0xFF);
	
	m_device.write(request, sizeof(request));
	return(m_device.read(buffer, buffer_size));
}

void CNavilock::readTracks() {
	readTrackInfos();
	for(size_t i=0; i<m_tracks.size(); ++i) readTrack(i);
}


void CNavilock::deleteTracks() {
	
	printf("Removing all tracks...\n");
	
	char request[]= { 0x45, 0x52, 0x00, 0x00, 0x00, 0x00 };
	m_device.write(request, sizeof(request));
	
	char response[40];
	int ret=3;
	memset(response, 0, sizeof(response));
	while(response[2]<0x64 && ret==3) {
		int ret=m_device.read(response, sizeof(response));
		
		if(ret==3) printf("\b\b\b%2i%%", (int)(unsigned char)response[2]);
		fflush(stdout);
		
		::usleep(200000);
	}
	printf("\n");
}


void CNavilock::setTotalDistance(double new_distance) {
	
	char request[]= { 0x43, 0x44, 0x00, 0x00, 0x00, 0x00 };
	
	int dist=(int)(new_distance*10);
	request[2]=(dist>>24) & 0xFF;
	request[3]=(dist>>16) & 0xFF;
	request[4]=(dist>>8) & 0xFF;
	request[5]=(dist & 0xFF);
	m_device.write(request, sizeof(request));
	
	char answer[20];
	int ret=m_device.read(answer, sizeof(answer));
	ASSERT_THROW_s(ret==3, "Failed to get a response after setting the total Distance to %lf", new_distance);
	//we don't care about the content of the response
	
}

double CNavilock::totalDistance() {
	char buffer[20];
	int ret=readAddr(0, buffer, sizeof(buffer));
	ASSERT_THROW_e(ret>=16, EDEVICE, "Failed to read the total distance");
	
	uint km=READ_UINT(buffer, 12);
	return((double)km/10.0);
}











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

#include "persistence.h"


/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistence
/*////////////////////////////////////////////////////////////////////////////////////////////////

CPersistence::CPersistence() {
}

CPersistence::~CPersistence() {
}

	
/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistenceGpx
/*////////////////////////////////////////////////////////////////////////////////////////////////

CPersistenceGpx::CPersistenceGpx() {
}

CPersistenceGpx::~CPersistenceGpx() {
}


string CPersistenceGpx::time(const EDate& date, const ETime& time) {
	//form: 2010-05-08T13:36:08Z
	char b[30];
	sprintf(b, "%04u-%02i-%02iT%02i:%02i:%02iZ", (uint)date.year, (int)date.month
			, (int)date.day, (int)time.hour, (int)time.min, (int)time.sec);
	return(b);
}
	
	
void CPersistenceGpx::write(FILE* hFile, const ETrack& track) {
	//header
	fprintf(hFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			"<gpx\n"
			"version=\"1.0\"\n"
			"creator=\"Navilock Reader\"\n"
			"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
			"xmlns =\"http://www.topografix.com/GPX/1/0\"\n"
			"xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\n"
			"<time>%s</time>\n"
			, time(track.start_date, track.start_time).c_str());
	
	
	if(track.point_count>0) {
		EPoint& first_p=track.points[0];
		EPoint& last_p=track.points[track.point_count-1];
		fprintf(hFile, "<wpt lat=\"%.10lf\" lon=\"%lf.10\">\n"
			"  <ele>%i</ele>\n"
			"  <time>%s</time>\n"
			"  <speed>%.1f</speed>\n"
			"  <name>Start %s %s</name>\n"
			"  <cmt>Start%s</cmt>\n"
			"  <desc>Start %s %s</desc>\n"
			"</wpt>\n"
			"<wpt lat=\"%.10lf\" lon=\"%.10lf\">\n"
			"  <ele>%i</ele>\n"
			"  <time>%s</time>\n"
			"  <speed>%.1f</speed>\n"
			"  <name>End %s %s</name>\n"
			"  <cmt>End%s</cmt>\n"
			"  <desc>End %s %s</desc>\n"
			"</wpt>\n"
			, first_p.latitude.degree, first_p.longitude.degree, first_p.altitude
			, time(track.start_date, track.start_time).c_str(), (float)first_p.speed*1.8
			, track.start_date.toStr().c_str(), track.start_time.toStr().c_str(), time(track.start_date, track.start_time).c_str()
			, track.start_date.toStr().c_str(), track.start_time.toStr().c_str()
			, last_p.latitude.degree, last_p.longitude.degree, last_p.altitude
			, time(track.end_date, track.end_time).c_str(), (float)last_p.speed*1.8
			, track.end_date.toStr().c_str(), track.end_time.toStr().c_str(), time(track.end_date, track.end_time).c_str()
			, track.end_date.toStr().c_str(), track.end_time.toStr().c_str()
		);
	}
	
	//POI's
	EDate point_date=track.start_date;
	char last_hour=track.start_time.hour;
	
	uint poi_id=0;
	for(uint i=0; i<track.point_count; ++i) {
		EPoint& point=track.points[i];
		if(point.time.hour < last_hour) { //date changed
			point_date.increaseDay();
		}
		if(track.points[i].type!=0) {
			++poi_id;
			fprintf(hFile, "<wpt lat=\"%.10lf\" lon=\"%.10lf\">\n"
				"  <ele>%i</ele>\n"
				"  <time>%s</time>\n"
				"  <name>POI %u</name>\n"
				"  <speed>%.1f</speed>\n"
				"  <desc>Altitude：%i m\n"
//				"Distance：  \n"       //TODO
				"Time：%s %s</desc>\n"
				"</wpt>\n"
				, point.latitude.degree, point.longitude.degree, point.altitude
				, time(point_date, point.time).c_str(), poi_id, (float)point.speed*1.8
				, point.altitude, point_date.toStr().c_str(),point.time.toStr().c_str());
		}
	}
	
	fprintf(hFile, "<trk>\n <trkseg>\n");
	
	point_date=track.start_date;
	last_hour=track.start_time.hour;
	
	//points
	for(uint i=0; i<track.point_count; ++i) {
		if(track.points[i].type==0) {
			EPoint& point=track.points[i];
			if(point.time.hour < last_hour) { //date changed
				point_date.increaseDay();
			}
			last_hour=point.time.hour;
			fprintf(hFile, 
				" <trkpt lat=\"%.10lf\" lon=\"%.10lf\">\n"
				"  <ele>%i</ele>\n"
				"  <time>%s</time>\n"
				"  <speed>%.1f</speed>\n"
				" </trkpt>\n"
				, point.latitude.degree, point.longitude.degree
				, point.altitude, time(point_date, point.time).c_str()
				, (float)point.speed*1.8);
		}
	}
	fprintf(hFile, " </trkseg>\n</trk>\n</gpx>\n\n");
}



/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistenceTxt
/*////////////////////////////////////////////////////////////////////////////////////////////////

CPersistenceTxt::CPersistenceTxt() {
}

CPersistenceTxt::~CPersistenceTxt() {
}


	
	
void CPersistenceTxt::write(FILE* hFile, const ETrack& track) {
	
	int track_time=track.tripDuration();
	int driving_time=track_time-track.time_zero_speed;
	
	fprintf(hFile, 
			"Points:                           %8u\n"
			"Points of interest:               %8u\n"
			"Start:                 %s %s\n"
			"End:                   %s %s\n"
			"Duration:        %25s\n"
			"Driving time:    %25s\n"
			"Distance:                      %8.3lf km\n"
			"Performace distance:           %8.3lf km\n"
			"Average speed:                  %5.1f km/h\n"
			"Max speed:                      %5.1f km/h\n"
			"Min altitude:                   %8i m\n"
			"Max altitude:                   %8i m\n"
			"Elevation:                      %8i m\n"
			"Descent:                        %8i m\n"
			"\n"
			, track.point_count-(uint)track.poi_count, (uint)track.poi_count
			, track.start_date.toStr().c_str(), track.start_time.toStr().c_str()
			, track.end_date.toStr().c_str(), track.end_time.toStr().c_str()
			, deltaTimeToStr(track_time).c_str(), deltaTimeToStr(driving_time).c_str()
			, track.tot_distance/1000.0, track.tot_distance/1000.0+(double)track.elevation/100.0
			, (float)(track.tot_distance/1000.0)/((float)driving_time/3600.0)
			, (float)track.max_speed*1.8
			, track.min_altitude, track.max_altitude
			, track.elevation, track.descent
			);
	
	
	//POI's
	EDate point_date=track.start_date;
	char last_hour=track.start_time.hour;
	
	uint poi_id=0;
	for(uint i=0; i<track.point_count; ++i) {
		EPoint& point=track.points[i];
		if(point.time.hour < last_hour) { //date changed
			point_date.increaseDay();
		}
		last_hour=point.time.hour;
		if(track.points[i].type!=0) {
			++poi_id;
			fprintf(hFile, 
				"POI   %2u:  lat=%.10lf  lon=%.10lf  time=%s  alt=%4im  dist=%7.3lfkm  x=%.10lfm  y=%.10lfm  z=%.10lfm\n"
				, poi_id, point.latitude.degree, point.longitude.degree
				, point.time.toStr().c_str(), point.altitude
				, point.dist/1000.0
				, point.point3d.x, point.point3d.y, point.point3d.z);
		}
	}
	
	fprintf(hFile, "\n");
	
	point_date=track.start_date;
	last_hour=track.start_time.hour;
	uint point_id=0;
	
	
	//points
	for(uint i=0; i<track.point_count; ++i) {
		if(track.points[i].type==0) {
			++point_id;
			EPoint& point=track.points[i];
			if(point.time.hour < last_hour) { //date changed
				point_date.increaseDay();
			}
			last_hour=point.time.hour;
			fprintf(hFile, 
				"Point %2u:  lat=%.10lf  lon=%.10lf  time=%s  alt=%4im  speed=%5.1fkm/h  delta_dist=%5.1lfm  dist=%7.3lfkm  x=%.10lfm  y=%.10lfm  z=%.10lfm\n"
				, point_id, point.latitude.degree, point.longitude.degree
				, point.time.toStr().c_str(), point.altitude, (float)point.speed*1.8
				, point.delta_dist, point.dist/1000.0
				, point.point3d.x, point.point3d.y, point.point3d.z);
			
		}
	}
	fprintf(hFile, "\n");
}




/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistenceGravity
/*////////////////////////////////////////////////////////////////////////////////////////////////

CPersistenceGravity::CPersistenceGravity() {
}

CPersistenceGravity::~CPersistenceGravity() {
}


	
	
void CPersistenceGravity::write(FILE* hFile, const ETrack& track) {
	
	
	E3dPoint offset;
	
	if(track.point_count>0) {
		offset=track.points[0].point3d;
		
		fprintf(hFile, 
				"$scale=1\n"
				"$rad=1\n"
				"$colr=0.8\n"
				"$colg=0\n"
				"$colb=0\n"
				"$cola=1\n"
				"\n"
				"[camera]\n"
				"pos=50, 0, 0\n"
				"look_direction=-50,-30,-100\n"
				"use_light=1\n"
				"\n");
	}
	
	//POI's
	
	EDate point_date=track.start_date;
	char last_hour=track.start_time.hour;
	/*
	uint poi_id=0;
	for(uint i=0; i<track.point_count; ++i) {
		EPoint& point=track.points[i];
		if(point.time.hour < last_hour) { //date changed
			point_date.increaseDay();
		}
		last_hour=point.time.hour;
		if(track.points[i].type!=0) {
			++poi_id;
			fprintf(hFile, 
				"POI   %2u:  lat=%.10lf  lon=%.10lf  time=%s  alt=%4im  dist=%7.3lfkm  x=%.10lfm  y=%.10lfm  z=%.10lfm\n"
				, poi_id, point.latitude.degree, point.longitude.degree
				, point.time.toStr().c_str(), point.altitude
				, point.dist/1000.0
				, point.point3d.x, point.point3d.y, point.point3d.z);
		}
	}
	
	fprintf(hFile, "\n");
	*/
	point_date=track.start_date;
	last_hour=track.start_time.hour;
	uint point_id=0;
	
	
	//points
	for(uint i=0; i<track.point_count; i+=10) {
		if(track.points[i].type==0) {
			++point_id;
			EPoint& point=track.points[i];
			if(point.time.hour < last_hour) { //date changed
				point_date.increaseDay();
			}
			last_hour=point.time.hour;
			fprintf(hFile,
					"[sphere]\n"
					"pos=$scale*%lf, $scale*%lf, $scale*%lf\n"
					"color=$colr, $colg, $colb, $cola\n"
					"mass=0.1\n"
					"radius=$rad\n"
					"fixed_pos=1\n"
					"\n"
					, point.point3d.x-offset.x, point.point3d.y-offset.y, point.point3d.z-offset.z);
			
		}
	}
	fprintf(hFile, "\n");
}



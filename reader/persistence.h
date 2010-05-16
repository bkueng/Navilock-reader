/*
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version 2
 *	as published by the Free Software Foundation.
 */

#ifndef PERSISTENCE_H_
#define PERSISTENCE_H_

#include "navilock.h"


/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistence
 * base class for writing the data
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CPersistence {
public:
	CPersistence();
	virtual ~CPersistence();
	
	
	virtual void write(FILE* hFile, const ETrack& track)=0;
	
private:
	
};

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistenceGpx
 * class for writing GPX data file
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CPersistenceGpx : public CPersistence {
public:
	CPersistenceGpx();
	virtual ~CPersistenceGpx();
	
	
	virtual void write(FILE* hFile, const ETrack& track);
	
private:
	// gpx standard time format
	string time(const EDate& date, const ETime& time);
};


/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistenceTxt
 * class for writing simple txt data file with additional track information
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CPersistenceTxt : public CPersistence {
public:
	CPersistenceTxt();
	virtual ~CPersistenceTxt();
	
	
	virtual void write(FILE* hFile, const ETrack& track);
	
private:
	
};

/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CPersistenceGravity
 * class for writing a gravity input file
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CPersistenceGravity : public CPersistence {
public:
	CPersistenceGravity();
	virtual ~CPersistenceGravity();
	
	
	virtual void write(FILE* hFile, const ETrack& track);
	
private:
	
};


#endif /* PERSISTENCE_H_ */

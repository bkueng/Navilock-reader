/*
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version 2
 *	as published by the Free Software Foundation.
 */

#include "serial.h"
#include "exception.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>


/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CDataPoint
/*////////////////////////////////////////////////////////////////////////////////////////////////

CDataPoint::CDataPoint() {
	
}
CDataPoint::~CDataPoint() {
	
}




/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CSerial
/*////////////////////////////////////////////////////////////////////////////////////////////////

CSerial::CSerial() : m_fd(-1) {
	
}

CSerial::~CSerial() {
	close();
}

void CSerial::open(const char* device) {
	if(m_fd==-1) {
		m_fd = ::open(device, O_RDWR | O_NOCTTY);
		ASSERT_THROW(m_fd>=0, EDEVICE);
		
		tcgetattr(m_fd,&m_oldtio); /* save current port settings */
	}
}

void CSerial::close() {
	if(m_fd!=-1) {
		tcsetattr(m_fd,TCSANOW,&m_oldtio); /* restore old port settings */
		::close(m_fd);
		m_fd=-1;
	}
}


void CSerial::initConnection(unsigned int baudrate) {
	ASSERT_THROW(m_fd!=-1, ENOT_INITIALIZED);
	
	termios tio;
	bzero(&tio, sizeof(tio));
	
	tio.c_cflag = baudrate | CRTSCTS | CS8 | CLOCAL | CREAD;
	tio.c_iflag = IGNPAR;
	tio.c_oflag = 0;
	
	/* set input mode (non-canonical, no echo,...) */
	tio.c_lflag = 0;
	
	tio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	tio.c_cc[VMIN]     = 4;   /* blocking read until 4 chars received */
	
	tcflush(m_fd, TCIFLUSH);
	ASSERT_THROW(tcsetattr(m_fd,TCSANOW, &tio)==0, EDEVICE);
}


int CSerial::read(char* buffer, int buffer_len) {
	return(::read(m_fd, buffer, buffer_len));
}

int CSerial::write(const char* buffer, int buffer_len) {
	return(::write(m_fd, buffer, buffer_len));
}







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

#include "serial.h"
#include "exception.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include <cstring>
#include <cstdio>

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
		ASSERT_THROW_e(m_fd>=0, EDEVICE, "Failed to open the serial device %s (%s)", device, strerror(errno));
		
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
	tio.c_cc[VMIN]     = 3;   /* blocking read until 3 chars received */
	
	tcflush(m_fd, TCIFLUSH);
	ASSERT_THROW_e(tcsetattr(m_fd,TCSANOW, &tio)==0, EDEVICE, "Failed to set device settings (%s)", strerror(errno));
}


int CSerial::read(char* buffer, int buffer_len) {
	return(::read(m_fd, buffer, buffer_len));
}

int CSerial::write(const char* buffer, int buffer_len) {
	return(::write(m_fd, buffer, buffer_len));
}







/*
 * Copyright (C) 2010 Beat Küng <beat-kueng@gmx.net>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License version 2
 *	as published by the Free Software Foundation.
 */

#ifndef SERIAL_H_
#define SERIAL_H_


#include <termios.h>


/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CDataPoint
 * abstraction class for reading/writing
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CDataPoint {
public:
	CDataPoint();
	virtual ~CDataPoint();
	
	virtual void open(const char* device)=0;
	virtual void close()=0;
	
	/* return: number of bytes read, -1 on error: see errno */
	virtual int read(char* buffer, int buffer_len)=0;
	
	/* return: number of written bytes, -1 on error: see errno */
	virtual int write(const char* buffer, int buffer_len)=0;
private:
	
};



/*////////////////////////////////////////////////////////////////////////////////////////////////
 ** class CSerial
 * opens a serial connection for reading and writing
/*////////////////////////////////////////////////////////////////////////////////////////////////

class CSerial : public CDataPoint {
public:
	CSerial();
	~CSerial();
	
	void open(const char* device);
	void close();
	
	//call this after open()
	void initConnection(unsigned int baudrate);
	
	int read(char* buffer, int buffer_len);
	int write(const char* buffer, int buffer_len);
	
private:
	int m_fd;
	struct termios m_oldtio;
};






#endif /* SERIAL_H_ */

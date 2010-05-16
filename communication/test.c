#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <cstdio>
#include <unistd.h>
#include <cstring>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyUSB1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 

//test app to communicate with the device



int main(){
	int fd,c, res;
	struct termios oldtio,newtio;
	char buf[255];

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY ); 
	if (fd <0) {perror(MODEMDEVICE); return(-1); }

	tcgetattr(fd,&oldtio); /* save current port settings */

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 4;   /* blocking read until 4 chars received */

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);


	char out[]= { 0x54, 0x46, 0x00, 0x00, 0x00, 0x00 };
	write(fd, out, sizeof(out));
	
	res = read(fd,buf,255);   /* returns after 5 chars have been input */
	printf("readport=got %i chars\n", res);
	for(int i=0; i<res; ++i) printf("%02X ", (int)(unsigned char)buf[i]);
	printf("\n");
	
	
	//second
	char out2[]= { 0x54, 0x50, 0x00, 0x00, 0x0E, 0x00  };
	write(fd, out2, sizeof(out2));
		
	res = read(fd,buf,255);   /* returns after 5 chars have been input */
	printf("readport=got %i chars\n", res);
	for(int i=0; i<res; ++i) printf("%02X ", (int)(unsigned char)buf[i]);
	printf("\n");
	
	
	tcsetattr(fd,TCSANOW,&oldtio);
}


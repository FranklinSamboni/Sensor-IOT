
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* baudrate settings are defined in <asm/termbits.h>, which is
   included by <termios.h> */
#define BAUDRATE B9600   // Change as needed, keep B

/* change this definition for the correct port */
#define MODEMDEVICE "/dev/ttyO2" //Beaglebone Black serial port

#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

int main(int argc, char *argv[]){
    printf("\nCoomienzo Modificado\n");

    int fd, c, res;
    struct termios oldtio, newtio;
    char buf[255];
    printf("\nCoomienzo Modificado\n");

// Load the pin configuration
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    if (fd < 0) {
	printf("\nError\n");
	perror(MODEMDEVICE);
	exit(-1); }
    printf("\nCoomienzo Modificado\n");
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */

    /* BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
       CRTSCTS : output hardware flow control (only used if the cable has
                 all necessary lines. See sect. 7 of Serial-HOWTO)
       CS8     : 8n1 (8bit,no parity,1 stopbit)
       CLOCAL  : local connection, no modem contol
       CREAD   : enable receiving characters */
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;

    /* IGNPAR  : ignore bytes with parity errors
       otherwise make device raw (no other input processing) */
    newtio.c_iflag = IGNPAR;

    /*  Raw output  */
    newtio.c_oflag = 0;

    /* ICANON  : enable canonical input
       disable all echo functionality, and don't send signals to calling program */
    newtio.c_lflag = ICANON;
    /* now clean the modem line and activate the settings for the port */
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
    // NMEA command to ouput all sentences
    // Note that this code & format values in manual are hexadecimal
    printf("\nleyo\n");
//    write(fd, "$PTNLSNM,273F,01*27\r\n", 21);
    // Para desabilitar los mensajes de salida del gps
/*    char buffer[9];
    buffer[0] = 0xA0;
    buffer[1] = 0xA1;
    buffer[2] = 0x00;
    buffer[3] = 0x03;
    buffer[4] = 0x09;
    buffer[5] = 0x01;
    buffer[6] = 0x09;
    buffer[7] = 0x0D;
    buffer[8] = 0x0A;
    write(fd,buffer,16);
*/
/*
    char buffe[9];
    buffe[0] = 0xA0;
    buffe[1] = 0xA1;
    buffe[2] = 0x00;
    buffe[3] = 0x02;
    buffe[4] = 0x30;
    buffe[5] = 0x00;
    buffe[6] = 0x30;
    buffe[7] = 0x0D;
    buffe[8] = 0x0A;
    write(fd,buffe,16);
*/
/*    char buffer[16];
    buffer[0] = 0xA0;
    buffer[1] = 0xA1;
    buffer[2] = 0x00;
    buffer[3] = 0x09;
    buffer[4] = 0x08;
    buffer[5] = 0x01;
    buffer[6] = 0x01;
    buffer[7] = 0x01;
    buffer[8] = 0x00;
    buffer[9] = 0x01;
    buffer[10] = 0x00;
    buffer[11] = 0x00;
    buffer[12] = 0x00;
    buffer[13] = 0x08;
    buffer[14] = 0x0D;
    buffer[15] = 0x0A;
    write(fd,buffer,16);
*/
    printf("termino\n");
    /* terminal settings done, now handle input*/
    while (TRUE) {     /* loop continuously */
	//printf("\n while\n");
        /*  read blocks program execution until a line terminating character is
            input, even if more than 255 chars are input. If the number
            of characters read is smaller than the number of chars available,
            subsequent reads will return the remaining chars. res will be set
            to the actual number of characters actually read */
	printf("\nreading\n");
	res = read(fd, buf, 255);
        if(res > 1){
		printf("\nres es: %d\n",res);
		 int i=0;
		 while(i<res){
		 //printf("%d- ",i);
          	 printf("!%hhX! ",buf[i],res);
			/*if(i==16){
			 printf("\n");
			}*/
        	 i = i + 1;
		}
		printf("\nlinea\n");
		buf[res] = 0;
		printf("%s",buf,res);

	}
	else{
		printf("eror leyendo");
		return 0;
	}
        //buf[res] = 0;             /* set end of string, so we can printf */
        //printf("%s", buf, res);
    }
    tcsetattr(fd, TCSANOW, &oldtio);
    return 0;
}


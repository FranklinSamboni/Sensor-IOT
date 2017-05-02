/*
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>

int main(int argc, char *argv[]){
   int file, count;
   if(argc!=2){
       printf("Invalid number of arguments, exiting!\n");
       return -2;
   }
   if ((file = open("/dev/ttyO2", O_RDWR | O_NOCTTY | O_NDELAY))<0){
      perror("UART: Failed to open the file.\n");
      return -1;
   }
   struct termios options;
   tcgetattr(file, &options);
   options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
   options.c_iflag = IGNPAR | ICRNL;
   tcflush(file, TCIFLUSH);
   tcsetattr(file, TCSANOW, &options);

   // send the string plus the null character
    unsigned char write_Buf[2];
    unsigned int addr = 0b00000000;
    unsigned int data = 0b00000000;
    printf("valor: %x \n", data);
    write_Buf[0] = addr;
    write_Buf[1] = data;


   if (count = write(file, write_Buf, 2)<0){
      perror("Failed to write to the output\n");
      return -1;
   }
   usleep(100000);
   unsigned char receive[100];
   if ((count = read(file, (void*)receive, 100))<0){
      perror("Failed to read from the input\n");
      return -1;
   }
   if (count==0) printf("There was no data available to read!\n");
   else {
      receive[count]=0;  //There is no null character sent by the Arduino
      printf("The following was read in [%d]: %s\n",count,receive);
   }
   close(file);
   return 0;
}
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* baudrate settings are defined in <asm/termbits.h>, which is
   included by <termios.h> */
#define BAUDRATE B115200   // Change as needed, keep B

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
    // Load the pin configuration

    //int ret = system("echo uart2 > /sys/devices/bone_capemgr.9/slots");
    /* Open modem device for reading and writing and not as controlling tty
       because we don't want to get killed if linenoise sends CTRL-C. */
    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );
    if (fd < 0) { perror(MODEMDEVICE); exit(-1); }

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
//    char data[17];
//    char * dat;
//    sprintf(data,"$PGRMO,GPRMC,0*3C\r\n");
//    char * trama;
//    trama = "$PGRMO,GPVTG,0*3C\r\n";
//     dat = data;
//    printf(trama);
/*    if(write(fd,"$PGRMO,,2*75\r\n",30) < 1){
	perror("Error al escribir en el GPS");
	abort();
	}
    if(write(fd,"$PGRMO,GPRMC,1*3D\r\n",30) < 1){
        perror("Error al escribir en el GPS");
        abort();
        }*/
    char buffer[16];
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
    printf("termino\n");
    /* terminal settings done, now handle input*/
    while (TRUE) {     /* loop continuously */
	//printf("\n while\n");
        /*  read blocks program execution until a line terminating character is
            input, even if more than 255 chars are input. If the number
            of characters read is smaller than the number of chars available,
            subsequent reads will return the remaining chars. res will be set
            to the actual number of characters actually read */
	res = read(fd, buf, 255);
        if(res > 1){
		buf[res] = 0;
		printf("%s",buf,res);
	}else{
		printf("eror leyendo");
		//break;
	}
        //buf[res] = 0;             /* set end of string, so we can printf */
        //printf("%s", buf, res);
    }
    tcsetattr(fd, TCSANOW, &oldtio);
return 0;
}


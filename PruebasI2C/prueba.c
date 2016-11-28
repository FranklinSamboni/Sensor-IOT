#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include </usr/include/linux/i2c-dev.h>


#define ADDRESS 0b1101000  // Define Adress

int testSendMsg(){
    int fd = open( (char*) "/dev/i2c-2", O_RDWR);	// Open Device
    if (fd < 0){                            // Check if it is opened
	perror("Can't open device");
	abort();
    }
    if(ioctl (fd, I2C_SLAVE, ADDRESS) < 0 ){	// Set slave address
	perror("Failed to set slave address\n");
	abort();
    }
    char buffer[10] = {0};				// Create Buffer
    buffer[0]=0xF8;					// Fill in some values
    buffer[1]=0x33;
    if( write( fd, buffer, 2) < 1) {		// Send message 
 	perror("Failed to send data");
	abort();
    }else{
  	close(fd);
	return 1;
    }
    return 0;
}

int main(int argc, const char *argv[]){  
    printf(testSendMsg() + "\n");
    return 1;
}


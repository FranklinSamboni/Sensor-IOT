#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "GPIO/gpio.h"

gpioParams drdy10; // para DRDY activo bajo - in

int main(){

	initGPIO(10, &drdy10);
	setDirection(INPUT, &drdy10);

	int count = 0;
	while(1){
		if(getValue(&drdy10) == LOW){

			count = count + 1;
			printf("count: %d\n", count);
		}
	}

	return 0;
}

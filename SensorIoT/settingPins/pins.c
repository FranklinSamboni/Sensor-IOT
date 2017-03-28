
/*
 * Programa para configurar los pines automaticamente apenas se inicie el sistema.
 *
 * */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "../libs/GPIO/gpio.h"

gpioParams gpio68_SYNC; // para RTC
gpioParams gpio26_PPS; // para GPS

gpioParams start20; // para START - out
gpioParams reset9; // para RESET - out
gpioParams drdy10; // para DRDY activo bajo - in

gpioParams led81; // led5 - out
gpioParams led73; // led1 - out
gpioParams led70; // led3 - out
gpioParams led71; // led2 - out

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd = 0;

	initGPIO(68, &gpio68_SYNC);
	setDirection(INPUT, &gpio68_SYNC);

	initGPIO(26, &gpio26_PPS);
	setDirection(INPUT, &gpio26_PPS);

	initGPIO(9, &reset9);
	setDirection(OUTPUT, &reset9);
	// RESET debe ser alto para que el adc funcione
	setValue(HIGH,&reset9);

	initGPIO(20, &start20);
	setDirection(OUTPUT, &start20);
	// como vamos a realizar configuración de los registro mantenemos START en bajo
	setValue(LOW,&start20);

	initGPIO(10, &drdy10);
	setDirection(INPUT, &drdy10); // para leer DRDY activo bajo
	//setValue(LOW,&drdy10);

	initGPIO(81, &led81);
	setDirection(OUTPUT, &led81);
	setValue(HIGH,&led81);

	initGPIO(73, &led73);
	setDirection(OUTPUT, &led73);
	setValue(HIGH,&led73);

	initGPIO(70, &led70);
	setDirection(OUTPUT, &led70);
	setValue(HIGH,&led70);

	initGPIO(71, &led71);
	setDirection(OUTPUT, &led71);
	setValue(HIGH,&led71);

	return 0;
}

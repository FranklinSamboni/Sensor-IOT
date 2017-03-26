
/*
 * ADS1262
 *
 * Comando de lectura 001r rrrr (20h+000r rrrr)  -- 000n nnnn
 * Comando de escritura 010r rrrr (40h+000r rrrr) -- 000n nnnn
 *
 * Donde r rrrr = es direccion del registro.
 * n nnnn = numero de registros a leer o escribir menos 1.
 * */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "../GPIO/gpio.h"

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

	char cc;
	char ss;
	char vv;
	while(1){
		printf("\n Opciones del programa: \n");
		printf("Cambiar pin Start: s\n");
		printf("Leer pin Start: t\n");
		printf("Cambiar pin RESET: r\n");
		printf("Leer pin RESET: p\n");
		printf("Ingresar opcion: ");

		scanf(" %c", &cc);

		printf("opcion ingresada: %c\n", cc);
		sleep(1);
		printf("\033[2J");
		switch (cc){
		case 's':

			printf("\n Ingresar valor para Start h o l: \n");

			scanf(" %c", &ss);

			if(ss == 'h'){
				setValue(HIGH,&start20);
			}
			else if(ss == 'l'){
				setValue(LOW,&start20);
			}
			else {
				printf("%c no es un valor correcto\n",ss);
			}

			break;

		case 't':

			if(getValue(&start20) == HIGH){
				printf("Valor de Start: 1\n");
			}
			else if(getValue(&start20) == LOW){
				printf("Valor de Start: 0\n");
			}
			else{
				printf("Error Start\n");
			}

			break;

		case 'r':
			printf("\n Ingresar valor para RESET h o l: \n");

			scanf(" %c", &vv);

			if(vv == 'h'){
				setValue(HIGH,&reset9);
			}
			else if(vv == 'l'){
				setValue(LOW,&reset9);
			}
			else {
				printf("%c no es un valor correcto\n",vv);
			}
			break;

		case 'p':

			if(getValue(&reset9) == HIGH){
				printf("Valor de RESET: 1\n");
			}
			else if(getValue(&reset9) == LOW){
				printf("Valor de RESET: 0\n");
			}
			else{
				printf("Error RESET\n");
			}

			break;

		default:
			printf("Opcion incorrecta\n");
			break;
		}

	}

	return 0;
}

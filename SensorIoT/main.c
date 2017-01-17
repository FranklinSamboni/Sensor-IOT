#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "GPS/gps.h"
#include "RTC/rtc.h"
#include "GPIO/gpio.h"

#include <stdint.h>
#include <time.h>
//para arhivos


/* retorna "a - b" en segundos */
double timeval_diff(struct timeval *a, struct timeval *b)
{
  return
    (double)(a->tv_sec + (double)a->tv_usec/1000000) -
    (double)(b->tv_sec + (double)b->tv_usec/1000000);
}

#define BILLION 1000000000L

int keepGoing = 1;
int sincronizar = -1;
char currentDirectory[100];

gpioParams gpio23; // para RTC
gpioParams gpio26; // para GPS

gpioParams gpio44;


void signal_handler(int sig);

// Directions INPUT, OUTPUT
// Values LOW, HIGH

int main(){

	//signal(SIGINT, signal_handler);

	int gps = 0, bits;
	int rtc = 0;
	char buf[255] = {0};
	char bufRtc[255] = {0};
	char time[255] = {0};

	initGPIO(23, &gpio23);
	setDirection(INPUT, &gpio23);

	initGPIO(26, &gpio26);
	setDirection(INPUT, &gpio26);

	openUART(1,DEVICE_UART);
	openI2C(DECIVE_I2C);

	//configureSerialPort(3);
	//setFactoryDefaults();
	//configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA)
	//configureNMEA_Messages(1,0,0,0,1,0,0);
	activeAlarmRtc();
	int flag = 0;

	struct timeval t_iniiii, t_finnnn;
	double secssss;

	uint64_t diff;
	struct timespec start, end;
	int i;

	while(keepGoing){



		if(getValue(&gpio26) == HIGH){

			gettimeofday(&t_iniiii, NULL);

			clock_gettime(CLOCK_MONOTONIC, &start);	/* mark start time */

			printf("\n ----- Señal pps ------- \n");
			gps = readUART(buf);
			if(gps != -1){
				while(gps != -1){
					flag = 0;
					if(isGGA(buf) == 1){
						bits = getTimeGps(time,buf);
						time[bits] = 0;
						if(sincronizar == -1){
							sincronizar = setTimeRtc(time); // Modificar tiempo en RTC
						}
					}
					printBuffer(gps,buf);
					saveDataGps(buf,currentDirectory); // Guardar en archivo de texto.
					gps = readUART(buf);
				}
			}
			else{
				if(getValue(&gpio23) == LOW){
					flag = 1;
					printf("Con RTC.\n");
					// Es necesario reinicar la bandera de la alarma en la direccion 0x0F.
					writeI2C(0x0F,0x88);
					//writeI2C(0x0F,0x88);

					rtc = readI2C(bufRtc);
					if(rtc != -1){
						printData(bufRtc);
						saveDataRtc(bufRtc,currentDirectory);
							//getTimeRtc(time,bufRtc);
							//time[bits] = 0;
							//printf("Tiempo RTC %s\n", time);

					}
				}
			}

			gettimeofday(&t_finnnn, NULL);

			clock_gettime(CLOCK_MONOTONIC, &end);	/* mark the end time */

			diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);

			secssss = timeval_diff(&t_finnnn, &t_iniiii);
			printf("%.16g get time of day milliseconds\n", secssss * 1000.0);

		}
		else{
			if(flag == 1){
				if(getValue(&gpio23) == LOW){
					printf("Low pps.\n");
					printf("Con RTC.\n");
					// Es necesario reinicar la bandera de la alarma en la direccion 0x0F.
					writeI2C(0x0F,0x88);
					//writeI2C(0x0F,0x88);

					rtc = readI2C(bufRtc);
					if(rtc != -1){
						printData(bufRtc);
						saveDataRtc(bufRtc,currentDirectory);
							//getTimeRtc(time,bufRtc);
							//time[bits] = 0;
							//printf("Tiempo RTC %s\n", time);

					}
				}
			}
		}

	}
	closeUART();
	closeI2C();
	return 0;
}

void signal_handler(int sig){
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepGoing = 0;
	destroyGPIO(&gpio23);
	destroyGPIO(&gpio26);
	desactiveAlarmRtc();
	closeUART();
	closeI2C();
}


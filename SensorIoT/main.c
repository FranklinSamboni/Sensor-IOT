#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "GPS/gps.h"
#include "RTC/rtc.h"
#include "GPIO/gpio.h"

int keepGoing = 1;
int sincronizar = -1;
gpioParams gpio66;

void signal_handler(int sig);

// Directions INPUT, OUTPUT
// Values LOW, HIGH

int main(){

	int res = 0, rtc=0, bits;
	char buf[255] = {0};
	char bufRtc[255] = {0};
	char time[255] = {0};


	initGPIO(66, &gpio66);
	setDirection(INPUT, &gpio66);

	signal(SIGINT, signal_handler);

	openUART(1,DEVICE_UART);
	openI2C(DECIVE_I2C);

	//configureSerialPort(3);
	//setFactoryDefaults();
	//configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA)
	//configureNMEA_Messages(1,0,0,0,1,0,0);

	while(keepGoing){

		if(getValue(&gpio66) == HIGH){
			printf("\n ----- Señal pps ------- \n");
			res = readUART(buf);

			while(res != -1){

				if(isGGA(buf) == 1){
					printBuffer(res,buf);
					bits = getTimeGps(time,buf);
					time[bits] = 0;
					printf("Tiempo GPS: %s\n", time);

					if(sincronizar == -1){
						sincronizar = setTimeRtc(time); // Modificar tiempo en RTC
					}

					rtc = readI2C(bufRtc);
					if(rtc != -1){
						saveDataRtc(rtc,bufRtc);
						bits = getTimeRtc(time);
						if(bits!=-1){
							time[bits] = 0;
							printf("Tiempo RTC %s\n", time);
						}
					}
				}


				saveDataGps(buf); // Guardar en archivo de texto.
				res = readUART(buf);
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
	destroyGPIO(&gpio66);
	closeUART();
	closeI2C();

}

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ADC/adc.h"

int main(){

	FILE *archivo;

	archivo = fopen ("archivo.txt", "w+");
	char recvBuf[6] = {0x00,};
	char recv2_3[6] = {0x00,};
	char recv4_5[6] = {0x00,};
	char recv6_7[6] = {0x00,};
	int result = 0;
	double volt = 0;

	settingPins();

	if (openSPI(DECIVE_SPI) < 0){
		exit(0);
	}

	settingADC();
	int count = 0;
	time_t inicio,fin;
	double dif = 0.0;
	double xx,yy,zz;

	while(1){
		printf("inicio count %d\n", count);
		if(count == 0){
			inicio = time(NULL);
		}

		readAIN2_3(recv2_3);
		readAIN4_5(recv4_5);
		readAIN6_7(recv6_7);
		//printf("\n");
		xx = getVoltage(recv2_3);
		yy = getVoltage(recv4_5);
		zz = getVoltage(recv6_7);

		fprintf(archivo,"Counter: %d | X: %lf | Y:%lf | Z: %lf\n",count, xx,yy,zz);

		count++;
		fin = time(NULL);
		dif = difftime(fin,inicio);
		//printf("\ntime: %lf",dif);
		if(difftime(fin,inicio) == 1.0){
			count = 0;
		}
	}
	closeSPI();
	fclose(archivo);


	return 0;
}

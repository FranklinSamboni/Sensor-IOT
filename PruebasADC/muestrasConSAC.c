#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include "ADC/adc.h"
#include "sacsubc.h"

void outputsac(int npts, int numData, float *arr, float dt, char *axis ,char *filename);

int main(){

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
/*
	if(readRegister(recvBuf , POWER) > 1){
			printf("valor leido POWER: %.2X \n",recvBuf[0]);
			usleep(10);
		}

		if(readRegister(recvBuf , MODE0) > 1){
			printf("valor leido MODE0 : %.2X \n",recvBuf[0]);
			usleep(10);
		}

		if(readRegister(recvBuf , MODE1) > 1){
			printf("valor leido MODE1: %.2X \n",recvBuf[0]);
			usleep(10);
		}

		if(readRegister(recvBuf , MODE2) > 1){
			printf("valor leido MODE2 : %.2X \n",recvBuf[0]);
			usleep(10);
		}

		if(readRegister(recvBuf , INPMUX) > 1){
			printf("valor leido INPMUX: %.2X \n",recvBuf[0]);
			usleep(10);
		}

		if(readRegister(recvBuf , REFMUX) > 1){
			printf("valor leido REFMUX : %.2X \n",recvBuf[0]);
			usleep(10);
		}
		*/
	settingADC();

	int count = 0;
	time_t inicio,fin;
	double dif = 0.0;
	double xx,yy,zz;

	char fileNameX[] = "BH2muestras.sac";
	char fileNameY[] = "BH1muestras.sac";
	char fileNameZ[] = "BHZmuestras.sac";

	float dataX[200] = {0};
	float dataY[200] = {0};
	float dataZ[200] = {0};
    float dt = 0.005 ;
    int npts = 0;
    int numData = 200;
    int offset = 10;

    char axisX[] = "BH2";
    char axisY[] = "BH1";
    char axisZ[] = "BHZ";

    float adc_countX = 0;
    float adc_countY = 0;
    float adc_countZ = 0;
	createFile(fileNameX);
	createFile(fileNameY);
	createFile(fileNameZ);

	while(1){
		//printf("inicio count %d\n", count);
		if(count == 0){
			inicio = time(NULL);
		}

		if(count < 200){
			readAIN2_3(recv2_3);
			readAIN2_3(recv4_5);
			readAIN2_3(recv6_7);

			adc_countX = (float) (((unsigned long)recv2_3[1]<<24)|((unsigned long)recv2_3[2]<<16)|(recv2_3[3]<<8)|recv2_3[4]);
			adc_countY = (float) (((unsigned long)recv4_5[1]<<24)|((unsigned long)recv4_5[2]<<16)|(recv4_5[3]<<8)|recv4_5[4]);
			adc_countZ = (float) (((unsigned long)recv6_7[1]<<24)|((unsigned long)recv6_7[2]<<16)|(recv6_7[3]<<8)|recv6_7[4]);

			dataX[count] = adc_countX;
			dataY[count] = adc_countY;
			dataZ[count] = adc_countZ;
			printf("count es: %d  -- data: %lf, data: %lf, data: %lf\n",count, adc_countX, adc_countY, adc_countZ);
		}

		count++;
		fin = time(NULL);
		dif = difftime(fin,inicio);


		if(difftime(fin,inicio) == 1.0){
			if(count > 200){
				npts = npts + numData;
				//data[offset] = 1.0/dt ;
				printf("Comenzando\n");
				outputsac(npts,numData,dataX,dt,axisX,fileNameX);
				outputsac(npts,numData,dataY,dt,axisY,fileNameY);
				outputsac(npts,numData,dataZ,dt,axisZ,fileNameZ);
				count = 0;
				printf("Actualizo\n");

			}else{
				count = 0;
			}
		}
	}
	closeSPI();

	return 0;
}

void outputsac(int npts, int numData, float *arr, float dt, char *axis ,char *filename)
{
        /* create the SAC file
           instead of using the wsac1 I will use the lower level
           routines to provide more control on the output */
        int nerr;
        float b, e, depmax, depmin, depmen;
        /* get the extrema of the trace */
        printf("antes de scmxmn\n");
        		//scmxmn(arr,npts,&depmax,&depmin,&depmen);
                scmxmn(arr,numData,&depmax,&depmin,&depmen);
        printf("despues de scmxmn y antes de newhdr\n");
        /* create a new header for the new SAC file */
                newhdr();
        printf("despues de newhdr y antes de los set \n");
        /* set some header values */
                setfhv("DEPMAX", depmax, &nerr);
                setfhv("DEPMIN", depmin, &nerr);
                setfhv("DEPMEN", depmen, &nerr);
                setnhv("NPTS    ",npts,&nerr);
                setfhv("DELTA   ",dt  ,&nerr);
                b = 0;
                setfhv("B       ",b  ,&nerr);
                setihv("IFTYPE  ","ITIME   ",&nerr);
                e = b + (npts -1 )*dt;
                setfhv("E       ",e     ,&nerr);
                setlhv("LEVEN   ",1,&nerr);
                setlhv("LOVROK  ",1,&nerr);
                setlhv("LCALDA  ",1,&nerr);
        /* put is a default time for the plot */
                setnhv("NZYEAR", 2017, &nerr);
                setnhv("NZJDAY", 2, &nerr);
                setnhv("NZHOUR", 1, &nerr);
                setnhv("NZMIN" , 1, &nerr);
                setnhv("NZSEC" , 1, &nerr);
                setnhv("NZMSEC", 1, &nerr);

                setkhv("KNETWK", "MEC",&nerr);
                setkhv("KSTNM", "POP",&nerr);
                setkhv("KCMPNM", axis,&nerr);
        /* output the SAC file */
         printf("despues de los set y antes de updateHeaders \n");
                updateHeaders(filename);
        printf("despues de updateHeaders y antes de updateData \n");
        		updateData(filename,numData,arr);
                //updateData(filename,npts,arr);
                //bwsac(npts,filename,arr);
                printf("metodo\n");
}

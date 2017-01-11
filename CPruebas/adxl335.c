/***************************************
#* adxl335.c
#*
#* This file demonstrates the usage of the ADXL335 accelerometer.
#* With the proper accelerometer axis inputs defined below,
#* this application will output and constantly update the readings
#* from the accelerometer
#*
***************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
//#include <sys/time.h>
#include <fcntl.h>
#include <time.h>

#define ANINX 0
#define ANINY 2
#define ANINZ 6

int keepGoing = 1;
FILE *archivo;
int fpx,fpy,fpz;

void signal_handler(int sig){

	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepGoing = 0;
	//fclose(archivo);
}
/*	int read_anin(char * fileName)
*
*	The argument is the fileName instead of the integer because this will be called a lot for only 3 filenames,
*	so we want to save ourselves the work of constructing the filename each time.
*/

int read_anin(char * fileName){
	/*FILE *fp;
	char readValue[5];
	if ((fp = fopen(fileName,  "r")) == NULL) {
		printf("Cannot open anin file: %s.\n", fileName);
		exit(1);
	}
	//Set pointer to begining of the file
	rewind(fp);
	//Write our value of "out" to the file
	fread(readValue, sizeof(char), 10, fp);
	readValue[4] = '\0'; //for some reason when reading 4 digit numbers you get weird garbage after the value
	fclose(fp);
	return atoi(readValue);*/
	int resultado;
	char buffer[255];
	int fp;
	fp = open(fileName,O_RDONLY);
	if(fp < 0){
		printf("Cannot open anin file: %s.\n", fileName);
		exit(1);
	}

	resultado = read(fp,buffer,255);
	if(resultado > 0){
		buffer[resultado] = '\0';
		close(fp);
		//printf("!resultado: %d!-",resultado);
		return atoi(buffer);

	}else{
		printf("Error leyendo el dispositivo: %s. resultado: %d\n", fileName,resultado);
		return 0;
		//exit(1);
	}
}


int main(int argc, char **argv, char **envp){

	char fileNameX[50], fileNameY[50], fileNameZ[50];
	sprintf(fileNameX, "/sys/devices/ocp.3/helper.16/AIN%d", ANINX);
	sprintf(fileNameY, "/sys/devices/ocp.3/helper.16/AIN%d", ANINY);
	sprintf(fileNameZ, "/sys/devices/ocp.3/helper.16/AIN%d", ANINZ);
	signal(SIGINT, signal_handler);
	archivo = fopen ("archivo.txt", "w+");

	int xx=0;
	int yy=0;
	int zz=0;

	int counter = 0;
	time_t inicio,fin;
	double dif = 0.0;
	while(keepGoing){
		if(counter == 0){
			inicio = time(NULL);
		}
		xx = read_anin(fileNameX);
		yy = read_anin(fileNameY);
		zz = read_anin(fileNameZ);
		//printf("Counter: %d | X: %d | Y: %d | Z: %d\n",counter,xx,yy,zz);
		fprintf(archivo,"Counter: %d | X: %d | Y: %d | Z: %d\n",counter, xx,yy,zz);
		//usleep(5); // usleep delay en microsegundos 
		//sleep(1);
		counter = counter + 1;
		fin = time(NULL);
		dif = difftime(fin,inicio);
		//printf("\ntime: %lf",dif);
		if(difftime(fin,inicio) == 1.0){
			counter = 0;
		}
	}
	fclose(archivo);
}

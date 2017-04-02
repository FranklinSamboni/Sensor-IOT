#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <json/json.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "libs/ADC/adc.h"
#include "libs/GPS/gps.h"
#include "libs/RTC/rtc.h"
#include "libs/GPIO/gpio.h"
#include "libs/SOCKET/socketlib.h"

/*
 * Directions INPUT, OUTPUT
 * Values LOW, HIGH
 * 	*/

#define SAMPLES_DIR_R "muestras"
#define BILLION 1000000000L

char currentDirectory[100] = {0};

gpioParams gpio68_SYNC; // para RTC
gpioParams gpio26_PPS; // para GPS

void signal_handler(int sig);

/*
 *  Obtiene y envia la información inicial del GPS:
 *  - Latitud
 *  - Longitud
 *  - Altitud
 *  - Fecha
 *  - Tiempo
 *   */
void loadingGpsData();

/*
 * Verifica la existencia de la señal PPS.
 * */
void checkingPPS();

/*
 * Activa la alarma cada segundo del RTC (Señal SYNC).
 * Sincroniza RTC y GPS.
 * */
void settingRtC();

int getStatus(json_object * jobj);

void readAndSaveData();

void readWithRTC();

int readAnalogInputsAndSaveData(char * date, char * time);

void createDirRtc(char *dir, char * date, char *time);

int main(){

	//signal(SIGINT, signal_handler);

	int gps = 0, bits;
	int rtc = 0;
	char bufSock[255] = {0};
	char buf[255] = {0};
	char bufRtc[255] = {0};
	char time[255] = {0};

	initGPIO(68, &gpio68_SYNC);
	setDirection(INPUT, &gpio68_SYNC);

	initGPIO(26, &gpio26_PPS);
	setDirection(INPUT, &gpio26_PPS);

	// Conectando al servidor

	if(openSOCKET(SERVER_IP,SOCKET_PORT)< 0){
		exit(0);
	}

	//TASA DE BAUDIOS D3 DE = B115200
	if(openUART(3,DEVICE_UART)< 0){
		exit(0);
	}

	if(openI2C(DECIVE_I2C)< 0){
		exit(0);
	}

	if (openSPI(DECIVE_SPI) < 0){
		exit(0);
	}

	settingPins(); // configurar pines de control del ADC
	settingADC();

	//setFactoryDefaults();
	loadingGpsData();

	checkingPPS();

	settingRtC();

	//writeSOCKET("Estado: Ready.\n");

	int fsc = 0;
	while(1){
		printf ("Antes\n");
		fsc = readSOCKET(bufSock);
		printf ("Despues\n");
		if(fsc != -1 ){
			printf ("JSON string: %s.\n", bufSock);
			json_object * jobj = json_tokener_parse(bufSock);
			printf ("Status: %d.\n", getStatus(jobj));
			readAndSaveData();
		}else{
			printf ("Error socket.\n");
		}
		printf ("Final\n");

	}

	destroyGPIO(&gpio68_SYNC);
	destroyGPIO(&gpio26_PPS);
	desactiveAlarmRtc();
	closeUART();
	closeI2C();
	closeSOCKET();
	closeSPI();

	return 0;
}

void signal_handler(int sig){
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	destroyGPIO(&gpio68_SYNC);
	destroyGPIO(&gpio26_PPS);
	desactiveAlarmRtc();
	closeUART();
	closeI2C();
	closeSOCKET();
}

void loadingGpsData(){

	int res = 0, flag = 0;
	char buf[255] = {0};
	char lat[24] = {0}, lng[24] = {0}, alt[24] = {0};
	char dateGps[24] = {0}, timeGps[24] = {0};

	json_object * jobj = json_object_new_object();

	//configureSerialPort(3); // Configurar tasa de baudios GPS a 115200
	sleep(1); // waiting for GPS
	//configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA)
	configureNMEA_Messages(1,0,0,0,1,0,0);
	sleep(1); // waiting for GPS

	json_object * jstatus = json_object_new_object();
	json_object *js = json_object_new_int(1);
	json_object *jmsg = json_object_new_string("Cargando datos del GPS.");

	json_object_object_add(jstatus,"status", js);
	json_object_object_add(jstatus,"msg", jmsg);

	writeSOCKET(json_object_to_json_string(jstatus));
	sleep(1);

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);


	while(1){
		res = readUART(buf);

		if(res != -1){
			inicio = time(NULL);
			printBuffer(res,buf);
			if(isGGA(buf) == 1){
				if(getAlt(alt,buf) != -1) {
					flag = 1;
				}
			}
			else if(isRmcStatusOk(buf) == 1){
				getLat(lat,buf);
				getLng(lng,buf);
				getDateGps(dateGps,buf);
				getTimeGps(timeGps,buf);
				if(flag == 1){

					js = json_object_new_int(2);
					json_object *jlat = json_object_new_string(lat);
					json_object *jlng = json_object_new_string(lng);
					json_object *jalt = json_object_new_string(alt);
					json_object *jdate = json_object_new_string(dateGps);
					json_object *jtime = json_object_new_string(timeGps);


					json_object_object_add(jobj,"status", js);
					json_object_object_add(jobj,"lat", jlat);
					json_object_object_add(jobj,"lng", jlng);
					json_object_object_add(jobj,"alt", jalt);
					json_object_object_add(jobj,"date", jdate);
					json_object_object_add(jobj,"time", jtime);

					writeSOCKET(json_object_to_json_string(jobj));
					sleep(1);
					break;
				}
			}
			else{
				writeSOCKET(json_object_to_json_string(jstatus));
				sleep(1);
			}
		}
		else{
			fin  = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){
					js = json_object_new_int(-1);
					jmsg = json_object_new_string("Revisa la conexión del GPS.");
					writeSOCKET(json_object_to_json_string(jstatus));
					sleep(1);
				}
				diff = difftime(fin,inicio);
			}
		}
	}

}

void checkingPPS(){

	int cont = 0;
	json_object * jobj = json_object_new_object();

	json_object * jstatus = json_object_new_object();
	json_object *js = json_object_new_int(3);
	json_object *jmsg = json_object_new_string("Verificando señal PPS.");

	json_object_object_add(jstatus,"status", js);
	json_object_object_add(jstatus,"msg", jmsg);

	writeSOCKET(json_object_to_json_string(jstatus));

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);

	while(1){
		if(getValue(&gpio26_PPS) == HIGH){
			inicio = time(NULL);
			if(cont == 5){

				json_object *jboolean = json_object_new_boolean(1);
				js = json_object_new_int(4);

				json_object_object_add(jobj,"status", js);
				json_object_object_add(jobj,"pps", jboolean);

				writeSOCKET(json_object_to_json_string(jobj));

				sleep(1);
				// Salida del blucle
				break;
			}
			cont++;
		}else{
			fin = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){
					js = json_object_new_int(-1);
					jmsg = json_object_new_string("Verifica la conexión de la señal PPS o espera una respuesta.");
					writeSOCKET(json_object_to_json_string(jstatus));
				}
				diff = difftime(fin,inicio);
			}
		}
	}

}

void settingRtC(){

	int res = 0, sync = 0;
	int cont = 0;
	char buf[255] = {0};
	char timeBuf[24] = {0}, dateBuf[24] = {0};
	json_object * jobj = json_object_new_object();


	json_object * jstatus = json_object_new_object();
	json_object *js = json_object_new_int(5);
	json_object *jmsg = json_object_new_string("Activando señal SYNC y sincronizando RTC.");

	json_object_object_add(jstatus,"status", js);
	json_object_object_add(jstatus,"msg", jmsg);

	writeSOCKET(json_object_to_json_string(jstatus));

	activeAlarmRtc();

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);

	while(1){
		if(getValue(&gpio26_PPS) == HIGH){
			res = readUART(buf);
			if(res != -1){
				if(isRmcStatusOk(buf) == 1){
					getTimeGps(timeBuf,buf); // configurando Hora
					setTimeRtc(timeBuf); // Sincroniza RTC y GPS.

					getDateGps(dateBuf,buf); // configurando fecha
					setDateRtc(dateBuf);
					sync = 1;
				}
			}
		}
		else{
			if(getValue(&gpio68_SYNC) == LOW){
				inicio = time(NULL);

				// Es necesario reinicar la bandera de la alarma en la direccion 0x0F.
				writeI2C(0x0F,0x88);

				if(sync == 1 && cont > 4){

					res = readI2C(buf);
					if(res != -1){

						getTimeRtc(timeBuf,buf);
						getDateRtc(dateBuf,buf);

						js = json_object_new_int(6);
						json_object *jboolean = json_object_new_boolean(1);
						json_object *jtime = json_object_new_string(timeBuf);
						json_object *jdate = json_object_new_string(dateBuf);

						json_object_object_add(jobj,"status", js);
						json_object_object_add(jobj,"sync", jboolean);
						json_object_object_add(jobj,"time", jtime);
						json_object_object_add(jobj,"date", jdate);

						writeSOCKET(json_object_to_json_string(jobj));
						sleep(1);

						 //  Salida del bucle
						break;
					}
				}
				cont++;
			}
			else{
				fin = time(NULL);
				if(difftime(fin,inicio) > 20.0){
					if(difftime(fin,inicio) > diff){
						json_object *js = json_object_new_int(-1);
						json_object *jmsg = json_object_new_string("Verifica la conexión de SQW (pin de SYNC) del RTC o espera una respuesta.");
						writeSOCKET(json_object_to_json_string(jstatus));
					}
					diff = difftime(fin,inicio);
				}
			}
		}
	}

}

int getStatus(json_object * jobj) {

	enum json_type type;
	int status = -1;
	json_object_object_foreach(jobj, key, val) {
		type = json_object_get_type(val);
		switch (type) {
		case json_type_int:
			//printf("type: json_type_int, ");
			//printf("value: %dn", json_object_get_int(val));
			status = json_object_get_int(val);
			break;
		}
	}
	return status;
}

void readAndSaveData(){

	char buf[255] = {0};
	char bufTime[15] = {0}, bufDate[10] = {0}, bufLat[15] = {0};
	int flag = 0;
	int gps = 0;
	int isData = 0;

	uint64_t diff;
	struct timespec start, end;

	time_t inicio,fin;
	double count_PPS = 0.0;

	inicio = time(NULL);

	while(1){


		if(getValue(&gpio26_PPS) == HIGH){
			inicio = time(NULL);
			clock_gettime(CLOCK_MONOTONIC, &start);

			printf("\n ----- Senial pps ------- \n");
			gps = readUART(buf);
			if(gps != -1){
				while(gps != -1){
					flag = 0;
					printBuffer(gps,buf);
					if(isRMC(buf) == 1) {
						getTimeGps(bufTime,buf);
						getDateGps(bufDate,buf);

						isData = getLat(bufLat,buf);
						break;
						//bits = getLat(data.lat,buffer);
						//bits = getLng(data.lng,buffer);
					}
					//saveDataGps(buf,currentDirectory); // Guardar en archivo de texto.
					gps = readUART(buf);
				}
				if(isData != -1){
					if(bufDate[0] != 0 && bufTime[0] != 0){
						readAnalogInputsAndSaveData(bufDate, bufTime);
					}
				}
				else{
					if(getValue(&gpio68_SYNC) == LOW){
						flag = 1;
						readWithRTC();
					}
				}
			}
			else{
				if(getValue(&gpio68_SYNC) == LOW){
					flag = 1;
					readWithRTC();
				}
			}

			clock_gettime(CLOCK_MONOTONIC, &end);

			diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			//printf("Tiempo nanosegundos = %llu ns\n", (long long unsigned int) diff);
			printf("Tiempo milisegundos = %llu ms\n", (long long unsigned int) diff/1000000);
		}
		else{
			fin = time(NULL);
			count_PPS = difftime(fin,inicio);

			if(flag == 1 || count_PPS > 4.0){
			//if(flag == 1){
				if(getValue(&gpio68_SYNC) == LOW){
					clock_gettime(CLOCK_MONOTONIC, &start);
					printf("Low pps.\n");

					readWithRTC();
					clock_gettime(CLOCK_MONOTONIC, &end);
					diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
					printf("Tiempo milisegundos = %llu ms\n", (long long unsigned int) diff/1000000);
				}
			}
		}
	}

}

void readWithRTC(){
	char bufRtc[255] = {0};
	char time[10] = {0};
	char date[10] = {0};
	int rtc = 0;
	printf("Con RTC.\n");
	// Es necesario reinicar la bandera de la alarma en la direccion 0x0F.
	writeI2C(0x0F,0x88);
	//writeI2C(0x0F,0x88);

	rtc = readI2C(bufRtc);
	if(rtc != -1){
		printData(bufRtc);
		getTimeRtc(time,bufRtc);
		getDateRtc(date,bufRtc);
		readAnalogInputsAndSaveData(date,time);
	}
}

int readAnalogInputsAndSaveData(char * date, char * time){

	char recvX[6] = {0x00,};
	char recvY[6] = {0x00,};
	char recvZ[6] = {0x00,};

	double xx = 0, yy = 0, zz = 0;

	createDirRtc(currentDirectory, date, time);
	int count = 0;
	FILE * sampleFile = fopen (currentDirectory, "a");
	fprintf(sampleFile,"date: %s, hour: %s\n",date, time);

	/*if(getValue(&gpio26_PPS) == HIGH){
		printf("PPS ES ALTA");
		return 0;
	}
	else{
		printf("PPS ES BAJA");
		return 0;
	}*/
	printf("Capturando datos ADC\n");
	while(count < 200){

		//printf("inicio count %d\n", count);

		readAIN2_3(recvX);
		readAIN4_5(recvY);
		readAIN6_7(recvZ);
		//printf("\n");
		xx = getVoltage(recvX);
		yy = getVoltage(recvY);
		zz = getVoltage(recvZ);

		fprintf(sampleFile,"Counter: %d | X: %lf | Y:%lf | Z: %lf\n",count, xx,yy,zz);
		count++;

	}
	printf("Termino camptura de datos ADC\n");
	fclose(sampleFile);
	return 0;
}



void createDirRtc(char *dir, char * date, char *time){
	char fecha[100] = {0};
	struct stat st = {0};
	sprintf(fecha,"%s/%s",SAMPLES_DIR_R,date);

	if (stat(SAMPLES_DIR_R, &st) == -1) {
	    mkdir(SAMPLES_DIR_R, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	if (stat(fecha, &st) == -1) {
	    mkdir(fecha, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	if(dir[0] == 0){
		sprintf(dir,"%s/%s/%s_%c%c0000.txt",SAMPLES_DIR_R,date,date,time[0],time[1]);
		FILE *archivo = fopen (dir, "a");
		fclose(archivo);
	}
	else if (time[2]=='0' && time[3]=='0' && time[4]=='0' && time[5]=='0'){ //Nueva Hora
		sprintf(dir,"%s/%s/%s_%c%c%c%c%c%c.txt",SAMPLES_DIR_R,date,date,time[0],time[1],time[2],time[3],time[4],time[5]);
		FILE *archivo = fopen (dir, "a");
		fclose(archivo);
	}
	printf("Dir: %s\n", dir);
}

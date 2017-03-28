#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <json/json.h>
#include <stdint.h>
#include <time.h>
#include "libs/ADC/adc.h"
#include "libs/GPS/gps.h"
#include "libs/RTC/rtc.h"
#include "libs/GPIO/gpio.h"
#include "libs/SOCKET/socketlib.h"

/*
 * Directions INPUT, OUTPUT
 * Values LOW, HIGH
 * 	*/

#define BILLION 1000000000L

int keepGoing = 1;
int sincronizar = -1;
char currentDirectory[100];

gpioParams gpio68_SYNC; // para RTC
gpioParams gpio26_PPS; // para GPS

void signal_handler(int sig);

/*
 *  Obtiene y envia la informaci�n inicial del GPS:
 *  - Latitud
 *  - Longitud
 *  - Altitud
 *  - Fecha
 *  - Tiempo
 *   */
void loadingGpsData();

/*
 * Verifica la existencia de la se�al PPS.
 * */
void checkingPPS();

/*
 * Activa la alarma cada segundo del RTC (Se�al SYNC).
 * Sincroniza RTC y GPS.
 * */
void settingRtC();

int getStatus(json_object * jobj);

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

	openSOCKET(SERVER_IP,SOCKET_PORT); // Conectando al servidor
	openUART(3,DEVICE_UART);
	openI2C(DECIVE_I2C);

	//setFactoryDefaults();
	loadingGpsData();

	checkingPPS();

	settingRtC();

	//writeSOCKET("Estado: Ready.\n");

	int fsc = 0;
	while(keepGoing){
		printf ("Antes\n");
		fsc = readSOCKET(bufSock);
		printf ("Despues\n");
		if(fsc != -1 ){
			printf ("JSON string: %s.\n", bufSock);
			json_object * jobj = json_tokener_parse(bufSock);
			printf ("Status: %d.\n", getStatus(jobj));
		}else{
			printf ("Error socket.\n");
		}
		printf ("Final\n");

	}

	keepGoing = 0;
	destroyGPIO(&gpio68_SYNC);
	destroyGPIO(&gpio26_PPS);
	desactiveAlarmRtc();
	closeUART();
	closeI2C();
	closeSOCKET();

	return 0;
}

void signal_handler(int sig){
	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepGoing = 0;
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
	usleep(5000000);

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);


	while(keepGoing){
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
					usleep(250000);
					keepGoing = 0;
					break;
				}
			}
			else{
				writeSOCKET(json_object_to_json_string(jstatus));
				usleep(5000000);
			}
		}
		else{
			fin  = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){
					js = json_object_new_int(-1);
					jmsg = json_object_new_string("Revisa la conexi�n del GPS.");
					writeSOCKET(json_object_to_json_string(jstatus));
					usleep(5000000);
				}
				diff = difftime(fin,inicio);
			}
		}
	}

	keepGoing = 1;

}

void checkingPPS(){

	int cont = 0;
	json_object * jobj = json_object_new_object();

	json_object * jstatus = json_object_new_object();
	json_object *js = json_object_new_int(3);
	json_object *jmsg = json_object_new_string("Verificando se�al PPS.");

	json_object_object_add(jstatus,"status", js);
	json_object_object_add(jstatus,"msg", jmsg);

	writeSOCKET(json_object_to_json_string(jstatus));

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);

	while(keepGoing){
		if(getValue(&gpio26_PPS) == HIGH){
			inicio = time(NULL);
			if(cont == 5){

				json_object *jboolean = json_object_new_boolean(1);
				js = json_object_new_int(4);

				json_object_object_add(jobj,"status", js);
				json_object_object_add(jobj,"pps", jboolean);

				writeSOCKET(json_object_to_json_string(jobj));

				usleep(250000);
				keepGoing = 0; // Salida del blucle
				break;
			}
			cont++;
		}else{
			fin = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){
					js = json_object_new_int(-1);
					jmsg = json_object_new_string("Verifica la conexi�n de la se�al PPS o espera una respuesta.");
					writeSOCKET(json_object_to_json_string(jstatus));
				}
				diff = difftime(fin,inicio);
			}
		}
	}
	keepGoing = 1;
}

void settingRtC(){

	int res = 0, sync = 0;
	int cont = 0;
	char buf[255] = {0};
	char timeBuf[24] = {0}, dateBuf[24] = {0};
	json_object * jobj = json_object_new_object();


	json_object * jstatus = json_object_new_object();
	json_object *js = json_object_new_int(5);
	json_object *jmsg = json_object_new_string("Activando se�al SYNC y sincronizando RTC.");

	json_object_object_add(jstatus,"status", js);
	json_object_object_add(jstatus,"msg", jmsg);

	writeSOCKET(json_object_to_json_string(jstatus));

	activeAlarmRtc();

	time_t inicio, fin;
	int diff = 0;
	inicio = time(NULL);

	while(keepGoing){
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
						usleep(250000);

						keepGoing = 0; //  Salida del bucle
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
						json_object *jmsg = json_object_new_string("Verifica la conexi�n de SQW (pin de SYNC) del RTC o espera una respuesta.");
						writeSOCKET(json_object_to_json_string(jstatus));
					}
					diff = difftime(fin,inicio);
				}
			}
		}
	}
	keepGoing = 1;

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
	/*
		int flag = 0;

		uint64_t diff;
		struct timespec start, end;
		int i;

		while(keepGoing){

			if(getValue(&gpio26_PPS) == HIGH){

				clock_gettime(CLOCK_MONOTONIC, &start);

				printf("\n ----- Senial pps ------- \n");
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
					if(getValue(&gpio68_SYNC) == LOW){
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

				clock_gettime(CLOCK_MONOTONIC, &end);

				diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
				printf("Tiempo nanosegundos = %llu ns\n", (long long unsigned int) diff);
				printf("Tiempo milisegundos = %llu ms\n", (long long unsigned int) diff/1000000);

			}
			else{
				if(flag == 1){
					if(getValue(&gpio68_SYNC) == LOW){
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
	*/
}
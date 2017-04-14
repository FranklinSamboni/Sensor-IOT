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
#include "libs/SAC_FILES/sacsubc.h"
/*
 * Directions INPUT, OUTPUT
 * Values LOW, HIGH
 * 	*/

/*
 * Usar 0x?A o 0x?B en el registro MODE2 del adc para conseguir valores de 200 muestras por segundo.
 * Usar 0x?9 en el registro MODE2 del adc para conseguir valores de 160 muestras por segundo.
 * Usar 0x?8 en el registro MODE2 del adc para conseguir valores de 80 muestras por segundo.
 */

#define CORRECT_STATUS_COMPONENT  "Correcto"
#define ERROR_STATUS_COMPONENT  "Error"

#define SAMPLES_DIR_R "muestras"
#define BILLION 1000000000L

#define MAX_SPS 400
#define SPS 200
#define DT 0.005

//#define SPS 200
//#define DT 0.005

char currentDirectoryX[100] = {0};
char currentDirectoryY[100] = {0};
char currentDirectoryZ[100] = {0};

char axisX[] = "BH2";
char axisY[] = "BH1";
char axisZ[] = "BHZ";

/*float dt = 0.005 ;
int npts = 0;
int dataNumber = 200;*/

// de la libreria de sacsub.h se agregaron para mantener orden en los datos.
fullDate strFullDate;
depValues strDepValues;

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
void componentsFile();
int getStatus(json_object * jobj);
void readAndSaveData();
void readWithRTC();
int readAnalogInputsAndSaveData(char * date, char * time, int isGPS);
void createDirRtc(char *dir, char *axis,char * date, char *time, int isGPS);
void writeSac(int npts, int numData, float *arr, float dt, char *axis ,char *filename);
void initDataofSamples(char * date, char *time, int isGPS);
void subMuestreo_xxx(float *currentData, float *newData, int factor);

void readDataPrueba(float * dataX, float * dataY, float * dataZ);
void read_prueba_solo_pps();

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
	/*printf("abriendosocket\n");
	if(openSOCKET(SERVER_IP,SOCKET_PORT)< 0){
		exit(0);
	}*/
	printf("abriendo  UART\n");

	//TASA DE BAUDIOS 3 DE = B115200
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
	componentsFile();

	//setFactoryDefaults();
	loadingGpsData();
	checkingPPS();
	settingRtC();

	//writeSOCKET("Estado: Ready.\n");

	int fsc = 0;
	while(1){
 		/*printf ("Antes\n");
		fsc = readSOCKET(bufSock);
		printf ("Despues\n");
		if(fsc != -1 ){
			printf ("JSON string: %s.\n", bufSock);
			json_object * jobj = json_tokener_parse(bufSock);
			printf ("Status: %d.\n", getStatus(jobj));*/
			readAndSaveData();
			//read_prueba_solo_pps();
		/*}else{
			printf ("Error socket.\n");
		}
		printf ("Final\n");*/

	}

	destroyGPIO(&gpio68_SYNC);
	destroyGPIO(&gpio26_PPS);
	desactiveAlarmRtc();
	closeUART();
	closeI2C();
	//closeSOCKET();
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
	//closeSOCKET();
}

void componentsFile(){

	//////////////////// ---- Accelerometer ---- ////////////////////////

	json_object * jobjAccelerometerData = json_object_new_object();

	json_object *status = json_object_new_string(CORRECT_STATUS_COMPONENT);
	json_object *descriptAccelerometer = json_object_new_string("adxl335");
	json_object *error = json_object_new_string("");

	json_object_object_add(jobjAccelerometerData,"status", status);
	json_object_object_add(jobjAccelerometerData,"descript", descriptAccelerometer);
	json_object_object_add(jobjAccelerometerData,"error", error);

	printf(json_object_to_json_string(jobjAccelerometerData));

	FILE * accelerometerData = fopen ("connection/componentsFiles/accelerometer.json", "w");
	fprintf(accelerometerData,"%s",json_object_to_json_string(jobjAccelerometerData));
	fclose(accelerometerData);

	//////////////////// ---- Adc ---- ////////////////////////

	char sps[10] = {0};
	sprintf(sps,"%d",SPS);

	json_object * jobjAdcData = json_object_new_object();

	//json_object *status = json_object_new_string(CORRECT_STATUS_COMPONENT);
	json_object *descriptAdc = json_object_new_string("ads126x de 32 bits");
	json_object *samples = json_object_new_string(sps);
	//json_object *error = json_object_new_string("");

	json_object_object_add(jobjAdcData,"status", status);
	json_object_object_add(jobjAdcData,"descript", descriptAdc);
	json_object_object_add(jobjAdcData,"samples", samples);
	json_object_object_add(jobjAdcData,"error", error);

	printf(json_object_to_json_string(jobjAdcData));

	FILE * adcData = fopen ("connection/componentsFiles/adc.json", "w");
	fprintf(adcData,"%s",json_object_to_json_string(jobjAdcData));
	fclose(adcData);

	//////////////////// ---- Cpu ---- ///////////////////////

	json_object * jobjCpuData = json_object_new_object();

	//json_object *status = json_object_new_string(CORRECT_STATUS_COMPONENT);
	json_object *descriptCpu = json_object_new_string("ARM Cortex-A8 – 1GHz");
	//json_object *error = json_object_new_string("");

	json_object_object_add(jobjCpuData,"status", status);
	json_object_object_add(jobjCpuData,"descript", descriptCpu);
	json_object_object_add(jobjCpuData,"error", error);

	printf(json_object_to_json_string(jobjCpuData));

	FILE * cpuData = fopen ("connection/componentsFiles/cpu.json", "w");
	fprintf(cpuData,"%s",json_object_to_json_string(jobjCpuData));
	fclose(cpuData);

	//////////////////// ---- Battery ---- ///////////////////////

	json_object * jobjBatteryData = json_object_new_object();

	//json_object *status = json_object_new_string(CORRECT_STATUS_COMPONENT);
	json_object *descriptBattery = json_object_new_string("Bateria");
	json_object *charge = json_object_new_string("50.00");
	//json_object *error = json_object_new_string("");

	json_object_object_add(jobjBatteryData,"status", status);
	json_object_object_add(jobjBatteryData,"descript", descriptBattery);
	json_object_object_add(jobjBatteryData,"charge", charge);
	json_object_object_add(jobjBatteryData,"error", error);

	printf(json_object_to_json_string(jobjBatteryData));

	FILE * batteryData = fopen ("connection/componentsFiles/battery.json", "w");
	fprintf(batteryData,"%s",json_object_to_json_string(jobjBatteryData));
	fclose(batteryData);
}

void loadingGpsData(){

	int res = 0, flag = 0;
	char buf[255] = {0};
	char lat[24] = {0}, lng[24] = {0}, alt[24] = {0};
	char dateGps[24] = {0}, timeGps[24] = {0};

	json_object * jobjLocation = json_object_new_object();
	json_object * jobjGpsData = json_object_new_object();
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

	printf(json_object_to_json_string(jstatus));
	//writeSOCKET(json_object_to_json_string(jstatus));
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

					json_object_object_add(jobjLocation,"status", js);
					json_object_object_add(jobjLocation,"lat", jlat);
					json_object_object_add(jobjLocation,"lng", jlng);
					json_object_object_add(jobjLocation,"alt", jalt);
					json_object_object_add(jobjLocation,"date", jdate);
					json_object_object_add(jobjLocation,"time", jtime);

					printf(json_object_to_json_string(jobjLocation));


					json_object *statusGps = json_object_new_string(CORRECT_STATUS_COMPONENT);
					json_object *descriptGps = json_object_new_string("Venus GPS logger");
					json_object *baudRate = json_object_new_string("115200");
					json_object *msjNMEA = json_object_new_string("GGA - RMC");
					json_object *error = json_object_new_string("");

					json_object_object_add(jobjGpsData,"status", statusGps);
					json_object_object_add(jobjGpsData,"descript", descriptGps);
					json_object_object_add(jobjGpsData,"baudRate", baudRate);
					json_object_object_add(jobjGpsData,"msjNMEA", msjNMEA);
					json_object_object_add(jobjGpsData,"error", error);

					printf(json_object_to_json_string(jobjGpsData));
					FILE * gpsData = fopen ("connection/componentsFiles/gps.json", "w");

					fprintf(gpsData,"%s",json_object_to_json_string(jobjGpsData));
					fclose(gpsData);

					//writeSOCKET(json_object_to_json_string(jobj));
					sleep(1);
					break;
				}
			}
			else{
				printf(json_object_to_json_string(jstatus));
				//writeSOCKET(json_object_to_json_string(jstatus));
				sleep(1);
			}
		}
		else{
			fin  = time(NULL);
			if(difftime(fin,inicio) > 20.0){
				if(difftime(fin,inicio) > diff){

					js = json_object_new_int(-1);
					jmsg = json_object_new_string("Revisa la conexión del GPS.");
					printf(json_object_to_json_string(jstatus));
					//writeSOCKET(json_object_to_json_string(jstatus));
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

	printf(json_object_to_json_string(jstatus));
	//writeSOCKET(json_object_to_json_string(jstatus));

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

				printf(json_object_to_json_string(jobj));
				//writeSOCKET(json_object_to_json_string(jobj));

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
					printf(json_object_to_json_string(jstatus));
					//writeSOCKET(json_object_to_json_string(jstatus));
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
	char fecha[50] = {0};

	json_object * jobjTime = json_object_new_object();
	json_object * jobjRtcData = json_object_new_object();

	json_object * jstatus = json_object_new_object();
	json_object *js = json_object_new_int(5);
	json_object *jmsg = json_object_new_string("Activando señal SYNC y sincronizando RTC.");

	json_object_object_add(jstatus,"status", js);
	json_object_object_add(jstatus,"msg", jmsg);

	printf(json_object_to_json_string(jstatus));
	//writeSOCKET(json_object_to_json_string(jstatus));

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

						json_object_object_add(jobjTime,"status", js);
						json_object_object_add(jobjTime,"sync", jboolean);
						json_object_object_add(jobjTime,"time", jtime);
						json_object_object_add(jobjTime,"date", jdate);

						printf(json_object_to_json_string(jobjTime));
						//writeSOCKET(json_object_to_json_string(jobj));

						sprintf(fecha,"%c%c/%c%c/%c%c %c%c:%c%c:%c%c",dateBuf[4],dateBuf[5],dateBuf[2],dateBuf[3],dateBuf[0],dateBuf[1], timeBuf[0],timeBuf[1],timeBuf[2],timeBuf[3],timeBuf[4],timeBuf[5]);

						json_object *statusRtc = json_object_new_string(CORRECT_STATUS_COMPONENT);
						json_object *descriptRtc = json_object_new_string("DS3231");
						json_object *dateHour = json_object_new_string(fecha);
						json_object *error = json_object_new_string("");

						json_object_object_add(jobjRtcData,"status", statusRtc);
						json_object_object_add(jobjRtcData,"descript", descriptRtc);
						json_object_object_add(jobjRtcData,"dateHour", dateHour);
						json_object_object_add(jobjRtcData,"error", error);

						printf(json_object_to_json_string(jobjRtcData));
						FILE * rtcData = fopen ("connection/componentsFiles/rtc.json", "w");

						fprintf(rtcData,"%s",json_object_to_json_string(jobjRtcData));
						fclose(rtcData);

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
						printf(json_object_to_json_string(jstatus));
						//writeSOCKET(json_object_to_json_string(jstatus));
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

void read_prueba_solo_pps(){

	float dataX[1000] = {0};
	float dataY[1000] = {0};
	float dataZ[1000] = {0};

	int num_data = 0;
	int reading_ADC = 0;

	int iii = 0;
	int gggg = 0;
	while(1){

		if(getValue(&gpio26_PPS) == HIGH){
		//if(getValue(&gpio68_SYNC) == LOW){
			gggg = gggg + 1;
			printf("\nggggg es  %d \n", gggg);
			printf("\n ----- Senial pps ------- \n");
			//writeI2C(0x0F,0x88);
			reading_ADC = 1;

			/*iii = 0;
			while(iii<num_data){
				printf("Counter: %d | X: %lf | Y:%lf | Z: %lf\n",iii, dataX[iii],dataY[iii],dataZ[iii]);
				iii = iii + 1;
			}*/

			printf("Numero de datos: %d.\n", num_data);

			num_data = 0;

			printf("Comenzo Lectura pruebas.\n");
			readDataPrueba(&dataX[num_data],&dataY[num_data],&dataZ[num_data]);
			printf("Counter: %d | X: %lf | Y:%lf | Z: %lf\n",num_data, dataX[num_data],dataY[num_data],dataZ[num_data]);
			num_data = num_data + 1;

		}
		else{
			//gggg = 0;
			//printf("low\n");
			if(reading_ADC == 1){
				readDataPrueba(&dataX[num_data],&dataY[num_data],&dataZ[num_data]);
				//printf("Counter: %d | X: %lf | Y:%lf | Z: %lf\n",num_data, dataX[num_data],dataY[num_data],dataZ[num_data]);
				//printf("Counter: %d\n",num_data);
				num_data = num_data + 1;
			}
		}
	}

}

void readDataPrueba(float * dataX, float * dataY, float * dataZ){

	char recvX[6] = {0x00,};
	char recvY[6] = {0x00,};
	char recvZ[6] = {0x00,};

    float adc_countX = 0;
    float adc_countY = 0;
    float adc_countZ = 0;

	readAIN2_3(recvX);
	readAIN4_5(recvY);
	readAIN6_7(recvZ);

	adc_countX = (float) (((unsigned long)recvX[1]<<24)|((unsigned long)recvX[2]<<16)|(recvX[3]<<8)|recvX[4]);
	adc_countY = (float) (((unsigned long)recvY[1]<<24)|((unsigned long)recvY[2]<<16)|(recvY[3]<<8)|recvY[4]);
	adc_countZ = (float) (((unsigned long)recvZ[1]<<24)|((unsigned long)recvZ[2]<<16)|(recvZ[3]<<8)|recvZ[4]);

	*dataX = adc_countX;
	*dataY = adc_countY;
	*dataZ = adc_countZ;

}

//////////////////////////////////////////////////// SEPARADOR ///////////////////////////////////////////////

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
	int isGPS = 1;
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
					gps = readUART(buf);
				}
				if(isData != -1){
					if(bufDate[0] != 0 && bufTime[0] != 0){

						readAnalogInputsAndSaveData(bufDate, bufTime,isGPS);
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
	int isGPS = 0;
	printf("Con RTC.\n");
	// Es necesario reinicar la bandera de la alarma en la direccion 0x0F.
	writeI2C(0x0F,0x88);

	rtc = readI2C(bufRtc);
	if(rtc != -1){
		printData(bufRtc);
		getTimeRtc(time,bufRtc);
		getDateRtc(date,bufRtc);
		readAnalogInputsAndSaveData(date,time,isGPS);
	}
}

int readAnalogInputsAndSaveData(char * date, char * time, int isGPS){

	char recvX[6] = {0x00,};
	char recvY[6] = {0x00,};
	char recvZ[6] = {0x00,};

    float adc_countX = 0;
    float adc_countY = 0;
    float adc_countZ = 0;

	float dataX[MAX_SPS] = {0};
	float dataY[MAX_SPS] = {0};
	float dataZ[MAX_SPS] = {0};

	float samplesX[SPS] = {0};
	float samplesY[SPS] = {0};
	float samplesZ[SPS] = {0};

	int factor =  MAX_SPS/SPS;

	// FACTOR_SPS_200 = 2
	//FACTOR_SPS_100 = 4
	// FACTOR_SPS_50  = 8

	createDirRtc(currentDirectoryX, axisX, date, time, isGPS);
	createDirRtc(currentDirectoryY, axisY, date, time, isGPS);
	createDirRtc(currentDirectoryZ, axisZ, date, time, isGPS);

	int count = 0;
	/*if(getValue(&gpio26_PPS) == HIGH){
		printf("PPS ES ALTA");
		return 0;
	}
	else{
		printf("PPS ES BAJA");
		return 0;
	}*/
	printf("Capturando datos ADC\n");
	while(count < MAX_SPS){

		//printf("inicio count %d\n", count);

		readAIN2_3(recvX);
		readAIN4_5(recvY);
		readAIN6_7(recvZ);
		//printf("\n");

		adc_countX = (float) (((unsigned long)recvX[1]<<24)|((unsigned long)recvX[2]<<16)|(recvX[3]<<8)|recvX[4]);
		adc_countY = (float) (((unsigned long)recvY[1]<<24)|((unsigned long)recvY[2]<<16)|(recvY[3]<<8)|recvY[4]);
		adc_countZ = (float) (((unsigned long)recvZ[1]<<24)|((unsigned long)recvZ[2]<<16)|(recvZ[3]<<8)|recvZ[4]);

		dataX[count] = adc_countX;
		dataY[count] = adc_countY;
		dataZ[count] = adc_countZ;

		/*xx = getVoltage(recvX);
		yy = getVoltage(recvY);
		zz = getVoltage(recvZ);*/

		//printf("Counter: %d | X: %lf | Y:%lf | Z: %lf\n",count, adc_countX,adc_countY,adc_countZ);
		count++;

	}
	//printf("num datos: %d\n",count);

	subMuestreo_xxx(dataX, samplesX, factor);
	subMuestreo_xxx(dataY, samplesY, factor);
	subMuestreo_xxx(dataZ, samplesZ, factor);

	strDepValues.npts = strDepValues.npts + strDepValues.dataNumber;

	writeSac(strDepValues.npts,strDepValues.dataNumber,dataX,strDepValues.dt,axisX,currentDirectoryX);
	writeSac(strDepValues.npts,strDepValues.dataNumber,dataY,strDepValues.dt,axisY,currentDirectoryY);
	writeSac(strDepValues.npts,strDepValues.dataNumber,dataZ,strDepValues.dt,axisZ,currentDirectoryZ);

	printf("Termino camptura de datos ADC\n");
	//fclose(sampleFile);
	count = 0;
	return 0;
}

void subMuestreo_xxx(float *currentData, float *newData, int factor){

	int i = 0;
	int samples = MAX_SPS/factor;

	while (i < samples){
		newData[i] = currentData[i*factor];
		//printf("i es %d y el dato tomado es %d\n",i,i*factor);
		i++;
	}

}

void createDirRtc(char *dir, char *axis,char * date, char *time, int isGPS){
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
		sprintf(dir,"%s/%s/%s_%c%c_%s.sac",SAMPLES_DIR_R,date,date,time[0],time[1],axis);
		initDataofSamples(date,time,isGPS);
		createFile(dir);

	}
	else if (time[2]=='0' && time[3]=='0' && time[4]=='0' && time[5]=='0'){ //Nueva Hora
		sprintf(dir,"%s/%s/%s_%c%c_%s.sac",SAMPLES_DIR_R,date,date,time[0],time[1],axis);
		//sprintf(dir,"%s/%s/%s%s_%c%c%c%c%c%c.sac",SAMPLES_DIR_R,date,axis,date,time[0],time[1],time[2],time[3],time[4],time[5]);
		initDataofSamples(date,time,isGPS);
		createFile(dir);

	}
	printf("Dir: %s\n", dir);
}

void initDataofSamples(char * date, char *time, int isGPS){
	char year[2] = {0};
	char month[2] = {0};
	char day[2] = {0};

	char hour[2] = {0};
	char min[2] = {0};
	char seg[2] = {0};
	char mseg[3] = {0};

	// Se guarda en un array cuando dias hay desde el primero de enero hasta un mes.
	//int numeroDiasPorMes[13] = {0,31,28,31,30,31,30  ,31,31,  30,31,30,31};
	int numeroDiasPorMes[13] = {0,31,59,90,120,151,181,212,243,273,304,334,365};
	year[0] = date[4];
	year[1] = date[5];

	month[0] = date[2];
	month[1] = date[3];

	day[0] = date[0];
	day[1] = date[1];

	hour[0] = time[0];
	hour[1] = time[1];

	min[0] = time[2];
	min[1] = time[3];

	seg[0] = time[4];
	seg[1] = time[5];

	if(isGPS == 1){
		mseg[0] = time[7];
		mseg[1] = time[8];
		mseg[2] = time[9];
	}

	strFullDate.year = 2000 + atoi(year);
	strFullDate.month = atoi(month);

	strFullDate.day = numeroDiasPorMes[strFullDate.month - 1] + atoi(day);

	strFullDate.hour = atoi(hour);
	strFullDate.min = atoi(min);
	strFullDate.seg = atoi(seg);
	strFullDate.mseg = atoi(mseg);

	// se reinician el numero de muestras para que comience a contar nuevamente en el siguiente archivo
	strDepValues.npts = 0;
	strDepValues.dt = DT;
	strDepValues.dataNumber = SPS;
	/// se definene los valores de DELTA, NTPS, y dataNumber que es el numero de datos por segundo
	// este no se incluye como tan en el archivo.
}

void writeSac(int npts, int dataNumber, float *arr, float dt, char *axis ,char *filename)
{
        /* create the SAC file
           instead of using the wsac1 I will use the lower level
           routines to provide more control on the output */
        int nerr;
        float b, e;
        //float depmax, depmin, depmen;
        /* get the extrema of the trace */
       // printf("antes de scmxmn\n");
        		//scmxmn(arr,npts,&depmax,&depmin,&depmen);
                scmxmn(arr,dataNumber,&strDepValues.depmax,&strDepValues.depmin,&strDepValues.depmen);
        //printf("despues de scmxmn y antes de newhdr\n");
        /* create a new header for the new SAC file */
                newhdr();
        //printf("despues de newhdr y antes de los set \n");
        /* set some header values */
                setfhv("DEPMAX", strDepValues.depmax, &nerr);
                setfhv("DEPMIN", strDepValues.depmin, &nerr);
                setfhv("DEPMEN", strDepValues.depmen, &nerr);
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
                setnhv("NZYEAR", strFullDate.year, &nerr);
                setnhv("NZJDAY", strFullDate.day, &nerr);
                setnhv("NZHOUR", strFullDate.hour, &nerr);
                setnhv("NZMIN" , strFullDate.min, &nerr);
                setnhv("NZSEC" , strFullDate.seg, &nerr);
                setnhv("NZMSEC", strFullDate.mseg, &nerr);

                setkhv("KNETWK", "MEC",&nerr);
                setkhv("KSTNM", "POP",&nerr);
                setkhv("KCMPNM", axis,&nerr);
        /* output the SAC file */
         //printf("despues de los set y antes de updateHeaders \n");
                updateHeaders(filename);
         //printf("despues de updateHeaders y antes de updateData \n");
        		updateData(filename,dataNumber,arr);
                //updateData(filename,npts,arr);
                //bwsac(npts,filename,arr);
           //     printf("metodo\n");
}


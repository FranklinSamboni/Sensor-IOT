#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "gps.h"

gpsStr gps;
gpsData data;

int openUART(int baudRate, char * gpsDevice){

	strcpy(gps.device,gpsDevice);

	gps.file = open(gps.device, O_RDWR | O_NOCTTY | O_NDELAY);
	if (gps.file < 0) {
		char msgError[] = "Error intentando abrir el dispositivo UART.";
		printErrorMsgGps(msgError);
		perror(gps.device);
		return -1;
	}

	//tcgetattr(file, &gps.options);
	bzero(&gps.options, sizeof(gps.options));
	if(baudRate == 0){
		gps.options.c_cflag = BAUDRATE0 | CS8 | CREAD | CLOCAL; // baudrate de 4800
	}
	else if(baudRate == 1){
		gps.options.c_cflag = BAUDRATE1 | CS8 | CREAD | CLOCAL; // baudrate de 9600
	}
	else if(baudRate == 2){
		gps.options.c_cflag = BAUDRATE2 | CS8 | CREAD | CLOCAL; // baudrate de 38400
	}
	else if(baudRate == 3){
		gps.options.c_cflag = BAUDRATE3 | CS8 | CREAD | CLOCAL; // baudrate de 115200
	}
	else{
		gps.options.c_cflag = BAUDRATE1 | CS8 | CREAD | CLOCAL;  // baudrate de 9600 -- por defecto
	}

	gps.options.c_iflag = IGNPAR;
	gps.options.c_oflag = 0;
	gps.options.c_lflag = ICANON;
	tcflush(gps.file, TCIFLUSH);
	tcsetattr(gps.file, TCSANOW, &gps.options);
	return 1;
}

int readUART(char * buf){
	/*char buf[255];  Buffer para la lectura del UART. */
	int resultado; /* Numero de bytes resultantes de la lectura del UART. */

	resultado = read(gps.file,buf,255);
	if(resultado < 0){
		char msg[] = "Error leyendo el UART.";
		printErrorMsgGps(msg);
		perror(gps.device);
		return -1;
	}
	buf[resultado] = 0;
	return resultado;
}

int writeUART(char *buffer){
	int bytes;

	bytes = write(gps.file,buffer,255);

	if(bytes < 0){
		char msg[] = "Error escribiendo en UART.";
		printErrorMsgGps(msg);
		perror(gps.device);
		return -1;
	}
	return 1;
}

int closeUART(){
	int fd = close(gps.file);
	if(fd == -1 ){
		return -1;
	}
	gps.file = fd; /* fd debe ser cero si la sentencia close es correcta.*/
	gps.device[0]=0;
	gps.device[1]=0;
	gps.device[2]=0;
	gps.device[3]=0;
	return 1;

}

char checkSum(int pl, char * payload){

	char cs = 0x00;
	int n = 0;
	while(n < pl){
		cs = cs ^ payload[n];
		n = n + 1;
	}
	printf("CS es : !%hhX!\n", cs);
	// malloc(255);
	return cs;
}

int setFactoryDefaults(){
	printf("Restaurando datos de fábrica.\n");
	char buffer[9] = {0};
	char payload[2] = {0};
	int res;
	buffer[0] = 0xA0;
	buffer[1] = 0xA1;
	buffer[2] = 0x00;
	buffer[3] = 0x02; // PL -> PayLoad length

	payload[0] = 0x04; // 0x04 -> Es el id del mensaje "setFactoryDefaults"
	payload[1] = 0x01; // 0x01 -> reiniciar despues de configurar los valores por defecto

	buffer[4] =  payload[0];
	buffer[5] =  payload[1];
	buffer[6] = checkSum(2,payload); // CS -> Se calcula el CheckSum
	buffer[7] = 0x0D;
	buffer[8] = 0x0A;

	printf("Escribiendo: !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8]);

	res = writeUART(buffer);
	return res;
}

int configureSerialPort(int bauds){
	printf("Inicio Cambio: %d\n", bauds);
	char options[4] = {0x00,0x01,0x03,0x05};
	char payload[4] = {0};
	char buffer[11] = {0};
	int res;

	payload[0] = 0x05; // 0x05 -> Es el id del mensaje "configureSerialPort"
	payload[1] = 0x00; // 0x00 -> COM0
	if(bauds > -1 && bauds < 4){
		payload[2] = options[bauds]; // Tasa de baudios
	}
	else{
		printErrorMsgGps("Opción incorrecta, Elige 0 para 4800,1 para 9600,2 para 38400,3 para 115200");
		return -1;
	}
	payload[3] = 0x01; // 0x01 -> actualizar en la SRAM y FLASH -- 0x00 -> solo en SRAM.

	/*Construyendo mensaje*/
	buffer[0] = 0xA0;
	buffer[1] = 0xA1;
	buffer[2] = 0x00;
	buffer[3] = 0x04; // PL -> PayLoad length
    /*---- Se elige 0x01 para que la configuracion no se pierda cuando se reinicie el sistema --- */
	buffer[4] = payload[0];
    buffer[5] = payload[1];;
    buffer[6] = payload[2];
    buffer[7] = payload[3];
	buffer[8] = checkSum(4,payload); // CS -> Se calcula el CheckSum.
    buffer[9] = 0x0D;
    buffer[10] = 0x0A;

	printf("Escribiendo: !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);

	// Escribe por el UART antes de modificar la tasa de baudios de la Beagle.
	res = writeUART(buffer);
	printf("WRITE");
	if(res == 1){ // verificar la escritura, cierre, y abertura de dispositivo.
		char device[24] = {0};
		strcpy(device,gps.device);
		res =  closeUART();
		printf("cLOSE");
		if(res == 1){
			res = openUART(bauds, device);
			printf("OPEN");
			return res;
		}
	}
	return -1;
}

int configureMessageType(int type)
{
	char buffer[9] = {0};
	char payload[2] = {0};
	int res;
	buffer[0] = 0xA0;
	buffer[1] = 0xA1;
	buffer[2] = 0x00;
	buffer[3] = 0x02; // PL -> PayLoad length
	payload[0] = 0x09; // 0x09 -> Es el id de "messageType"
	if(type == 2){
		payload[1] = 0x02;  // 0x02 -> Mensajes Binarios
	}
	else if(type == 0){
		payload[1] = 0x00;   // 0x00 -> No output
	}
	else{ //por defecto se va a configurar los mensajes NMEA
		payload[1] = 0x01; // 0x01 -> Mensajes NMEA
	}
	buffer[4] = payload[0];
	buffer[5] = payload[1];
	buffer[6] = checkSum(2,payload); // CS -> CheckSum
	buffer[7] = 0x0D;
	buffer[8] = 0x0A;
	printf("Escribiendo: !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8]);
	res = writeUART(buffer);
	return res;
}

int configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA){

	if (GGA > 1 || GGA < 0 ||
		GSA > 1 || GSA < 0 ||
		GSV > 1 || GSV < 0 ||
		GLL > 1 || GLL < 0 ||
		RMC > 1 || RMC < 0 ||
		VTG > 1 || VTG < 0 ||
		ZDA > 1 || ZDA < 0 )
	{
		char msgError[] = "Error, los valores para configureNMEA_Messages deben ser : 0 o 1 ";
		printErrorMsgGps(msgError);
		return -1;
	}
	else{
		char buffer[16];
		char payload[9] = {0};
		int res;
		buffer[0] = 0xA0;
		buffer[1] = 0xA1;
		buffer[2] = 0x00;
		buffer[3] = 0x09; // PL -> PayLoad length

		payload[0] = 0x08; // 0x08 -> Es el id del "configureNMEA_Messages"
		payload[1] = GGA; // -> GGA
		payload[2] = GSA; // -> GSA
		payload[3] = GSV; // -> GSV
		payload[4] = GLL; // -> GLL
		payload[5] = RMC; // -> RMC
		payload[6] = VTG; // -> VTG
		payload[7] = ZDA; // -> ZDA

/*--------------------------------------------------------------------------------------------*/
		payload[8] = 0x01; // 0x01 -> update to SRAM & FLASH  ---  0x00 -> only SRAM.
/*---- Se elige 0x01 para que la configuracion no se pierda cuando se reinicie el sistema --- */

		buffer[4] = payload[0];
		buffer[5] = payload[1];
		buffer[6] = payload[2];
		buffer[7] = payload[3];
		buffer[8] = payload[4];
		buffer[9] = payload[5];
		buffer[10] = payload[6];
		buffer[11] = payload[7];
		buffer[12] = payload[8];

		buffer[13] = checkSum(9,payload); // CS -> CheckSum
		buffer[14] = 0x0D;
		buffer[15] = 0x0A;

		printf("Escribiendo: !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8]);
		printf("!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n",buffer[9],buffer[10],buffer[11],buffer[12],buffer[13],buffer[14],buffer[15]);
		res = writeUART(buffer);
		return res;
	}
}

void printErrorMsgGps(char *msgError){
	printf("\nError GPSLIB: %s\n",msgError);
}

void printBuffer(int size, char * buffer){
	int i = 0;
	/*printf("Leyendo Buffer # bytes: %d ------------\n",size);
	while (i < size){ // imprimir el mensaje en hexadecimal.
			printf("!%hhX!",buffer[i]);
			i = i + 1;
		}*/
	printf("\n%s\n",buffer);

}

void saveDataGps(char * buffer, char * dir){

	int bits = 0;
	if(isGGA(buffer) == 1){
		bits = getTimeGps(data.time,buffer);
		data.time[bits] = 0;
		bits = getLat(data.lat,buffer);
		data.lat[bits] = 0;
		bits = getLng(data.lng,buffer);
		data.lng[bits] = 0;
		bits = getAlt(data.alt,buffer);
		data.alt[bits] = 0;
	}
	else if(isRMC(buffer) == 1) {
		bits = getDateGps(data.date,buffer);
		data.date[bits] = 0;
	}

	if(data.date[0] != 0 && data.time[0] != 0){

		createDirGps(dir,data.date,data.time);

		//FILE * ubicacion = fopen ("ubicacion.txt", "w+");
		//FILE * ubicacion = fopen ("muestras/pruebas/datos.txt", "w");
		FILE * ubicacion = fopen (dir, "r+");
		printf("GPS Archivo -> date: %s | time: %s | lat: %s | lng: %s | alt: %s\n",data.date,data.time,data.lat,data.lng,data.alt);
		fprintf(ubicacion,"date: %s | time: %s | lat: %s | lng: %s | alt: %s\n",data.date,data.time,data.lat,data.lng,data.alt);
		fclose(ubicacion);
	}
	/*
	int result = -1, i=0;
	char *token = strtok(buffer,",");
	result = strcmp(token,"$GPGGA"); // Para comparar si dos cadenas son iguales. retorna 0 si son iguales.
	if(result == 0){
	  	while (token != NULL)
	   	{
	   		strcpy(data.gpgga[i],token);
	   		//printf("array: %s \n", data.gpgga[i]);
	   		token = strtok(NULL, ",");
    		i = i + 1;
    	}
		i=0;
	}
	else {
		result = strcmp(token,"$GPRMC");
		if(result == 0){
		  	while (token != NULL)
		   	{
		   		strcpy(data.gprmc[i],token);
		   		//printf("array: %s \n", data.gprmc[i]);
		   		token = strtok(NULL, ",");
	    		i = i + 1;
	    	}
		}
	}

	FILE * ubicacion = fopen ("ubicacion.txt", "w+");
    printf("date: %s | time: %s | lat: %s%s | lng: %s%s | alt: %s\n",data.gprmc[9],data.gpgga[1],data.gpgga[2],data.gpgga[3],data.gpgga[4],data.gpgga[5],data.gpgga[9]);
    fprintf(ubicacion,"date: %s | time: %s | lat: %s%s | lng: %s%s | alt: %s\n",data.gprmc[9],data.gpgga[1],data.gpgga[2],data.gpgga[3],data.gpgga[4],data.gpgga[5],data.gpgga[9]);
    fclose(ubicacion);

	*/

}

int isGpsConectedToSat(char * buffer){
	if(isGGA(buffer) == 1){

	}
	else if(isRMC(buffer) == 1){

	}
	return -1;
}

/*Verificar que la trama sea GGA*/
int isGGA(char * buffer){

	if(buffer[0]=='$' && buffer[1]=='G' && buffer[2]=='P' && buffer[3]=='G' && buffer[4]=='G' && buffer[5]=='A'){
		return 1;
	}
	return -1;

	/*char *token = strtok(buffer,",");
	if(strcmp(token,"$GPGGA") == 0){
		return 1; // es GGA
	}
	return -1;*/
}

/*Verificar que la trama sea RMC*/
int isRMC(char * buffer){

	if(buffer[0]=='$' && buffer[1]=='G' && buffer[2]=='P' && buffer[3]=='R' && buffer[4]=='M' && buffer[5]=='C'){
		return 1;
	}
	return -1;
	/*char *token = strtok(buffer,",");
	if(strcmp(token,"$GPRMC") == 0){
		return 1; // es RMC
	}
	return -1;*/
}

/* Verificar que la trama sea GGA antes de llamar a este metodo. Con isGGA() */
int getTimeGps(char * buffer, char * GGA_NEMEA){

	int i=0;
	while(i<10){
		buffer[i] = GGA_NEMEA[i+7];  // El tiempo comienza en la posición 7 y su longitud es de 10 bits.
		i++;
	}

	return i; // # bits

	/*if(data.gprmc[3][0] == 0 && data.gprmc[3][1] == 0 && data.gprmc[3][2] == 0 && data.gprmc[3][3] == 0){
		printErrorMsgGps("Error Tiempo no se ha capturado\n");
	    return -1;
	}
	int tam = sizeof(data.gprmc[1]); // posicion del Tiempo
	int count = 0;
	while(count < tam){
		if(data.gprmc[1][count] == 0){
			break;
		}
		buffer[count] = data.gprmc[1][count];
		count = count + 1;
	}
	return count;*/
}

/* Verificar que la trama sea RMC antes de llamar a este metodo. Con isRMC()*/
int getDateGps(char * buffer, char * RMC_NEMEA){

	int i=0;
	while(i<6){
		buffer[i] = RMC_NEMEA[i+57]; // La fecha comienza en la posición 57 y su longitud es de 6 bits.
		i++;
	}

	return i; // # bits

	/*if(data.gprmc[3][0] == 0 && data.gprmc[3][1] == 0 && data.gprmc[3][2] == 0 && data.gprmc[3][3] == 0){
		printErrorMsgGps("Error Fecha no se ha capturado\n");
	    return -1;
	}
	int tam = sizeof(data.gprmc[9]); // posicion de la fecha
	int count = 0;
	while(count < tam){
		if(data.gprmc[9][count] == 0){
			break;
		}
		buffer[count] = data.gprmc[9][count];
		count = count + 1;
	}
	return count;*/

}

/* Verificar que la trama sea GGA antes de llamar a este metodo. Con isGGA()*/
int getLat(char * buffer, char * GGA_NEMEA){

	int i=0;
	while(i<9){
		buffer[i] = GGA_NEMEA[i+18];  // La latitud comienza en la posición 18 y su longitud es de 9 bits.
		i++;
	}

	buffer[i] = GGA_NEMEA[28]; //Añadimos el indicador de N o S ubicados en la posicion 28, generando cadena de 10 bits.
	i++;

	return i; // # bits

	/*if(data.gprmc[3][0] == 0 && data.gprmc[3][1] == 0 && data.gprmc[3][2] == 0 && data.gprmc[3][3] == 0){
		printErrorMsgGps("Error Latitud no se ha capturado\n");
	    return -1;
	}
	int tam = sizeof(data.gprmc[3]); // posicion de la fecha
	int count = 0;
	while(count < tam){
		if(data.gprmc[3][count] == 0){
			buffer[count] = data.gprmc[4][0];
			count = count + 1;
			break;
		}
		buffer[count] = data.gprmc[3][count];
		count = count + 1;
	}
	return count;*/
}

/* Verificar que la trama sea GGA antes de llamar a este metodo. Con isGGA()*/
int getLng(char * buffer, char * GGA_NEMEA){

	int i=0;
	while(i<10){
		buffer[i] = GGA_NEMEA[i+30];  // La longitud comienza en la posición 30 y su longitud es de 10 bits.
		i++;
	}

	buffer[i] = GGA_NEMEA[41]; //Añadimos el indicador de W o E ubicados en la posicion 41, generando cadena de 11 bits.
	i++;

	return i; // # bits


	/*if(data.gprmc[3][0] == 0 && data.gprmc[3][1] == 0 && data.gprmc[3][2] == 0 && data.gprmc[3][3] == 0){
		printErrorMsgGps("Error Longitud no se ha capturado\n");
	    return -1;
	}
	int tam = sizeof(data.gprmc[5]); // posicion de la fecha
	int count = 0;
	while(count < tam){
		if(data.gprmc[5][count] == 0){
			buffer[count] = data.gprmc[6][0];
			count = count + 1;
			break;
		}
		buffer[count] = data.gprmc[5][count];
		count = count + 1;
	}
	return count;*/
}

/* Verificar que la trama sea GGA antes de llamar a este metodo. Con isGGA()*/
int getAlt(char * buffer, char * GGA_NEMEA){

	int i=0;
	while(i<6){
		buffer[i] = GGA_NEMEA[i+52];  // La altitud comienza en la posición 52 y su longitud es de 6 bits.
		i++;
	}

	return i; // # bits

	/*if(data.gpgga[9][0] == 0 && data.gpgga[9][1] == 0 && data.gpgga[9][2] == 0 && data.gpgga[9][3] == 0){
		printErrorMsgGps("Error Altitud no se ha capturado\n");
	    return -1;
	}
	int tam = sizeof(data.gpgga[9]); // posicion de la fecha
	int count = 0;
	while(count < tam){
		if(data.gpgga[9][count] == 0){
			break;
		}
		buffer[count] = data.gpgga[9][count];
		count = count + 1;
	}
	return count;*/
}

void createDirGps(char *dir, char * date, char *time){
	char fecha[100] = {0};
	struct stat st = {0};
	sprintf(fecha,"%s/%s",SAMPLES_DIR,date);

	if (stat(SAMPLES_DIR, &st) == -1) {
	    mkdir(SAMPLES_DIR, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	if (stat(fecha, &st) == -1) {
	    mkdir(fecha, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}

	if(dir[0] == 0){
		sprintf(dir,"%s/%s/%c%c%c%c%c%c.txt",SAMPLES_DIR,date,time[0],time[1],time[2],time[3],time[4],time[5]);
		FILE *archivo = fopen (dir, "w");
		fclose(archivo);
	}
	else if (time[2]=='0' && time[3]=='0' && time[4]=='0' && time[5]=='0'){ //Nueva Hora
		sprintf(dir,"%s/%s/%c%c%c%c%c%c.txt",SAMPLES_DIR,date,time[0],time[1],time[2],time[3],time[4],time[5]);
		FILE *archivo = fopen (dir, "w");
		fclose(archivo);
	}

}




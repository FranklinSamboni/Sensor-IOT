/*
 * ConfigGPS.c
 *
 *  Created on: 8/11/2016
 *      Author: Frank
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEVICE "/dev/ttyO2" // Ubicación del dispositivo, en este caso UART2
#define BAUDRATE0 B4800   // 9600 es el valor establecido por fabrica.
#define BAUDRATE1 B9600
#define BAUDRATE2 B38400
#define BAUDRATE3 B115200

#define TRUE 1
#define FALSE 0

int file; 	   // Identificador del archivo
struct termios options; // Opciones de configuracion del UART
FILE *ubicacion;
FILE *tiempo;

void setFactoryDefaults();
void configureSerialPort(int bauds);
void configureMessageType(int type);
void configureNMEA_MESSAGES(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA);
void printERRORmsg(char * msgError);
void printBuffer(int size, char * buffer);
void saveTime(int size, char * buffer);
void savePosition( int size, char * buffer);
void openUART(int baudRate);
void readUART();
void writeUART(char * buffer);
void writeConfigMessage(char *buffer);
void closeUART();

/*
void signal_handler(int sig){

	printf( "Ctrl-C pressed, cleaning up and exiting..\n" );
	keepGoing = 0;
}*/

int main(int argc, char *argv[]){
	printf("-----------------------------------------------------------------------------------------------\n");
    printf("--------------------------- Programa para la configuración del GPS ----------------------------\n");
    printf("-----------------------------------------------------------------------------------------------\n");

    openUART(1); // 1 -> B9600
    printf("Abrio\n");

    /*char buffe[9];
    buffe[0] = 0xA0;
    buffe[1] = 0xA1;
    buffe[2] = 0x00;
    buffe[3] = 0x02;
    buffe[4] = 0x30;
    buffe[5] = 0x00;
    buffe[6] = 0x30;
    buffe[7] = 0x0D;
    buffe[8] = 0x0A;
    write(file,buffe,16);*/

    //setFactoryDefaults();
    //configureSerialPort(0);//115200
    //configureMessageType(0);//NMEA

    //configureNMEA_MESSAGES(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA);
    //configureNMEA_MESSAGES(1,1,0,0,1,0,0);

    while (TRUE) {
    	readUART();
	}

    closeUART();
    return 0;
}

void openUART(int baudRate){
	file = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
	if (file < 0) {
		char msgError[] = "Error intentando abrir el dispositivo UART.";
		printERRORmsg(msgError);
		perror(DEVICE);
		exit(-1);
	}

	//tcgetattr(file, &options);
	bzero(&options, sizeof(options));
	if(baudRate == 0){
		options.c_cflag = BAUDRATE0 | CS8 | CREAD | CLOCAL; // baudrate de 4800
	}
	else if(baudRate == 1){
		options.c_cflag = BAUDRATE1 | CS8 | CREAD | CLOCAL; // baudrate de 9600
	}
	else if(baudRate == 2){
		options.c_cflag = BAUDRATE2 | CS8 | CREAD | CLOCAL; // baudrate de 38400
	}
	else if(baudRate == 3){
		options.c_cflag = BAUDRATE3 | CS8 | CREAD | CLOCAL; // baudrate de 115200
	}
	else{
		options.c_cflag = BAUDRATE1 | CS8 | CREAD | CLOCAL;  // baudrate de 9600 -- por defecto
	}

	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = ICANON;
	tcflush(file, TCIFLUSH);
	tcsetattr(file, TCSANOW, &options);

}

void readUART(){
	int resultado; // Numero de bits resultantes de la lectura del UART
	char buf[255]; // Buffer para la lectura del UART

	resultado = read(file,buf,255);
	if(resultado < 0){
		//char msg[] = "Error leyendo en UART.";
		//printERRORmsg(msg);
		//perror(DEVICE);
		//readUART();
	}
	else{
		//	printBuffer(resultado, buf);
		savePosition(resultado, buf);
		//saveTime(resultado, buf);
	}
}

void writeUART(char * buffer){
	write(file,buffer,255);
}

void writeConfigMessage(char *buffer){
	int bytes;

	bytes = write(file,buffer,255);
	if(bytes < 0){
		char msg[] = "Error escribiendo en UART.";
		printERRORmsg(msg);
		perror(DEVICE);
	}

	readUART(); // Para leer la respuesta del gps.
	readUART();

}

void closeUART(){
	int fd = close(file);
	if(fd != -1 ){
		file = fd; // fd debe ser cero si la sentencia close es correcta.
	}
}

/* ----------------------------------------------------------------------------------------------------
 * ---Para la configuración del GPS se utiliza los mensajes binarios de SkyTraq Venus 6 GPS receiver---
 * ----------------------------------------------------------------------------------------------------
 * ---Sintaxis de los mensajes: <0xA0,0xA1><PL><Message ID><Message Body><CS><0x0D,0x0A> --------------
 * ----------------------------------------------------------------------------------------------------
 * PL -> PayLoad length --- CS -> CheckSum*/

/*-- Restaurar los parametros del GPS a valores de fabrica. --*/
void setFactoryDefaults(){
	char buffer[9] = {0};
	buffer[0] = 0xA0;
	buffer[1] = 0xA1;
	buffer[2] = 0x00;
	buffer[3] = 0x02; // PL -> PayLoad length
	buffer[4] = 0x04; // 0x04 -> Es el id de "setFactoryDefaults"
	buffer[5] = 0x01; // 0x01 -> reiniciar despues de configurar los valores por defecto
	buffer[6] = 0x04; // CS -> CheckSum
	buffer[7] = 0x0D;
	buffer[8] = 0x0A;
	writeConfigMessage(buffer);
}



/*---- Configurar el puerto serial ----
 *-- Se configura la tasa de baudios---
 *-- bauds = 0 -> 4800
 *-- bauds = 1 -> 9600
 *-- bauds = 2 -> 38400
 *-- bauds = 3 -> 115200
 *-------------------------------------
 *---El Venus GPS logger no maneja las--
 *---tasas de baudios de : ---------
 *---19200 y 57600 */
void configureSerialPort(int bauds){
	printf("Inicio Cambio");
	char buffer[11] = {0};
	buffer[0] = 0xA0;
	buffer[1] = 0xA1;
	buffer[2] = 0x00;
	buffer[3] = 0x04; // PL -> PayLoad length
	buffer[4] = 0x05; // 0x05 -> Es el id de "configureSerialPort"
	buffer[5] = 0x00; // 0x00 -> COM0
	buffer[6] = 0x01; // Aqui va la tasa de baudios -> por defecto es 9600
    /*--------------------------------------------------------------------------------------------*/
    buffer[7] = 0x01; // 0x01 -> update to SRAM & FLASH  ---  0x00 -> only SRAM.
    /*---- Se elige 0x01 para que la configuracion no se pierda cuando se reinicie el sistema --- */
	buffer[8] = 0x05; // CS -> CheckSum
	buffer[9] = 0x0D;
	buffer[10] = 0x0A;

	switch(bauds){
	case 0:
		buffer[6] = 0x00;  // -> 4800
		break;
	case 1:
		buffer[6] = 0x01; // -> 9600
		break;
	case 2:
		buffer[6] = 0x03; // -> 38400
		break;
	case 3:
		buffer[6] = 0x05; //  -> 115200
		break;
	default:
		buffer[6] = 0x01; // -> 9600
		break;
	}

	printf("Escribiendo");
	writeConfigMessage(buffer); // escribe por el UART antes de modificar la tasa de baudios de la Beagle.

	printf("cambio tasa de baudios");
    /*
	tcgetattr(file, &options);
	if(bauds == 0){
		cfsetispeed(&options, BAUDRATE0); // cambiar la velocidad de entrada y salida del UART de la Beagle.
	}
	else if(bauds == 2){
		cfsetispeed(&options, BAUDRATE2);
	}
	else if(bauds == 3){
		cfsetispeed(&options, BAUDRATE3);
	}
	else{
		cfsetispeed(&options, BAUDRATE1);
	}
	tcflush(file, TCIFLUSH);
	tcsetattr(file, TCSANOW, &options);*/

}


/*-- type = 0 -> mensajes NMEA, type = 1 -> mensajes Binarios. --*/
void configureMessageType(int type)
{
	char buffer[9] = {0};
	if(type == 0){
		buffer[0] = 0xA0;
		buffer[1] = 0xA1;
		buffer[2] = 0x00;
		buffer[3] = 0x02; // PL -> PayLoad length
		buffer[4] = 0x09; // 0x09 -> Es el id de "messageType"
		buffer[5] = 0x01; // 0x01 -> Mensajes NMEA
		buffer[6] = 0x09; // CS -> CheckSum
		buffer[7] = 0x0D;
		buffer[8] = 0x0A;
		writeConfigMessage(buffer);
	}
	else if(type == 1){
		buffer[0] = 0xA0;
		buffer[1] = 0xA1;
		buffer[2] = 0x00;
		buffer[3] = 0x02; // PL -> PayLoad length
		buffer[4] = 0x09; // 0x09 -> Es el id del "messageType"
		buffer[5] = 0x01; // 0x02 -> Mensajes Binarios
		buffer[6] = 0x09; // CS -> CheckSum
		buffer[7] = 0x0D;
		buffer[8] = 0x0A;
		writeConfigMessage(buffer);
	}
	else {
		char msgError[] = "Error, los valores para configureMessageType deben ser : 0 -> NMEA, 1 -> Binario";
		printERRORmsg(msgError);
	}
}

/*-- Habilitar o deshabilitar los mensasjes NMEA --
 *-- Los parametros de esta función deben tomar los valores de 0x00 -> 0 o 0x01 -> 1 --
 *-- 0 -> deshabilitado
 *-- 1 -> habilitado --*/
void configureNMEA_MESSAGES(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA){

	if (GGA > 1 || GGA < 0 ||
		GSA > 1 || GSA < 0 ||
		GSV > 1 || GSV < 0 ||
		GLL > 1 || GLL < 0 ||
		RMC > 1 || RMC < 0 ||
		VTG > 1 || VTG < 0 ||
		ZDA > 1 || ZDA < 0 )
	{
		char msgError[] = "Error, los valores para configureNMEA_MESSAGES deben ser : 0 o 1 ";
		printERRORmsg(msgError);
	}
	else{
		char buffer[16];
		buffer[0] = 0xA0;
		buffer[1] = 0xA1;
		buffer[2] = 0x00;
		buffer[3] = 0x09; // PL -> PayLoad length
		buffer[4] = 0x08; // 0x08 -> Es el id del "configureNMEA_MESSAGES"
		buffer[5] = GGA; // -> GGA
		buffer[6] = GSA; // -> GSA
		buffer[7] = GSV; // -> GSV
		buffer[8] = GLL; // -> GLL
		buffer[9] = RMC; // -> RMC
		buffer[10] = VTG; // -> VTG
		buffer[11] = ZDA; // -> ZDA
		/*--------------------------------------------------------------------------------------------*/
		buffer[12] = 0x01; // 0x01 -> update to SRAM & FLASH  ---  0x00 -> only SRAM.
		/*---- Se elige 0x01 para que la configuracion no se pierda cuando se reinicie el sistema --- */
		buffer[13] = 0x08; // CS -> CheckSum
		buffer[14] = 0x0D;
		buffer[15] = 0x0A;

		writeConfigMessage(buffer);
	}
}

void printERRORmsg(char *msgError){
	printf("\n------------------------Error-------------------------------\n");
	printf("--%s--\n",msgError);
	printf("\n------------------------------------------------------------\n");
}

void printBuffer(int size, char * buffer){
	int i = 0;
	printf("\nLeyendo Buffer # bytes: %d ------------\n",size);
	while (i < size){
			printf("!%hhX!",buffer[i]);
			i = i + 1;
		}
	if(buffer)
	printf("\n%s\n",buffer);

}


void saveTime(int size, char * buffer){

}
void savePosition( int size, char * buffer){

	printf("\n%s\n",buffer);
	if(buffer[0] == '$')
	{
	    char *lat = NULL;
	    char *lng = NULL;
	    char *alt = NULL;
	    int i=0;
	    int result = -1;
	    char *token = strtok(buffer,",");
	    while (token != NULL)
	    {
	    	printf("token_: %s\n", token);
	    	result = strcmp(token,"$GPGGA"); // Para comparar si dos cadenas son iguales. retorna 0 si son iguales

	        if(i == 0 && result != 0){
	        	token = NULL;
	        }
	        else if (i == 2 ){
	        	result = strcmp(token,"0000.0000");
	        	if(result != 0){
	        		lat = token;
	        		token = strtok(NULL, ",");
	        	}
	        	else{
	        		token = NULL;
	        	}
	        }
	        else if (i == 3 ){
	        	strcat(lat,token);
	        	token = strtok(NULL, ",");
	        }
	        else if (i == 4){
	        	lng = token;
	        	token = strtok(NULL, ",");
	        }
	        else if (i == 5){
	        	strcat(lng,token);
	        	token = strtok(NULL, ",");
	        }
	        else if (i == 9){
	        	alt = token;
	        	token = strtok(NULL, ",");

	        }else{
	        	token = strtok(NULL, ",");
	        }

	        i = i +1;
	    }
	    if( lat != NULL && lng != NULL && alt != NULL){
	    	ubicacion = fopen ("ubicacion.txt", "w+");
	    	printf("lat: %s | lng: %s | alt: %s\n",lat,lng,alt);
	    	fprintf(ubicacion,"lat: %s | lng: %s | alt: %s\n",lat,lng,alt);
	    	fclose(ubicacion);
	    }

	}


}
/*
char split(char * string){

    // Returns first token
    char *token = strtok(str,",");

    // Keep printing tokens while one of the
    // delimiters present in str[].

    while (token != NULL)
    {
        //printf("%s\n", token);
        token = strtok(NULL, ",");
    }

    return "0";
}*/




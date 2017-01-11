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
#include <time.h>

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
void configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA);
void printErrorMsg(char * msgError);
void printBuffer(int size, char * buffer);
void saveTime(int size, char * buffer);
void savePosition( int size, char * buffer);
void openUART(int baudRate);
void readUART();
void closeUART();
char checkSum(int pl, char * payload);

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

//TOCA VER EL CHECK SUM PARA QUE FUNCIONE, SINO NO FUNCIONA LOS
//COMANDOS DE CONFIGURACIÓN.


    //115200
    //configureMessageType(1);// NMEA
    //configureSerialPort(3);
    //configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA);
    //configureNMEA_Messages(1,0,0,0,0,0,0);
    time_t inicio, fin;
    int cont = 0;
    while (TRUE) {
//	printf("Inicio\n");

    	readUART();
    	if(cont == 0){
    				inicio = time(NULL);
    				//configureMessageType(2);
    				cont = 1;

    			}
	//sleep(1);
    	fin = time(NULL);
    	if(difftime(fin,inicio) > 5.0 && cont < 5){
    		inicio = time(NULL);

    		cont = 0;
    		/*if(cont == 1){
    			cont = 2;
    			printf("\nBinary\n");
    			configureMessageType(2);
    			//configureSerialPort(0);
    		}
    		else if(cont == 2){
    			cont = 3;
    			printf("\nNmea\n");
    			configureMessageType(1);
    			//configureSerialPort(1);
    		}
    		else if(cont == 3){
    			cont = 4;
    		    printf("\nNo output\n");
    		    configureMessageType(0);
    		    //configureSerialPort(2);
    		   }
    		else if(cont == 4){
    			cont = 5;
    		    printf("\n115000\n");
    		    //configureSerialPort(2);
    		}*/

    	}

//	printf("\nEn leer demoro: %f segundos.\n",difftime(fin,inicio));
	}
    closeUART();
    return 0;
}

void openUART(int baudRate){
	file = open(DEVICE, O_RDWR | O_NOCTTY | O_NDELAY);
	if (file < 0) {
		char msgError[] = "Error intentando abrir el dispositivo UART.";
		printErrorMsg(msgError);
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
		//printErrorMsg(msg);
		//perror(DEVICE);
		//readUART();
	}
	else{
		printBuffer(resultado, buf);
		//savePosition(resultado, buf);
		//saveTime(resultado, buf);
	}
}


void writeConfigMessage(char *buffer){
	int bytes;

	bytes = write(file,buffer,255);

	if(bytes < 0){
		char msg[] = "Error escribiendo en UART.";
		printErrorMsg(msg);
		perror(DEVICE);
	}

	//readUART(); // Para leer la respuesta del gps.
	//readUART();

}

void closeUART(){
	int fd = close(file);
	if(fd != -1 ){
		file = fd; // fd debe ser cero si la sentencia close es correcta.
	}
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

/* ----------------------------------------------------------------------------------------------------
 * ---Para la configuración del GPS se utiliza los mensajes binarios de SkyTraq Venus 6 GPS receiver---
 * ----------------------------------------------------------------------------------------------------
 * ---Sintaxis de los mensajes: <0xA0,0xA1><PL><Message ID><Message Body><CS><0x0D,0x0A> --------------
 * ----------------------------------------------------------------------------------------------------
 * PL -> PayLoad length --- CS -> CheckSum*/

/*-- Restaurar los parametros del GPS a valores de fabrica. --*/
void setFactoryDefaults(){
	char buffer[9] = {0};
	char payload[2] = {0};
	buffer[0] = 0xA0;
	buffer[1] = 0xA1;
	buffer[2] = 0x00;
	buffer[3] = 0x02; // PL -> PayLoad length

	payload[0] = 0x04; // 0x04 -> Es el id del mensaje "setFactoryDefaults"
	payload[1] = 0x01; // 0x01 -> reiniciar despues de configurar los valores por defecto

	buffer[4] =  payload[0];
	buffer[5] =  payload[1];
	buffer[6] = checkSum(2,payload); // CS -> CheckSum
	buffer[7] = 0x0D;
	buffer[8] = 0x0A;

	printf("Escribiendo: !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8]);

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
	printf("Inicio Cambio: %d\n", bauds);
	char payload[4] = {0};
	char buffer[11] = {0};
	buffer[0] = 0xA0;
	buffer[1] = 0xA1;
	buffer[2] = 0x00;
	buffer[3] = 0x04; // PL -> PayLoad length
	payload[0] = 0x05; // 0x05 -> Es el id de "configureSerialPort"
	payload[1] = 0x00; // 0x00 -> COM0

	switch(bauds){
	case 0:
		payload[2] = 0x00;  // -> 4800 // Aqui va la tasa de baudios -> por defecto es 9600
		break;
	case 1:
		payload[2] = 0x01; // -> 9600
		break;
	case 2:
		payload[2] = 0x03; // -> 38400
		break;
	case 3:
		payload[2] = 0x05; //  -> 115200
		break;
	default:
		payload[2] = 0x01; // -> 9600
		break;
	}
    /*--------------------------------------------------------------------------------------------*/
	payload[3] = 0x01; // 0x01 -> update to SRAM & FLASH  ---  0x00 -> only SRAM.
    /*---- Se elige 0x01 para que la configuracion no se pierda cuando se reinicie el sistema --- */
	buffer[4] = payload[0];
    buffer[5] = payload[1];;
    buffer[6] = payload[2];
    buffer[7] = payload[3];
	buffer[8] = checkSum(4,payload); // CS -> CheckSum
    buffer[9] = 0x0D;
    buffer[10] = 0x0A;
    //buffer[7] = 0x00;

	printf("Escribiendo: !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);
	writeConfigMessage(buffer); // escribe por el UART antes de modificar la tasa de baudios de la Beagle.

	closeUART();
	openUART(bauds);
}


/*-- type = 1 -> mensajes NMEA, type = 2 -> mensajes Binarios. -- type == 0 -> no output*/
void configureMessageType(int type)
{
	char buffer[9] = {0};
	char payload[2] = {0};
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
	writeConfigMessage(buffer);
}

/*-- Habilitar o deshabilitar los mensasjes NMEA --
 *-- Los parametros de esta función deben tomar los valores de 0x00 -> 0 o 0x01 -> 1 --
 *-- 0 -> deshabilitado
 *-- 1 -> habilitado --*/
void configureNMEA_Messages(int GGA, int GSA, int GSV, int GLL, int RMC, int VTG, int ZDA){

	if (GGA > 1 || GGA < 0 ||
		GSA > 1 || GSA < 0 ||
		GSV > 1 || GSV < 0 ||
		GLL > 1 || GLL < 0 ||
		RMC > 1 || RMC < 0 ||
		VTG > 1 || VTG < 0 ||
		ZDA > 1 || ZDA < 0 )
	{
		char msgError[] = "Error, los valores para configureNMEA_Messages deben ser : 0 o 1 ";
		printErrorMsg(msgError);
	}
	else{
		char buffer[16];
		char payload[9] = {0};
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
		payload[8] = ZDA; // -> ZDA
/*--------------------------------------------------------------------------------------------*/
		payload[9] = 0x01; // 0x01 -> update to SRAM & FLASH  ---  0x00 -> only SRAM.
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
		writeConfigMessage(buffer);
	}
}

void printErrorMsg(char *msgError){
	printf("\n------------------------Error-------------------------------");
	printf("\n %s",msgError);
	printf("\n------------------------------------------------------------\n");
}

void printBuffer(int size, char * buffer){
	int i = 0;
	buffer[size] = 0;
	printf("Leyendo Buffer # bytes: %d ------------\n",size);
	while (i < size){
			printf("!%hhX!",buffer[i]);
			i = i + 1;
		}
	printf("\n%s\n",buffer);

}

void saveTime(int size, char * buffer){

}

void savePosition( int size, char * buffer){
        time_t inicio, fin;
	printf("Inicio \n");
	inicio = time(NULL);
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
		//usleep(500000);
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
	fin = time(NULL);
	printf("\nEn leer demoro: %f segundos.\n",difftime(fin,inicio));
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




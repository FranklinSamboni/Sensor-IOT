#include <stdio.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include "rtc.h"

rtcStr rtc;
rtcData values;

int openI2C(char * rtcDevice){
	printf("Modificado RTC;\n");
	strcpy(rtc.device,rtcDevice);
	rtc.file = open(rtc.device, O_RDWR);
    if (rtc.file < 0) {
		printErrorMsgRtc("Error intentando abrir el dispositivo I2C.");
		perror(rtc.device);
		return -1;
    }
    if (ioctl(rtc.file, I2C_SLAVE, SLAVE_ADDRESS) < 0) {
        printErrorMsgRtc("Error al intentar conectar al dispositivo esclavo");
        perror(rtc.device);
        return -1;
    }
    return 1;

}

int readI2C(char * buffer){

	int resultado; /* Numero de bytes resultantes de la lectura. */

    /*char buffer[BUF_SIZE_I2C];*/

    resultado = read(rtc.file,buffer,BUF_SIZE_I2C);

    if(resultado < 0){
        printErrorMsgRtc("Error al intentar leer el bus I2C");
        perror(rtc.device);
        return -1;
    }
    buffer[resultado] = 0;
    return resultado;
}

int writeI2C(unsigned int address, unsigned int content){
	char buffer[2];
	int bytes;

	buffer[0] = address;
	buffer[1] = content;
	printf("Escribiendo: !%hhX!. En la dirección  !%hhX! \n",content, address);

	bytes = write(rtc.file,buffer,2);

    if( bytes != 2) {
    	printErrorMsgRtc("Error escribiendo en I2C.");
    	perror(rtc.device);
    	return -1;
    }
    return 1;
}

int closeI2C(){
	int fd = close(rtc.file);
	if(fd == -1 ){
		return -1;
	}
	rtc.file = fd; /* fd debe ser cero si la sentencia close es correcta.*/
	rtc.device[0] = 0;
	rtc.device[1] = 0;
	rtc.device[2] = 0;
	rtc.device[3] = 0;

	return 1;
}

void printErrorMsgRtc(char *msgError){
	printf("\nError RTCLIB: %s",msgError);
}

void saveDataRtc( int size, char * buffer){

	FILE *archivo = fopen ("fecha.txt", "w+");

	sprintf(values.date,"%02x%02x%02x",buffer[4],buffer[5],buffer[6]);
	sprintf(values.time,"%02x%02x%02x",buffer[2],buffer[1],buffer[0]);

	printf("RTC archivo -> date: %02x%02x%02x | time: %02x%02x%02x \n",buffer[4],buffer[5],buffer[6],buffer[2],buffer[1],buffer[0]);
	fprintf(archivo,"date: %02x%02x%02x | time: %02x%02x%02x \n",buffer[4],buffer[5],buffer[6],buffer[2],buffer[1],buffer[0]);

	fclose(archivo);
}

int getTimeRtc(char * buffer){

	if(values.time[0] == 0 && values.time[1] == 0 && values.time[2] == 0){
		printErrorMsgRtc("Error Tiempo no se ha capturado\n");
	    return -1;
	}

	int tam = sizeof(values.time);
	int count = 0;
	while(count < tam){

		if(values.time[count] == 0){
			break;
		}
		buffer[count] = values.time[count];
		count = count + 1;
	}
	return count;
}

int getDateRtc(char * buffer){

	if(values.date[0] == 0 && values.date[1] == 0 && values.date[2] == 0){
		printErrorMsgRtc("Error La fecha no se ha capturado\n");
	    return -1;
	}
	int tam = sizeof(values.date);
	int count = 0;
	while(count < tam){
		if(values.date[count] == 0){
			break;
		}
		buffer[count] = values.date[count];
		count = count + 1;
	}
	return count;
}

int setTimeRtc(char * buffer){

	printf("\nSincronizando RTC y GPS\n");

	int res = 0;

	char hor[2] = {buffer[0],buffer[1]};
	char min[2] = {buffer[2],buffer[3]};
	char vseg[2] = {buffer[4],buffer[5]};

	unsigned int nuh = (int)strtol(hor, NULL, 16);
	unsigned int num = (int)strtol(min, NULL, 16);
	unsigned int nus = (int)strtol(vseg, NULL, 16);

	/*printf("hora en Hexa es : !%hhX!!%hhX!!%hhX!!%hhX!!%hhX!!%hhX!\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);
	printf("hora en ASCII es : !%c!!%c!!%c!!%c!!%c!!%c!\n", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);

	printf("Convertido  !%hhX! !%hhX! !%hhX!\n", nuh,num,nus);*/


	res = writeI2C(0x02,nuh);
	if(res != -1){
		res = writeI2C(0x01,num);
		if(res != -1){
			res = writeI2C(0x00,nus);
 			if(res != -1){
				return 1;
			}
		}
	}

	printErrorMsgRtc("Fallo Sincronización");
	writeI2C(0x00,0x00);
	return -1;
}



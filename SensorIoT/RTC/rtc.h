#ifndef _RTC_H_
#define _RTC_H_

/*
 * El RTC usado es el DS3231 este se comunica a través del puerto I2C, para
 * su utilización es necesario abrir el puerto y enviar la dirección de esclavo
 * del dispositivo la cual es 0x68.
 *
 * A este RTC se le puede configurar a través de su registro de 19 direcciones
 * que van desde 0x00 a 0x12 se le puede configurar la hora y fecha, asi como
 * dos alarmas.
 *
 * La alarma 1 será configurada cada segundo como señal de sincronización similar a la señal
 * PPS que genera el dispositivo Venus GPS Logger.
 *
 * Para configurar esta alarma se pone 1 en los bits AM1,AM2,AM3 y AM4 de los registros 0x07,
 * 0x08,0x09 y 0x0A respectivamente, ademas se pone en 1 el bit A1IE del registro de control 0x0E.
 *
 * Cuando la alarma se activa el pin INT/SQW (Que es activo a nivel 0 - ActiveLow) se pone en 0 y es
 * necesario reiniciarlo, escribiendo 0 en el bit A1F del registro Status 0x0F.
 *
 * Para mas información consultar el datasheet del dispositivo.
 * https://datasheets.maximintegrated.com/en/ds/DS3231.pdf
 * */

	#define SLAVE_ADDRESS 0x68
	#define DECIVE_I2C "/dev/i2c-1"
	#define BUF_SIZE_I2C 0x13
	#define SAMPLES_DIR_R "muestras"
	typedef struct rtcStr rtcStr;
	typedef struct rtcData rtcData;

	struct rtcStr {
		int file; 	   			/* Identificador del archivo. */
		char device[24];
		//struct termios options; /* Opciones de configuración del UART. */
	};

	struct rtcData {
		char time[10];
		char date[10];
	};

	int openI2C(char * rtcDevice);
	int readI2C(char * buffer);
	int writeI2C(unsigned int address, unsigned int data);
	int closeI2C();

	void printErrorMsgRtc(char *msgError);
	void printData(char * buffer);

	void saveDataRtc(char * buffer, char * dir);

	int activeAlarmRtc();
	int desactiveAlarmRtc();

	void getTimeRtc(char * buffer, char *I2C_DATA);
	void getDateRtc(char * buffer, char *I2C_DATA);
	int setTimeRtc(char * buffer);

	void createDirRtc(char *dir, char * date, char *time);
#endif

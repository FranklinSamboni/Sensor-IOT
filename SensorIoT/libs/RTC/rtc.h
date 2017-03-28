#ifndef _RTC_H_
#define _RTC_H_

/*
 * El RTC usado es el DS3231 este se comunica a trav√©s del puerto I2C, para
 * su utilizaci√≥n es necesario abrir el puerto y enviar la direcci√≥n de esclavo
 * del dispositivo la cual es 0x68.
 *
 * A este RTC se le puede configurar a trav√©s de su registro de 19 direcciones
 * que van desde 0x00 a 0x12 se le puede configurar la hora y fecha, asi como
 * dos alarmas.
 *
 * La alarma 1 ser√° configurada cada segundo como se√±al de sincronizaci√≥n similar a la se√±al
 * PPS que genera el dispositivo Venus GPS Logger.
 *
 * Para configurar esta alarma se pone 1 en los bits AM1,AM2,AM3 y AM4 de los registros 0x07,
 * 0x08,0x09 y 0x0A respectivamente, ademas se pone en 1 el bit A1IE del registro de control 0x0E.
 *
 * Cuando la alarma se activa el pin INT/SQW (Que es activo a nivel 0 - ActiveLow) se pone en 0 y es
 * necesario reiniciarlo, escribiendo 0 en el bit A1F del registro Status 0x0F.
 *
 * Para mas informaci√≥n consultar el datasheet del dispositivo.
 * https://datasheets.maximintegrated.com/en/ds/DS3231.pdf
 * */

	#define SLAVE_ADDRESS 0x68
	#define DECIVE_I2C "/dev/i2c-2"
	#define BUF_SIZE_I2C 0x13

	#define SECONDS 0x00 //00ñ59
	#define MINUTES 0x01 //00ñ59
	#define HOURS 0x02 //1ñ12 + AM/PM 00ñ23
	#define DAY_OF_WEEK 0x03 // 1 - 7
	#define DAY_OF_MONTH 0x04 // 0 - 31
	#define MONTH_CENTURY 0x05 // 01ñ12 + Century
	#define YEAR 0x06 // 00ñ99

	#define A1M1_SECONDS 0x07 //00ñ59
	#define A1M2_MINUTES 0x08 //00ñ59
	#define A1M3_HOUR 0x09 //1ñ12 + AM/PM 00ñ23
	#define A1M4_DAY_DATE 0x0A // 1 - 7 or 0 - 31

	#define A2M2_MINUTES 0x0B //00ñ59
	#define A2M3_HOUR 0x0C //1ñ12 + AM/PM 00ñ23
	#define A2M4_DAY_DATE 0x0D // 1 - 7 or 0 - 31

	#define EOSC 0x0E
	#define OSF 0x0F
	#define SIGN1 0x10
	#define SIGN2 0x11
	#define DATA 0x12

	typedef struct rtcStr rtcStr;
	typedef struct rtcData rtcData;

	struct rtcStr {
		int file; 	   			/* Identificador del archivo. */
		char device[24];
		//struct termios options; /* Opciones de configuraci√≥n del UART. */
	};


	int openI2C(char * rtcDevice);
	int readI2C(char * buffer);
	int writeI2C(unsigned int address, unsigned int data);
	int closeI2C();

	void errorRtc(char *msgError);
	void printData(char * buffer);

	void saveDataRtc(char * buffer, char * dir);

	int activeAlarmRtc();
	int desactiveAlarmRtc();

	void getTimeRtc(char * buffer, char *I2C_DATA);
	void getDateRtc(char * buffer, char *I2C_DATA);
	int setTimeRtc(char * buffer);
	int setDateRtc(char * buffer);

#endif

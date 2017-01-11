#ifndef _RTC_H_
#define _RTC_H_

	#define SLAVE_ADDRESS 0x68
	#define DECIVE_I2C "/dev/i2c-1"
	#define BUF_SIZE_I2C 0x13

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

	void saveDataRtc( int size, char * buffer);

	int getTimeRtc(char * buffer);
	int getDateRtc(char * buffer);
	int setTimeRtc(char * buffer);

#endif

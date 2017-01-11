#ifndef _GPIOT_H_
#define _GPIOT_H_


	#define GPIO_PATH "/sys/class/gpio/"
	//typedef int (*CallbackType)(int);

	typedef enum gpioDirection gpioDirection;
	typedef enum gpioValue gpioValue;
	typedef enum gpioEdge gpioEdge;

	enum gpioDirection{ INPUT, OUTPUT };
	enum gpioValue{ LOW=0, HIGH=1 };
	enum gpioEdge{ NONE, RISING, FALLING, BOTH };

	typedef struct gpioParams gpioParams;

	struct gpioParams{
		int number;
		char name[10], path[24];
	};

	void initGPIO(int number, gpioParams* params);

	int exportGPIO(gpioParams* params);
	int unexportGPIO(gpioParams* params);

	int writeInt(char * path, char * filename, int value);
	int write(char * path, char * filename, char * value);
	int read(char * path, char * filename, char * buffer);

	int setDirection(gpioDirection direction, gpioParams* params);
	int setValue(gpioValue value, gpioParams* params);
	int setEdgeType(gpioEdge edge, gpioParams* params);

	gpioDirection getDirection(gpioParams* params);
	gpioValue getValue(gpioParams* params);
	gpioEdge getEdgeType(gpioParams* params);

	int switchOutputValue(gpioParams* params); // toggleOutput

	void destroyGPIO();  //destructor will unexport the pin

	void eraseParams(gpioParams* params);
	void printErrorMsgGpio(char *msgError);

#endif


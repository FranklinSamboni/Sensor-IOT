#include <stdio.h>
#include "gpioTest.h"

// Directions INPUT, OUTPUT
// Values LOW, HIGH

int main(){

	printf("Hola mundo\n");
	gpioParams gpio66,gpio67;

	initGPIO(66, &gpio66);
	initGPIO(67, &gpio67);

	setDirection(INPUT, &gpio66);
	setDirection(OUTPUT, &gpio67);
	setValue(HIGH,&gpio67);
	int i = 0;
	while(1){
		//switchOutputValue(&gpio66);
		//switchOutputValue(&gpio67);
		getValue(&gpio66);
		//usleep(10000);
		i=i+1;
	}
	destroyGPIO(&gpio66);
	destroyGPIO(&gpio67);

	return 0;
}

lib = libs/

sacdir = $(lib)SAC_FILES/
adcdir = $(lib)ADC/
gpsdir = $(lib)GPS/
rtcdir = $(lib)RTC/
gpiodir = $(lib)GPIO/
socketdir = $(lib)SOCKET/
jsonfilesdir = $(lib)JSON_FILES/

MAIN = SensorIoT.c

OBJECTS = $(jsonfilesdir)filesJ.o $(adcdir)adc.o $(gpsdir)gps.o $(rtcdir)rtc.o $(gpiodir)gpio.o $(sacdir)sacsubc.o  $(socketdir)socketlib.o

SensorIoT : $(OBJECTS)
	gcc tests/testGps.c -o tests/testGps $(OBJECTS) -lrt -l json
	gcc tests/testAdc.c -o tests/testAdc $(OBJECTS) -lrt -l json
	gcc tests/testRtc.c -o tests/testRtc $(OBJECTS) -lrt -l json
	gcc $(MAIN) -o SensorIoT $(OBJECTS) -lrt -l json

$(jsonfilesdir)filesJ.o : $(jsonfilesdir)filesJ.c $(jsonfilesdir)
	gcc -o $(jsonfilesdir)filesJ.o -lrt -l json -c $(jsonfilesdir)filesJ.c 

$(sacdir)sacsubc.o : $(sacdir)sacsubc.c $(sacdir)
	gcc -o $(sacdir)sacsubc.o -c $(sacdir)sacsubc.c

$(adcdir)adc.o : $(adcdir)adc.c $(adcdir)
	gcc -o $(adcdir)adc.o -c $(adcdir)adc.c

$(gpsdir)gps.o : $(gpsdir)gps.c $(gpsdir)
	gcc -o $(gpsdir)gps.o -c $(gpsdir)gps.c

$(rtcdir)rtc.o : $(rtcdir)rtc.c $(rtcdir)
	gcc -o $(rtcdir)rtc.o -c $(rtcdir)rtc.c

$(gpiodir)gpio.o : $(gpiodir)gpio.c $(gpiodir)
	gcc -o $(gpiodir)gpio.o -c $(gpiodir)gpio.c

$(socketdir)socketlib.o : $(socketdir)socketlib.c $(socketdir)
	gcc -o $(socketdir)socketlib.o -c $(socketdir)socketlib.c

.PHONY : clean
clean :
	-rm SensorIoT $(OBJECTS)



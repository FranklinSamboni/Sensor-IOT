lib = libs/
testsDir = tests/

sacdir = $(lib)SAC_FILES/
adcdir = $(lib)ADC/
gpsdir = $(lib)GPS/
rtcdir = $(lib)RTC/
gpiodir = $(lib)GPIO/
socketdir = $(lib)SOCKET/

testsDir = $(tests)

MAIN = sensor.c

OBJECTS = $(adcdir)adc.o $(gpsdir)gps.o $(rtcdir)rtc.o $(gpiodir)gpio.o $(sacdir)sacsubc.o $(socketdir)socketlib.o

sensor : $(OBJECTS)
	gcc $(MAIN) -o sensor $(OBJECTS) -lrt -l json
	gcc tests/testGps.c -o tests/testGps $(OBJECTS) -lrt -l json

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
	-rm sensor $(OBJECTS)



gpsdir = GPS/
rtcdir = RTC/
gpiodir = GPIO/
MAIN = main.c
OBJECTS = $(gpsdir)gps.o $(rtcdir)rtc.o $(gpiodir)gpio.o

build : $(OBJECTS)
	gcc $(MAIN) -o build $(OBJECTS)

$(gpsdir)gps.o : $(gpsdir)gps.c $(gpsdir)
	gcc -o $(gpsdir)gps.o -c $(gpsdir)gps.c

$(rtcdir)rtc.o : $(rtcdir)rtc.c $(rtcdir)
	gcc -o $(rtcdir)rtc.o -c $(rtcdir)rtc.c

$(gpiodir)gpio.o : $(gpiodir)gpio.c $(gpiodir)
	gcc -o $(gpiodir)gpio.o -c $(gpiodir)gpio.c

.PHONY : clean
clean :
	-rm build $(OBJECTS)

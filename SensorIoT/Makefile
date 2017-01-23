lib = libs/
gpsdir = $(lib)GPS/
rtcdir = $(lib)RTC/
gpiodir = $(lib)GPIO/
socketdir = $(lib)SOCKET/
MAIN = main.c
OBJECTS = $(gpsdir)gps.o $(rtcdir)rtc.o $(gpiodir)gpio.o $(socketdir)socketlib.o

main : $(OBJECTS)
	gcc $(MAIN) -o main $(OBJECTS) -lrt -l json

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
	-rm main $(OBJECTS)


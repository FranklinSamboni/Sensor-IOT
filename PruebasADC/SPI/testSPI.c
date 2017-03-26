#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device = "/dev/spidev1.0";

uint8_t mode = 1;
uint8_t bits = 8;
uint32_t speed = 1000000;
uint16_t delay = 1;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

//int transfer(int fd, unsigned char send[], unsigned char rec[], int len){
int transfer(int fd){

	/*unsigned char opcode1[1] = {0x25};
	unsigned char opcode2[1] = {0x01};
	unsigned char ceroOp[1] = {0x00};
	unsigned char recv[4] = {0};

	struct spi_ioc_transfer trOp1 = { // transfer structure
		.tx_buf = (unsigned long) opcode1, // buffer for sending data
		.rx_buf = (unsigned long) recv, //buffer for receiving data
		.len = 1, // length of buffer
		.speed_hz = speed,  // speed in Hz
		.bits_per_word = bits, // bits per word
		.delay_usecs = delay, // delay in us
		.cs_change = 0, // affects chip select after transfer
		// .tx_nbits = 0, // no. bits for writing (default 0)
		// .rx_nbits = 0, // no. bits for reading (default0)
		// .pad = 0, // interbyte delay - check version

	};

	struct spi_ioc_transfer trOp2 = { // transfer structure
		.tx_buf = (unsigned long) opcode2, // buffer for sending data
		.rx_buf = (unsigned long) recv, //buffer for receiving data
		.len = 1, // length of buffer
		.speed_hz = speed,  // speed in Hz
		.bits_per_word = bits, // bits per word
		.delay_usecs = delay, // delay in us
		.cs_change = 0, // affects chip select after transfer
		// .tx_nbits = 0, // no. bits for writing (default 0)
		// .rx_nbits = 0, // no. bits for reading (default0)
		// .pad = 0, // interbyte delay - check version

	};

	struct spi_ioc_transfer crOp = { // transfer structure
		.tx_buf = (unsigned long) ceroOp, // buffer for sending data
		.rx_buf = (unsigned long) recv, //buffer for receiving data
		.len = 1, // length of buffer
		.speed_hz = speed,  // speed in Hz
		.bits_per_word = bits, // bits per word
		.delay_usecs = delay, // delay in us
		.cs_change = 0, // affects chip select after transfer
		// .tx_nbits = 0, // no. bits for writing (default 0)
		// .rx_nbits = 0, // no. bits for reading (default0)
		// .pad = 0, // interbyte delay - check version

	};*/
	//08 o 09 start

	unsigned char RREG = 0x20;
	unsigned char WREG = 0x40;
	unsigned char ADDR = 0x00;
	unsigned char NUM_REGS = 0x0C;

	unsigned char START = 0x08;
	unsigned char READ = 0x12;
	unsigned char STOP = 0x0A;
	unsigned char DisablePGA = 0b10000100;
	/*unsigned char tx[8] = {
			WREG|0x05,0x00,DisablePGA
	};*/

	/*unsigned char tx[8] = {
			RREG|0x05,0x00,
	};*/

	unsigned char tx[8] = {
		0x00,
	};

	unsigned char rx[8] = {0};

	struct spi_ioc_transfer tr = { // transfer structure
		.tx_buf = (unsigned long) tx, // buffer for sending data
		.rx_buf = (unsigned long) rx, //buffer for receiving data
		.len = 8, // length of buffer
		.speed_hz = speed,  // speed in Hz
		.bits_per_word = bits, // bits per word
		.delay_usecs = delay, // delay in us
		.cs_change = 0, // affects chip select after transfer
		// .tx_nbits = 0, // no. bits for writing (default 0)
		// .rx_nbits = 0, // no. bits for reading (default0)
		// .pad = 0, // interbyte delay - check version

	};

	int status = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if(status < 0){
		perror("SPI: SPI_IOC_MESSAGE Failed");
		return -1;
	}

	printf("\n Recibiendo \n");

	unsigned long adc_count = 0;
	double resl = 0;
	double VREF = 2.5;

	float Vol_V = 0;
	float Vol_mV = 0;

	for (status = 0; status < 8; status++) {
		if (!(status % 6))
			puts("");
		printf("%.2X ", rx[status]);

	}
	adc_count = (((unsigned long)rx[1]<<24)|((unsigned long)rx[2]<<16)|(rx[3]<<8)|rx[4]);
	resl = VREF / pow(2.0, 31);

	Vol_V = resl * (double) adc_count;
	Vol_mV = Vol_V * 1000;


	printf("\nadc_count es: %lu\n", adc_count);
	printf("RESOLUTION es: %lu\n", resl);
	printf("Vol_V es: %f\n", Vol_V);
	printf("Vol_mV es: %f\n", Vol_mV);



	printf("\n");

	return status;
}

int main(){

	int ret = 0;
	int fd = 0;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	transfer(fd);

	/*unsigned char send[4] = {0x25, 0x01};
	unsigned char recv[ARRAY_SIZE(send)] = {0};

	ret = transfer(fd, (unsigned char *) &send[0],(unsigned char *) &recv[0], ARRAY_SIZE(send));
	if(ret == -1){
		printf("Error Con transfer\n");
		exit(1);
	}

	printf("\n Recibiendo \n");
	for (ret = 0; ret < ARRAY_SIZE(send); ret++) {
		if (!(ret % 6))
			puts("");
		printf("!%hhX! ", recv[ret]);
	}
	printf("\n");
	*/
	close(fd);

	return 0;
}

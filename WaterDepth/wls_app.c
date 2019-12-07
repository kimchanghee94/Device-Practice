#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>   
#include <unistd.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#include <linux/spi/spidev.h>

#define WLS_MAJOR_NUMBER	501
#define WLS_MINOR_NUMBER	101
#define WLS_DEV_PATH	"/dev/wls"

#define IOCTL_MAGIC_NUMBER 'j'
#define IOCTL_CMD_SET_SPI_ACTIVE _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_SET_SPI_INACTIVE _IOWR(IOCTL_MAGIC_NUMBER, 1, int)

static const char *spiDev0 = "/dev/spidev0.0";
static const char *spiDev1 = "/dev/spidev0.1";
static const uint8_t spiBPW = 8;
static const uint16_t spiDelay = 0;

int main(void){
	dev_t wls;
	int spi_fd, wls_fd;
	int mode = 0 & 3;
	int channel = 0 & 1;
	
	wls = makedev(WLS_MAJOR_NUMBER, WLS_MINOR_NUMBER);
	mknod(WLS_DEV_PATH, S_IFCHR | 0666, wls);
	
	wls_fd = open(WLS_DEV_PATH, O_RDWR);
	spi_fd = open(channel == 0 ? spiDev0 : spiDev1, O_RDWR);
	
	if(spi_fd<0){
		printf("fail to open spidev0.0\n");
		return -1;
	}
	if(wls_fd < 0){
		printf("fail to open water level sensor\n");
		return -1;
	}
	
	// Implementation
	int wls_value;
	unsigned char buff[3];	// communication with ADC
	struct spi_ioc_transfer spi;	// in "spidev.h"
	
	while(1){
		ioctl(wls_fd, IOCTL_CMD_SET_SPI_ACTIVE, &channel);
		
		// SYNC msg
		buff[0] = 0x06 | ((channel & 0x07) >> 7);
		buff[1] = ((channel & 0x07) << 6);
		buff[2] = 0x00;
		
		memset(&spi, 0, sizeof(spi));
		
		spi.tx_buf = (unsigned long)buff;
		spi.rx_buf = (unsigned long)buff;
		spi.len = 3;
		spi.delay_usecs = spiDelay;
		spi.speed_hz = 1000000;	// 1MHz
		spi.bits_per_word = spiBPW;
		
		ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
		buff[1] = 0x0F & buff[1];
		wls_value = (buff[1] << 8) | buff[2];
		
		ioctl(wls_fd, IOCTL_CMD_SET_SPI_INACTIVE, &channel);
		if(wls_value > 10)
			printf("wls_value is %u\n", wls_value);
	}
	
	close(wls_fd);
	close(spi_fd);
	return 0;
}

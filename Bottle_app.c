#define _CRT_SECURE_NO_WARNINGS
#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <linux/spi/spidev.h>

#define WLS_MAJOR_NUMBER	506
#define WLS_MINOR_NUMBER	101
#define WLS_DEV_PATH	"/dev/wls"

#define IOCTL_MAGIC_NUMBER 'j'
#define IOCTL_CMD_SET_SPI_ACTIVE _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_SET_SPI_INACTIVE _IOWR(IOCTL_MAGIC_NUMBER, 1, int)

#define LED_MAJOR_NUMBER 501
#define LED_MINOR_NUMBER 100
#define LED_DEV_PATH_NAME	"/dev/led"

#define BUZZER_MAJOR_NUMBER	502
#define BUZZER_MINOR_NUMBER	100
#define	BUZZER_DEV_NAME	"buzzer"
#define BUZZER_DEV_PATH	"/dev/buzzer"

#define IOCTL_MAGIC_BUZZER	'k'
#define IOCTL_CMD_SOUND		_IOW(IOCTL_MAGIC_BUZZER, 0, int)

#define SERVO_MAJOR_NUMBER 500
#define SERVO_MINOR_NUMBER 100

#define SERVO_DEV_PATH  "/dev/servo"
#define IOCTL_MAGIC_SERVO 'i'

#define IOCTL_CMD_SET_RNG _IOWR(IOCTL_MAGIC_SERVO, 0, int)
#define IOCTL_CMD_SET_DAT _IOWR(IOCTL_MAGIC_SERVO, 1, int)
#define IOCTL_CMD_SET_MIN _IOWR(IOCTL_MAGIC_SERVO, 2, int)


#define BUF_SIZE 100
	
void * led_msg(void * arg);
void * buzzer_msg(void * arg);
void * servo_msg(void * arg);
void * wls_msg(void * arg);
void error_handling(char * msg);

static const char *spiDev0 = "/dev/spidev0.0";
static const char *spiDev1 = "/dev/spidev0.1";
static const uint8_t spiBPW = 8;
static const uint16_t spiDelay = 0;
int bef_status=0,tmp_status=0, led_power=0;
int buzzer_dev, led_dev;
// [0] : temper
// [1] : soil moisture
// [2] : light
char received_data[3] = {0, 0, 0};

int main(int argc, char * argv[]){	
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t servo_thread, wls_thread;
	pthread_t led_thread,buzzer_thread;

	void * thread_return;
	
	if(argc!=3){
		printf("Usage : %s <IP> <port>\n",argv[0]);
		exit(1);
	}
	
	dev_t wls;
	int wls_fd;
	wls = makedev(WLS_MAJOR_NUMBER, WLS_MINOR_NUMBER);
	mknod(WLS_DEV_PATH, S_IFCHR | 0666, wls);
	wls_fd = open(WLS_DEV_PATH, O_RDWR);
	
	if(wls_fd < 0){
		printf("fail to open water level sensor\n");
		return -1;
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
		
	buzzer_dev;
	buzzer_dev = makedev(BUZZER_MAJOR_NUMBER, BUZZER_MINOR_NUMBER);
	mknod(BUZZER_DEV_PATH, S_IFCHR | 0666, buzzer_dev);
	
	buzzer_dev = open(BUZZER_DEV_PATH, O_WRONLY);
	
	if(buzzer_dev < 0){
		printf("fail to open buzzer device driver\n");
		return -1;
	}
	
	led_dev = makedev(LED_MAJOR_NUMBER, LED_MINOR_NUMBER);
	mknod(LED_DEV_PATH_NAME, S_IFCHR|0666, led_dev);
	
	led_dev = open(LED_DEV_PATH_NAME, O_RDWR);
	
	if(led_dev<0){
		printf("fail to open led\n");
		return -1;
	}

	pthread_create(&led_thread, NULL, led_msg, (void*)&led_dev);				
	pthread_create(&buzzer_thread, NULL, buzzer_msg, (void*)&buzzer_dev);
	pthread_create(&servo_thread, NULL, servo_msg, (void*)&sock);
	pthread_create(&wls_thread, NULL, wls_msg, (void*)&wls_fd);
	pthread_join(servo_thread, &thread_return);
	pthread_join(wls_thread, &thread_return);
	pthread_join(buzzer_thread, &thread_return);
	pthread_join(led_thread, &thread_return);
	close(sock);
	close(wls_fd);
	return 0;
}

void * servo_msg(void * arg)
{
	int flag = 0;
	int f = 0;
	int sock=*((int*)arg);
	char name_msg[BUF_SIZE];
	int str_len;
	int bservo_min = 100;
	int sservo_min = 110;
	int servo_dat = 8;
	int servo_rng = 41;
	int sbef=0, scur=0;
	int bbef=0, bcur=0;
	
	int servo = makedev(SERVO_MAJOR_NUMBER, SERVO_MINOR_NUMBER);
	mknod(SERVO_DEV_PATH, S_IFCHR | 0666, servo);
    
	servo = open(SERVO_DEV_PATH, O_RDWR);
	
	if(servo < 0){
		printf("fail to open bservo\n");
		return (void*)-1;
	}
	ioctl(servo, IOCTL_CMD_SET_RNG, &servo_rng);
	
	while(1){
		str_len=read(sock, name_msg, BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		
		// received data
		switch(*name_msg){
			case '1':
				received_data[0] = *name_msg;
				break;
			case '2':
				received_data[0] = *name_msg;
				break;
			case '3':	// soil mositure
				received_data[1] = *name_msg;
				break;
			case '4':	// soil mositure
				received_data[1] = *name_msg;
				break;
			case '5':
				received_data[2] = *name_msg;
				break;
			case '6':
				received_data[2] = *name_msg;
				break;
			default:
				printf("name_msg error!\n");
				break;
		}	
		// received soil moisture sensor
		if(*name_msg == '3' || *name_msg == '4'){
			bbef=bcur;
			// already exist many water
			if(*name_msg == '3'){
				bcur = 0;
				if(!bcur && bbef){
					ioctl(servo, IOCTL_CMD_SET_MIN, &bservo_min);
					printf("close water bottle\n");
					servo_dat = 8;
					usleep(10);
					flag = ioctl(servo, IOCTL_CMD_SET_DAT, &servo_dat);
					f = 0;
					while(flag != 4){
						f++;
						if(f>1000){
							printf("timeout\n");
						}
					}
					flag = 0;
				}
			}			
			// flower need water
			else{
				bcur=1;
				if(bcur && !bbef){
					ioctl(servo, IOCTL_CMD_SET_MIN, &bservo_min);
					printf("open water bottle\n");
					servo_dat = 24;
					usleep(10);
					flag = ioctl(servo, IOCTL_CMD_SET_DAT, &servo_dat);
					f = 0;
					while(flag != 4){
						f++;
						if(f>1000){
							printf("timeout\n");
						}
					}
					flag = 0;
				}
			}
		}
		// received temp or light sensor
		else{
			sbef=scur;
			printf("select shadow sservo!!!!\n");
			
			// high temp and many light
			printf("tem? %c\n" ,received_data[0]);
			if(received_data[0] == '1' && received_data[2] == '5'){
				scur=1;
				if(scur && !sbef){
					ioctl(servo, IOCTL_CMD_SET_MIN, &sservo_min);
					printf("open Shadow panel\n");
					servo_dat = 24;
					usleep(10);
					flag = ioctl(servo, IOCTL_CMD_SET_DAT, &servo_dat);
					f = 0;
					while(flag != 4){
						f++;
						if(f>1000){
							printf("timeout\n");
						}
					}
					flag = 0;					
				}
			}
			// don't need umbrella
			else{
				scur=0;
				if(!scur && sbef){
					ioctl(servo, IOCTL_CMD_SET_MIN, &sservo_min);
					printf("Close Shadow panel\n");
					servo_dat = 8;
					usleep(10);
					flag = ioctl(servo, IOCTL_CMD_SET_DAT, &servo_dat);
					f = 0;
					while(flag != 4){
						f++;
						if(f>1000){
							printf("timeout\n");
						}
					}
					flag = 0;
				}
			}
		}
		sleep(1);
	}
	return NULL;
}

void * led_msg(void * arg){
	int led_dev = *((int*)arg);
	while(1){
		write(led_dev, &led_power,sizeof(int));
		usleep(500000);
	}		
}

void * buzzer_msg(void * arg){
	int buzzer_dev = *((int*)arg);
	
	while(1){
		if(!bef_status && tmp_status){
			int signal = 1;
			ioctl(buzzer_dev, IOCTL_CMD_SOUND, signal);
		}
	}
}

void * wls_msg(void * arg){
	int spi_fd, wls_fd;
	int mode = 0 & 3;
	int channel = 0 & 1;
	
	
	wls_fd = *((int*)arg);
	spi_fd = open(channel == 0 ? spiDev0 : spiDev1, O_RDWR);
		
	if(spi_fd<0){
		printf("fail to open spidev0.0\n");
		return (void*)-1;
	}
	
	// Implementation
	int wls_value;
	unsigned char buff[3];	// communication with ADC
	struct spi_ioc_transfer spi;	// in "spidev.h"
	
	while(1){
		// chip select gpio active in adc
		ioctl(wls_fd, IOCTL_CMD_SET_SPI_ACTIVE, &channel);
		
		// make msg
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
		
		// send and read 
		ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi);
		buff[1] = 0x0F & buff[1];
		wls_value = (buff[1] << 8) | buff[2];
		
		// chip select gpio inactive in adc
		ioctl(wls_fd, IOCTL_CMD_SET_SPI_INACTIVE, &channel);
		
		bef_status=tmp_status;
		printf("wls_value is %u\n", wls_value);
		
		if(wls_value<2000){
			led_power=1;
			tmp_status=1;
		}else{
			led_power=0;
			tmp_status=0;
		}
		sleep(2);
	}
	close(buzzer_dev);
	close(led_dev);
	close(spi_fd);
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

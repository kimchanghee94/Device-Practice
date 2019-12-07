#define _CRT_SECURE_NO_WARNINGS
#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>   
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define SERVO_MAJOR_NUMBER 501
#define SERVO_MINOR_NUMBER 100
#define SERVO2_MINOR_NUMBER 110

#define SERVO_DEV_PATH  "/dev/servo"

#define IOCTL_MAGIC_NUMBER 'i'
#define IOCTL_CMD_SET_RNG _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_SET_DAT _IOWR(IOCTL_MAGIC_NUMBER, 1, int)
#define IOCTL_CMD_SET_MIN _IOWR(IOCTL_MAGIC_NUMBER, 2, int)

#define BUF_SIZE 100
#define NAME_SIZE 20
	
void * recv_msg(void * arg);
void error_handling(char * msg);

int buzzer_dev;

char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];

dev_t servo;

int main(int argc, char * argv[])
{
 	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
   
   if(argc!=3){
		printf("Usage : %s <IP> <port>\n",argv[0]);
		exit(1);
	}
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");
   
   servo = makedev(SERVO_MAJOR_NUMBER, SERVO_MINOR_NUMBER);
   mknod(SERVO_DEV_PATH, S_IFCHR | 0666, servo);
    
   servo = open(SERVO_DEV_PATH, O_RDWR);
   
   if(servo < 0){
      printf("fail to open servo\n");
      return -1;
   }
   
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	close(buzzer_dev);
	return 0;
}

void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	int servo_min = 100;
	int servo_dat = 24;
	int servo_rng = 41;
	while(1){
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		ioctl(servo, IOCTL_CMD_SET_RNG, &servo_rng);
		ioctl(servo, IOCTL_CMD_SET_MIN, &servo_min);
		ioctl(servo, IOCTL_CMD_SET_DAT, &servo_dat);
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

#define _REENTRANT
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#define LED_MAJOR_NUMBER 501
#define LED_MINOR_NUMBER 100
#define LED_DEV_PATH_NAME	"/dev/led"

#define BUF_SIZE 100
#define NAME_SIZE 20
	
void * recv_msg(void * arg);
void error_handling(char * msg);

int led_dev;

char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char * argv[]){
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
	
	led_dev = makedev(LED_MAJOR_NUMBER, LED_MINOR_NUMBER);
	mknod(LED_DEV_PATH_NAME, S_IFCHR|0666, led_dev);
	
	led_dev = open(LED_DEV_PATH_NAME, O_RDWR);
	
	if(led_dev<0){
		printf("fail to open led\n");
		return -1;
	}
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(rcv_thread, &thread_return);
	close(sock);
	close(led_dev);
	return 0;
}


void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg);
	char name_msg[NAME_SIZE+BUF_SIZE];
	int str_len;
	while(1){
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		fputs(name_msg, stdout);

		int led_state=0;
		led_state=1;
		write(led_dev, &led_state,4);
		sleep(1);
		led_state=0;
		write(led_dev,&led_state,4);
		sleep(1);
	}
	return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

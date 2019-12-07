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


#define BUZZER_MAJOR_NUMBER	502
#define BUZZER_MINOR_NUMBER	100
#define	BUZZER_DEV_NAME	"buzzer"
#define BUZZER_DEV_PATH	"/dev/buzzer"

#define IOCTL_MAGIC_NUMBER	'j'
#define IOCTL_CMD_SOUND		_IOW(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_LOG		_IOW(IOCTL_MAGIC_NUMBER, 1, int)

#define INTERVAL	50000

#define BUF_SIZE 100
#define NAME_SIZE 20
	
void * recv_msg(void * arg);
void error_handling(char * msg);

int buzzer_dev;

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
		
	buzzer_dev = makedev(BUZZER_MAJOR_NUMBER, BUZZER_MINOR_NUMBER);
	mknod(BUZZER_DEV_PATH, S_IFCHR | 0666, buzzer_dev);
	
	buzzer_dev = open(BUZZER_DEV_PATH, O_WRONLY);
	
	if(buzzer_dev < 0){
		printf("fail to open buzzer device driver\n");
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
	
	while(1){
		printf("test\n");
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1);
		printf("stop\n");
		if(str_len==-1) 
			return (void*)-1;
		name_msg[str_len]=0;
		fputs(name_msg, stdout);
		
		int signal=0;
		if(signal){
			ioctl(buzzer_dev, IOCTL_CMD_SOUND, signal);
		}
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

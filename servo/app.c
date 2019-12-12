#include <stdio.h>
#include <fcntl.h>   
#include <unistd.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#define SERVO_MAJOR_NUMBER 500
#define SERVO_MINOR_NUMBER 100
#define SERVO2_MINOR_NUMBER 110

#define SERVO_DEV_PATH  "/dev/servo"

#define IOCTL_MAGIC_NUMBER 'i'
#define IOCTL_CMD_SET_RNG _IOWR(IOCTL_MAGIC_NUMBER, 0, int)
#define IOCTL_CMD_SET_DAT _IOWR(IOCTL_MAGIC_NUMBER, 1, int)
#define IOCTL_CMD_SET_MIN _IOWR(IOCTL_MAGIC_NUMBER, 2, int)


int main(void)
{
   dev_t servo;
   int servo_fd, servo_dat, servo_rng, servo_min;
   
   servo = makedev(SERVO_MAJOR_NUMBER, SERVO_MINOR_NUMBER);
   mknod(SERVO_DEV_PATH, S_IFCHR | 0666, servo);
    
   servo_fd = open(SERVO_DEV_PATH, O_RDWR);
   
   if(servo_fd < 0){
      printf("fail to open servo\n");
      return -1;
   }
   servo_rng = 41;
   ioctl(servo_fd, IOCTL_CMD_SET_RNG, &servo_rng);
   while(1){
      printf("min : ");
      scanf("%d", &servo_min);//100 or 110
      ioctl(servo_fd, IOCTL_CMD_SET_MIN, &servo_min);
      printf("dat : ");
      scanf("%d", &servo_dat);
      ioctl(servo_fd, IOCTL_CMD_SET_DAT, &servo_dat);
   }
   
   close(servo_fd);
   return 0;
}

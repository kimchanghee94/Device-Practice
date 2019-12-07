#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define BUZZER_MAJOR_NUMBER	502
#define BUZZER_MINOR_NUMBER 100
#define	BUZZER_DEV_NAME	"buzzer"

#define GPIO_BASE_ADDR	0x3F200000
#define GPFSEL1	0x04
#define	GPSET0	0x1c
#define	GPCLR0	0x28

#define IOCTL_MAGIC_BUZZER	'k'
#define IOCTL_CMD_SOUND		_IOW(IOCTL_MAGIC_BUZZER, 0, int)
#define IOCTL_CMD_LOG		_IOW(IOCTL_MAGIC_BUZZER, 1, int)

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset0;
volatile unsigned int *gpclr0;

int buzzer_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Buzzer driver open!!.....buzzer_open(), ioremap()\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
	gpset0 = (volatile unsigned int *)(gpio_base + GPSET0);
	gpclr0 = (volatile unsigned int *)(gpio_base + GPCLR0);
	
	*gpsel1 |= (1<<21);	// gpio 12 is setted output mode
	return 0;
}

int buzzer_release(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "Buzzer dirver closed!!......buzzer_release(), iounmap()\n");
	iounmap((void *)gpio_base);
	return 0;
}

// Data Send
long buzzer_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	int i=0;
	switch(cmd){
		// Print Log
		case IOCTL_CMD_LOG:
			if(arg == 1)
				printk(KERN_ALERT "DATA Received\n");
			else if(arg == 0)
				printk(KERN_ALERT "DATA Break\n");
			else
				printk(KERN_ALERT "IOCTL_CMD_LOG error!!\n");
			break;
		
		// SOUND START(gpio 4)
		case IOCTL_CMD_SOUND:
			for(i=0;i<4000;i++){
				*gpset0 |= (1<<17);
				udelay(150);
				*gpclr0 |= (1<<17);
				udelay(150);
			}
			break;		
		default:
			printk(KERN_ALERT "ioctl : command error\n");
	}
	return 0;
}

static struct file_operations buzzer_fops = {
	.owner = THIS_MODULE,
	.open = buzzer_open,
	.release = buzzer_release,
	.unlocked_ioctl = buzzer_ioctl
};

int __init buzzer_init(void){
	if(register_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME, &buzzer_fops) < 0)
		printk(KERN_ALERT "BUZZER driver initialization fail\n");
	else
		printk(KERN_ALERT "BUZZER dirver initialization success.......buzzer_init(), register()\n");
	return 0;
}

void __exit buzzer_exit(void){
	unregister_chrdev(BUZZER_MAJOR_NUMBER, BUZZER_DEV_NAME);
	printk(KERN_ALERT "BUZZER driver exit done......buzzer_exit(), unregister()\n");
}

module_init(buzzer_init);
module_exit(buzzer_exit);

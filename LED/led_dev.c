#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include <asm/mach/map.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#define LED_MAJOR_NUMBER	501
#define LED_DEV_NAME		"led"

#define GPIO_BASE_ADDR	0x3F200000
#define GPFSEL1 0x04
#define GPSET0	0x1C
#define GPCLR0	0x28

static void __iomem *gpio_base;
volatile unsigned int *gpsel1;
volatile unsigned int *gpset0;
volatile unsigned int *gpclr0;

int led_open(struct inode *inode, struct file *filp){
	printk(KERN_ALERT "LED driver open!!\n");
	
	gpio_base = ioremap(GPIO_BASE_ADDR, 0x60);
	gpsel1 = (volatile unsigned int *)(gpio_base + GPFSEL1);
	gpset0 = (volatile unsigned int *)(gpio_base +GPSET0);
	gpclr0 = (volatile unsigned int *)(gpio_base + GPCLR0);
	*gpsel1 |= (1<<9);
	return 0;
}

int led_release(struct inode * inode, struct file *filp){
	printk(KERN_ALERT "LED driver closed!!\n");
	iounmap((void *)gpio_base);
	return 0;
}

ssize_t led_write(struct file *filp, const char *buf, size_t size, loff_t *f_pos){
	int led_onoff=0;
	copy_from_user(&led_onoff, buf, 4);
	if(led_onoff==1){
		printk(KERN_ALERT "LED On!!\n");
		*gpset0 |= (1<<13);
	}
	else{
		printk(KERN_ALERT "LED Off!!\n");
		*gpclr0 |= (1<<13);
	}
	return size;
}

static struct file_operations led_fops= {
	.owner = THIS_MODULE,
	.write = led_write,
	.open = led_open,
	.release = led_release
};

int __init led_init(void){
	if(register_chrdev(LED_MAJOR_NUMBER, LED_DEV_NAME, &led_fops)<0){
		printk(KERN_ALERT "LED driver initialization fail\n");
	}
	else{
		printk(KERN_ALERT "LED driver initialization success\n");
	}
	return 0;
}

void __exit led_exit(void){
	unregister_chrdev(LED_MAJOR_NUMBER, LED_DEV_NAME);
	printk(KERN_ALERT "LED driver exit done\n");
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jiwoong");
MODULE_DESCRIPTION("des");



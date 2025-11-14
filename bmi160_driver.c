#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>


/* Meta Info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton@ZeroTwo");
MODULE_DESCRIPTION("A driver for BMI160 IMU sensor");


/* READ function for device node:
	file *f -> represents the file being read
	char __user *u -> pointer to user-space 
		(__user tells kernel that the pointer lives in user-space)
	size_t l -> number of bytes that user-space wants to read
	loff_t *o -> file offset pointer
*/
static ssize_t read_dev(struct file *f, char __user *u, size_t l, loff_t *o){
	printk(KERN_DEBUG "bmi160 - Read is called\n");
	return 0;
}


/* variable to store the major CHARACTER device number of bmi160 driver */
static int major_dev_number;

static struct file_operations fops = {
	.read = read_dev
};


/* this function is called only when the module is loaded to kernel */
static int __init mod_init(void) {
	printk(KERN_INFO "bmi160 - Hello, Kernel!\n");

	/* passing zero as major is needed to allocate it dynamicly */
	major_dev_number = register_chrdev(0, "bmi160", &fops);
	if (major_dev_number < 0){
		printk(KERN_ERR "bmi160 - ERROR with allocating device major number");
		return major_dev_number;
	}

	printk(KERN_DEBUG "bmi160 - Major device number: %d\n", major_dev_number);
	return 0;
}

/* this function is called only when the module is removed from kernel */
static void __exit mod_exit(void) {
	unregister_chrdev(major_dev_number, "bmi160");
	printk(KERN_INFO "bmi160 - Goodbye, Kernel\n");
}

module_init(mod_init);
module_exit(mod_exit);


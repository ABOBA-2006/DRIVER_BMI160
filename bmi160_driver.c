#include <linux/module.h>
#include <linux/init.h>


/* Meta Info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton@ZeroTwo");
MODULE_DESCRIPTION("A driver for BMI160 IMU sensor");

/* this function is called only when the module is loaded to kernel */
static int __init my_init(void) {
	printk("Hello, Kernel!\n");
	return 0;
}

/* this function is called only when the module is removed from kernel */
static void __exit my_exit(void) {
	printk("Goodbye, Kernel\n");
}

module_init(my_init);
module_exit(my_exit);

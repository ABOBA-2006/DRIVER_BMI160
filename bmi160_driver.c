#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


/* Meta Info */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton@ZeroTwo");
MODULE_DESCRIPTION("A driver for BMI160 IMU sensor");
MODULE_VERSION("0.1");


/* READ function for device node:
	@f -> pointer to the file being read
	@u -> pointer to user-space buffer (__user tells kernel that the pointer lives in user-space)
	@l -> number of bytes that user-space wants to read
	@o -> file offset pointer

	returns: number of bytes read or negative error code
*/
static ssize_t read_dev_file(struct file *f, char __user *user_buffer, size_t len, loff_t *offset){
	printk(KERN_INFO "bmi160 - Read is called\n");

	// check if offset is beyond the buffer size
	if (*offset >= sizeof(text_buffer)){
		return 0; // EOF
	}

	// calculate number of bytes to copy
	int bytes_to_copy = (len + *offset) < sizeof(text_buffer) ? len : (sizeof(text_buffer) - *offset);

	// returns 0 if all bytes were copied successfully
	int bytes_not_copied = copy_to_user(user_buffer, text_buffer + *offset, bytes_to_copy);
	if (bytes_not_copied){
		printk(KERN_WARNING "bmi160 - Could not copy all bytes to user-space\n");
	}

	// update the offset
	*offset += bytes_to_copy;

	return bytes_to_copy;
}

/* WRITE function for device node:
	@f -> pointer to the file being read
	@u -> pointer to user-space buffer (__user tells kernel that the pointer lives in user-space)
	@l -> number of bytes that user-space wants to read
	@o -> file offset pointer

	returns: number of bytes written or negative error code
*/
static ssize_t write_dev_file(struct file *f, const char __user *user_buffer, size_t len, loff_t *offset){
	printk(KERN_INFO "bmi160 - Write is called\n");

	// check if offset is beyond the buffer size
	if (*offset >= sizeof(text_buffer)){
		return -ENOSPC; // No space left on device
	}

	// calculate number of bytes to copy
	int bytes_to_copy = (len + *offset) < sizeof(text_buffer) ? len : (sizeof(text_buffer) - *offset);

	// returns 0 if all bytes were copied successfully
	int bytes_not_copied = copy_from_user(text_buffer + *offset, user_buffer, bytes_to_copy);
	if (bytes_not_copied){
		printk(KERN_WARNING "bmi160 - Could not copy all bytes from user-space\n");
	}

	// update the offset
	*offset += bytes_to_copy;

	return bytes_to_copy;
}

/* OPEN function for device node (ex. user space program opens the file)
	@i -> pointer to inode struct representing this device in the kernel
	@f -> pointer to the file being opened

	returns: 0 on success or negative error code
*/
static int open_dev_file(struct inode *node, struct file *f){
	printk(KERN_INFO "bmi160 - Open is called\n");
	return 0;
}

/* RELEASE function for device node (ex. user space program closes the file)
	@i -> pointer to inode struct representing this device in the kernel
	@filp -> pointer to the file being closed

	returns: 0 on success or negative error code
*/
static int release_dev_file(struct inode *node, struct file *f){
	printk(KERN_INFO "bmi160 - Release is called\n");
	return 0;
}


// variable to store the major CHARACTER device number of bmi160 driver
static int major_dev_number;
// buffer to store data to be sent to user-space
static char text_buffer[256] = "";

static struct file_operations fops = {
	.read = read_dev_file,
	.write = write_dev_file,
	.open = open_dev_file,
	.release = release_dev_file
};


// this function is called only when the module is loaded to kernel
static int __init mod_init(void) {
	printk(KERN_INFO "bmi160 - Hello, Kernel!\n");

	// passing zero (as major) is needed to allocate it dynamicly
	major_dev_number = register_chrdev(0, "bmi160", &fops);
	if (major_dev_number < 0){
		printk(KERN_ERR "bmi160 - ERROR with allocating device major number");
		return major_dev_number;
	}

	printk(KERN_DEBUG "bmi160 - Major device number: %d\n", major_dev_number);
	return 0;
}

// this function is called only when the module is removed from kernel
static void __exit mod_exit(void) {
	unregister_chrdev(major_dev_number, "bmi160");
	printk(KERN_INFO "bmi160 - Goodbye, Kernel\n");
}

module_init(mod_init);
module_exit(mod_exit);


#include "bmi160_driver.h"


/* ------------------ GLOBAL VARIABLES ------------------ */

// device number (major and minor)
static dev_t dev_nr;

// character device struct
static struct cdev bmi_cdev;

// device class struct
static struct class *bmi_class;

// adapter to i2c bus (represents the i2c bus itself)
static struct i2c_adapter *bmi_i2c_adapter = NULL;

// i2c client struct (represents the specific device on the i2c bus)
static struct i2c_client *bmi_i2c_client = NULL;

// i2c device ID table for NON-DEVICE-TREE systems
static const struct i2c_device_id bmi_id[]={
	{SLAVE_DEVICE_NAME, 0},
	{}
};

// i2c driver struct
static struct i2c_driver bmi_driver = {
	.driver = {
		.name = SLAVE_DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};

// manual i2c device registration info for NON-DEVICE-TREE systems
static struct i2c_board_info bmi_i2c_board_info = {
	I2C_BOARD_INFO(SLAVE_DEVICE_NAME, BMI160_I2C_ADDRESS)
};

// buffer to store data to be sent to user-space
static char text_buffer[BUFFER_SIZE] = "";


/* ------------------ FILE OPERATIONS ------------------ */

static ssize_t read_dev_file(struct file *f, char __user *user_buffer, size_t len, loff_t *offset){
	printk(KERN_INFO "bmi160 - Read is called\n");

	// check if offset is beyond the buffer size
	if (*offset >= sizeof(text_buffer)){
		return 0; // EOF
	}

	// calculate number of bytes to copy
	int bytes_to_copy = (len + *offset) < sizeof(text_buffer) ? len : (sizeof(text_buffer) - *offset);

	s16 accel_x = read_accel_axis(0x12); // example: read X-axis acceleration
	printk(KERN_INFO "bmi160 - Acceleration X-axis: %d\n", accel_x);

	// returns the number of bytes NOT copied (0 means success)
	int bytes_not_copied = copy_to_user(user_buffer, text_buffer + *offset, bytes_to_copy);
	if (bytes_not_copied){
		printk(KERN_WARNING "bmi160 - Could not copy all bytes to user-space\n");
	}

	// update the offset
	*offset += bytes_to_copy;

	return bytes_to_copy;
}

static ssize_t write_dev_file(struct file *f, const char __user *user_buffer, size_t len, loff_t *offset){
	printk(KERN_INFO "bmi160 - Write is called\n");

	// check if offset is beyond the buffer size
	if (*offset >= sizeof(text_buffer)){
		return -ENOSPC; // No space left on device
	}

	// calculate number of bytes to copy
	int bytes_to_copy = (len + *offset) < sizeof(text_buffer) ? len : (sizeof(text_buffer) - *offset);

	// returns the number of bytes NOT copied (0 means success)
	int bytes_not_copied = copy_from_user(text_buffer + *offset, user_buffer, bytes_to_copy);
	if (bytes_not_copied){
		printk(KERN_WARNING "bmi160 - Could not copy all bytes from user-space\n");
	}

	// update the offset
	*offset += bytes_to_copy;

	return bytes_to_copy;
}

static int open_dev_file(struct inode *node, struct file *f){
	printk(KERN_INFO "bmi160 - Open is called\n");
	return 0;
}

static int release_dev_file(struct inode *node, struct file *f){
	printk(KERN_INFO "bmi160 - Release is called\n");
	return 0;
}

/* File operations supported by this character device */
static struct file_operations fops = {
	.read = read_dev_file,
	.write = write_dev_file,
	.open = open_dev_file,
	.release = release_dev_file,
	.owner = THIS_MODULE
};


/* ------------------ ADDITIONAL FUNCTIONS ------------------ */

s16 read_accel_axis(u8 register_low){
	u8 low = i2c_smbus_read_byte_data(bmi_i2c_client, register_low);
	u8 high = i2c_smbus_read_byte_data(bmi_i2c_client, register_low + 1);
	return (s16)((high << 8) | low);
}

/* ------------------ MODULE INIT & EXIT ------------------ */

static int __init mod_init(void) {
	printk(KERN_INFO "bmi160 - Hello, Kernel!\n");

	// dynamically allocate major and minor device numbers (saved in dev_nr)
	int status = alloc_chrdev_region(&dev_nr, 0, MINORMASK	+ 1, "bmi160");
	if (status){
		printk(KERN_ERR "bmi160 - ERROR with allocating device number");
		return status;
	}

	// initialize cdev structure and bind it to file operations table
	cdev_init(&bmi_cdev, &fops);
	bmi_cdev.owner = THIS_MODULE;

	// add the character device to the kernel (makes it active)
	status = cdev_add(&bmi_cdev, dev_nr, MINORMASK + 1);
	if (status){
		printk(KERN_ERR "bmi160 - ERROR with adding cdev to kernel");
		goto free_devnr;
	}

	printk(KERN_DEBUG "bmi160 - Major device number: %d\n", MAJOR(dev_nr));

	// create device class in /sys/class/
	bmi_class = class_create("bmi160_class");
	// check for failure
	if (!bmi_class){
		printk(KERN_ERR "bmi160 - ERROR with creating device class");
		status = -ENOMEM;
		goto delete_cdev;
	}

	// create the actual device node in /dev/ + check for failure
	if (!device_create(bmi_class, NULL, dev_nr, NULL, "bmi160_device")){
		printk(KERN_ERR "bmi160 - ERROR with creating device node");
		status = -ENOMEM;
		goto delete_class;
	}

	// test version of i2c client and driver registration
	bmi_i2c_adapter = i2c_get_adapter(I2C_BUS_NUMBER);
	if (bmi_i2c_adapter != NULL){
		bmi_i2c_client = i2c_new_client_device(bmi_i2c_adapter, &bmi_i2c_board_info);
		if (bmi_i2c_client != NULL){
			if(i2c_add_driver(&bmi_driver) != -1){
				status = 0;
			}else{
				printk(KERN_ERR "bmi160 - ERROR registering i2c driver\n");
				status = -ENODEV;
				goto delete_class;
			}
		}
		i2c_put_adapter(bmi_i2c_adapter); // release the adapter
	}

	return 0; // success

delete_class:
	class_unregister(bmi_class);
	class_destroy(bmi_class);
delete_cdev:
	cdev_del(&bmi_cdev);
free_devnr:
	unregister_chrdev_region(dev_nr, MINORMASK + 1);
	return status;
}

static void __exit mod_exit(void) {
	printk(KERN_INFO "bmi160 - Goodbye, Kernel\n");

	i2c_unregister_device(bmi_i2c_client);
	i2c_del_driver(&bmi_driver);
	// remove device node
	device_destroy(bmi_class, dev_nr);
	// unregister device class
	class_unregister(bmi_class);
	// destroy device class
	class_destroy(bmi_class);
	// remove the character device from the kernel
	cdev_del(&bmi_cdev);
	// release the device numbers
	unregister_chrdev_region(dev_nr, MINORMASK + 1);
}


module_init(mod_init);
module_exit(mod_exit);

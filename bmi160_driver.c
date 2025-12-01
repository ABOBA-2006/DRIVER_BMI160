#include "bmi160_driver.h"
#include <linux/delay.h>


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


/* ------------------ FILE OPERATIONS ------------------ */

static long int ioctl_dev_file(struct file *f, unsigned int cmd, unsigned long args){
	switch (cmd){
		case IOCTL_GET_ACCEL_X:{
			printk(KERN_INFO "bmi160 - GET_ACCEL_X is called");
			s16 accel_x = read_accel_gyro_axis(BMI160_ACCEL_X_REGISTER);
			if (copy_to_user((s16 __user *)args, &accel_x, sizeof(s16)))
				return -EFAULT; // failed to copy data to user space
			break;
		}

		case IOCTL_GET_ACCEL_Y:{
			printk(KERN_INFO "bmi160 - GET_ACCEL_Y is called");
			s16 accel_y = read_accel_gyro_axis(BMI160_ACCEL_Y_REGISTER);
			if (copy_to_user((s16 __user *)args, &accel_y, sizeof(s16)))
				return -EFAULT; // failed to copy data to user space
			break;
		}

		case IOCTL_GET_ACCEL_Z:{
			printk(KERN_INFO "bmi160 - GET_ACCEL_Z is called");
			s16 accel_z = read_accel_gyro_axis(BMI160_ACCEL_Z_REGISTER);
			if (copy_to_user((s16 __user *)args, &accel_z, sizeof(s16)))
				return -EFAULT; // failed to copy data to user space
			break;
		}
			
		case IOCTL_CALIBRATE_SENSOR:{
			printk(KERN_INFO "bmi160 - CALIBRATE_SENSOR is called");
			s16 flag = (s16)bmi160_calibrate_sensor();
			if (copy_to_user((s16 __user *)args, &flag, sizeof(s16)))
				return -EFAULT; // failed to copy data to user space
			break;
		}

		case IOCTL_GET_GYRO_X:{
			printk(KERN_INFO "bmi160 - GET_GYRO_X is called");
			s16 gyro_x = read_accel_gyro_axis(BMI160_GYRO_X_REGISTER);
			if (copy_to_user((s16 __user *)args, &gyro_x, sizeof(s16)))
				return -EFAULT; // failed to copy data to user space
			break;
		}		
		
		case IOCTL_GET_GYRO_Y:{
			printk(KERN_INFO "bmi160 - GET_GYRO_Y is called");
			s16 gyro_y = read_accel_gyro_axis(BMI160_GYRO_Y_REGISTER);
			if (copy_to_user((s16 __user *)args, &gyro_y, sizeof(s16)))
				return -EFAULT; // failed to copy data to user space
			break;
		}	
		
		case IOCTL_GET_GYRO_Z:{
			printk(KERN_INFO "bmi160 - GET_GYRO_Z is called");
			s16 gyro_z = read_accel_gyro_axis(BMI160_GYRO_Z_REGISTER);
			if (copy_to_user((s16 __user *)args, &gyro_z, sizeof(s16)))
				return -EFAULT; // failed to copy data to user space
			break;
		}	

		default:
			return -EOPNOTSUPP; // command not supported
	}
	
	return 0;
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
	.unlocked_ioctl = ioctl_dev_file,
	.open = open_dev_file,
	.release = release_dev_file,
	.owner = THIS_MODULE
};


/* ------------------ ADDITIONAL FUNCTIONS ------------------ */

s16 read_accel_gyro_axis(u8 register_low){
	u8 low = i2c_smbus_read_byte_data(bmi_i2c_client, register_low);
	u8 high = i2c_smbus_read_byte_data(bmi_i2c_client, register_low + 1);
	return (s16)((high << 8) | low);
}

static char *bmi160_devnode(const struct device *dev, umode_t *mode){
	if (mode)
		*mode = 0666; // rw-rw-rw-
	return NULL;
}

static int bmi160_init_sensor(void){
	const int init_delay = 100; // small delay after mode set
	int ret;

	// small delat to give some time for bmi160 on cold boot
	mdelay(init_delay);

	// set the accel normal mode
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_MODE_REGISTER, ACCEL_NORMAL_MODE);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to set the accel start");
		return ret;
	}

	// small delay to give some time to properly init the accel
	mdelay(init_delay);

	// set the gyro normal mode
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_MODE_REGISTER, GYRO_NORMAL_MODE);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to set the gyro start");
		return ret;
	}
		
	// small delay to give some time to properly init the gyro
	mdelay(init_delay);

	// set the update frequency for accel
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_ACCEL_CONF_REGISTER, ACCEL_UPDATE_100HZ);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to set the update frequency for accel");
		return ret;
	}

	// set the update frequency for gyro
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_GYRO_CONF_REGISTER, GYRO_UPDATE_100HZ);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to set the update frequency for gyro");
		return ret;
	}

	// set the accel range
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_ACCEL_RANGE_REGISTER, ACCEL_RANGE_2G);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to set the accel range");
		return ret;
	}
	
	// set the gyro range	
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_GYRO_RANGE_REGISTER, GYRO_RANGE_500DPS);
	
	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to set the gyro range");
		return ret;
	}

	return 0;
}

static int bmi160_calibrate_sensor(void){
	const int pre_cal_delay = 1500; // delay to give user some time to stabilize the imu sensor berfore calibration
	const int cal_delay = 1500; // delay after calling foc to apply it (avg 250ms)
	int ret;

	mdelay(pre_cal_delay);

	// set the FOC->enable to config (FOC = Fast Offset Compensation)
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_FOC_CONF_REGISTER, ENABLE_FOC);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to set the foc enable");
		return ret;
	}

	// set the FOC MODE (MUST BE IN THE NORMAL MODE BEFORE)
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_MODE_REGISTER, FOC_MODE);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to start FOC");
		return ret;
	}

	mdelay(cal_delay);

	// set the flag to use accel ang gyro calibrate data (offsets)
	ret = i2c_smbus_write_byte_data(bmi_i2c_client, BMI160_OFFSET_USE_REGISTER, USE_ACCEL_GYRO_OFFSET);

	if (ret < 0){
		printk(KERN_ERR "bmi160 - Failed to enable using accel and gyro offsets");
		return ret;
	}

	return ret;
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
	// give access to the dev file for all users
	bmi_class->devnode = bmi160_devnode;

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
				status = bmi160_init_sensor();
				if (status < 0) goto delete_class;
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

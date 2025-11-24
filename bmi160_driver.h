#ifndef BMI160_DRIVER_H
#define BMI160_DRIVER_H


/* ------------------ INCLUDES ------------------ */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/i2c.h>
#include <linux/kernel.h>


/* ------------------ META INFO ------------------ */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton@ZeroTwo");
MODULE_DESCRIPTION("A driver for BMI160 IMU sensor");
MODULE_VERSION("0.1b");


/* ------------------ DEFINES ------------------ */

/* device buffer size */
#define BUFFER_SIZE 256

/* I2C characteristics */
#define I2C_BUS_NUMBER 1 
#define BMI160_I2C_ADDRESS 0x68 // default address for BMI160
#define SLAVE_DEVICE_NAME "BMI160"

/* IOCTL command numbers */
// 'B' is a magic number to distinguish our commands from others
#define BMI160_IOCTL_MAGIC 'B'
#define IOCTL_GET_ACCEL_X _IOR(BMI160_IOCTL_MAGIC, 1, s16) // IOR -> copy kernel to user space
#define IOCTL_GET_ACCEL_Y _IOR(BMI160_IOCTL_MAGIC, 2, s16)
#define IOCTL_GET_ACCEL_Z _IOR(BMI160_IOCTL_MAGIC, 3, s16)
#define IOCTL_CALIBRATE_SENSOR _IO(BMI160_IOCTL_MAGIC, 4) // IO -> no data transfer

/*BMI160 register adresses*/
#define BMI160_ACCEL_X 0x12
#define BMI160_ACCEL_Y 0x14
#define BMI160_ACCEL_Z 0x16

/* ------------------ FILE OPERATIONS PROTOTYPES ------------------ */

/* IOCTL handler for device node (ex. user space program sends control commands)
    @f -> kernel file structure representing the opened device
    @cmd -> ioctl command number provided by user space
    @args -> pointer or integer argument passed from user space

    returns: command-specific value on success or negative error code
*/
static long int ioctl_dev_file(struct file *f, unsigned int cmd, unsigned long args);

/* OPEN function for device node (ex. user space program opens the file)
	@node -> pointer to inode struct representing this device in the kernel
	@f -> kernel file structure for the opened device

	returns: 0 on success or negative error code
*/
static int open_dev_file(struct inode *node, struct file *f);

/* RELEASE function for device node (ex. user space program closes the file)
	@node -> pointer to inode struct representing this device in the kernel
	@f -> kernel file structure for the opened device

	returns: 0 on success or negative error code
*/
static int release_dev_file(struct inode *node, struct file *f);


/* ------------------ ADDITIONAL FUNCTIONS PROTOTYPES ------------------ */

/* read accel axis from bmi160 imu sensor */
s16 read_accel_axis(u8 register_low);

/* ------------------ MODULE INIT & EXIT PROTOTYPES ------------------ */

/* this function is called only when the module is loaded to kernel */
static int __init mod_init(void);

/* this function is called only when the module is removed from kernel */
static void __exit mod_exit(void);


#endif // BMI160_DRIVER_H

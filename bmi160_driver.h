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
#define IOCTL_CALIBRATE_SENSOR _IOR(BMI160_IOCTL_MAGIC, 4, s16)
#define IOCTL_GET_GYRO_X _IOR(BMI160_IOCTL_MAGIC, 5, s16)
#define IOCTL_GET_GYRO_Y _IOR(BMI160_IOCTL_MAGIC, 6, s16)
#define IOCTL_GET_GYRO_Z _IOR(BMI160_IOCTL_MAGIC, 7, s16)

/* BMI160 register adresses */
#define BMI160_ACCEL_X_REGISTER 0x12
#define BMI160_ACCEL_Y_REGISTER 0x14
#define BMI160_ACCEL_Z_REGISTER 0x16
#define BMI160_GYRO_X_REGISTER 0x0C
#define BMI160_GYRO_Y_REGISTER 0x0E
#define BMI160_GYRO_Z_REGISTER 0x10
#define BMI160_MODE_REGISTER 0x7E
#define BMI160_ACCEL_CONF_REGISTER 0x40
#define BMI160_ACCEL_RANGE_REGISTER 0x41
#define BMI160_GYRO_CONF_REGISTER 0x42
#define BMI160_GYRO_RANGE_REGISTER 0x43
#define BMI160_FOC_CONF_REGISTER 0x69
#define BMI160_OFFSET_USE_REGISTER 0x77

/* BMI160 register values */
#define ACCEL_NORMAL_MODE 0x11
#define GYRO_NORMAL_MODE 0x15
#define FOC_MODE 0x03
#define ACCEL_RANGE_2G 0x03 
#define GYRO_RANGE_500DPS 0x02
#define ACCEL_UPDATE_100HZ 0x08
#define GYRO_UPDATE_100HZ 0x08
#define ENABLE_FOC 0x7D
#define USE_ACCEL_GYRO_OFFSET 0xC0

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
s16 read_accel_gyro_axis(u8 register_low);

/* function to set the file permissions  */
static char *bmi160_devnode(const struct device *dev, umode_t *mode);

/* send init params to bmi160 sensor such as: mode, frequency, accel range */
static int bmi160_init_sensor(void);

/* send the calibrate command to FOC register (Fast Offset Compensation) to start calibration proccess */
static int  bmi160_calibrate_sensor(void);

/* ------------------ MODULE INIT & EXIT PROTOTYPES ------------------ */

/* this function is called only when the module is loaded to kernel */
static int __init mod_init(void);

/* this function is called only when the module is removed from kernel */
static void __exit mod_exit(void);


#endif // BMI160_DRIVER_H

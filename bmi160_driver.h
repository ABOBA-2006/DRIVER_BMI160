#ifndef BMI160_DRIVER_H
#define BMI160_DRIVER_H


/* ------------------ INCLUDES ------------------ */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>


/* ------------------ META INFO ------------------ */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anton@ZeroTwo");
MODULE_DESCRIPTION("A driver for BMI160 IMU sensor");
MODULE_VERSION("0.1b");


/* ------------------ DEFINES ------------------ */
#define BUFFER_SIZE 256


/* ------------------ FILE OPERATIONS PROTOTYPES ------------------ */

/* READ:
	@f -> kernel file structure for the opened device
	@user_buffer -> buffer in user space where data must be copied
					(__user tells kernel that the pointer lives in user-space)
	@len -> number of bytes that user-space wants to read
	@offset -> file offset pointer

	returns: number of bytes read or negative error code
*/
static ssize_t read_dev_file(struct file *f, char __user *user_buffer, size_t len, loff_t *offset);

/* WRITE:
	@f -> kernel file structure for the opened device
	@user_buffer -> data coming from user space
	@len -> number of bytes that user-space wants to write
	@offset -> write position in the buffer

	returns: number of bytes written or negative error code
*/
static ssize_t write_dev_file(struct file *f, const char __user *user_buffer, size_t len, loff_t *offset);

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


/* ------------------ MODULE INIT & EXIT PROTOTYPES ------------------ */

/* this function is called only when the module is loaded to kernel */
static int __init mod_init(void);

/* this function is called only when the module is removed from kernel */
static void __exit mod_exit(void);


#endif // BMI160_DRIVER_H
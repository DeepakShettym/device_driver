#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World kernel module");
MODULE_VERSION("1.0");

dev_t dev = 0;
static struct class *deepak_class;
static struct cdev deepak_cdev;

static int __init deepak_driver_init(void);
static void __exit deepak_driver_exit(void);
static int  deepak_open(struct inode *inode, struct file *file);
static int deepak_release(struct inode *inode, struct file *file);
static ssize_t deepak_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t deepak_write(struct file *file, const char __user *buf, size_t len, loff_t *off);

static struct file_operations fops =
{
.owner          = THIS_MODULE,
.read           = deepak_read,
.write          = deepak_write,
.open           = deepak_open,
.release        = deepak_release,
};


static int  deepak_open(struct inode *inode, struct file *file)
{
	pr_info("file is opend by deepak");
	return 0;
}

static int deepak_release(struct inode *inode, struct file *file)
{
	pr_info("file is released by deepak");
	return 0;
}

static ssize_t deepak_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
	pr_info("file read by deepak");
	return 0;
}

static ssize_t deepak_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("file write by deepak");
	return len;
}


static int __init deepak_driver_init(void)
{
    int ret;

    // Allocate a major number for the device
    ret = alloc_chrdev_region(&dev, 0, 1, "deepak_dev");
    if (ret < 0) {
        pr_err("Cannot allocate major number\n");
        return ret;  // Return the error code from alloc_chrdev_region
    }
    
    // Print the major and minor numbers
    pr_info("Major: %d Minor: %d\n", MAJOR(dev), MINOR(dev));

    // Initialize the character device with file operations
    cdev_init(&deepak_cdev, &fops);

    // Add the device to the system
    ret = cdev_add(&deepak_cdev, dev, 1);
    if (ret < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;  // Jump to cleanup code if cdev_add fails
    }
          if(IS_ERR(deepak_class = class_create(THIS_MODULE,"deepak_class"))){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
	   if(IS_ERR(device_create(deepak_class,NULL,dev,NULL,"deepak_device"))){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
	pr_info("Device Driver Insert...Done!!!\n");
        return 0;

	r_device:
        class_destroy(deepak_class);
        r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}


// Exit function
static void __exit deepak_driver_exit(void)
{
     device_destroy(deepak_class,dev);
        class_destroy(deepak_class);
        cdev_del(&deepak_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}

// Register the init and exit functions
module_init(deepak_driver_init);
module_exit(deepak_driver_exit);

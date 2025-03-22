/***************************************************************************//**
 *  \file       driver.c
 *
 *  \details    Simple Linux device driver (procfs)
 *
 *  \author     DeepakM <deepakshettyksd@gmail.com>
 *
 **************************************************************************/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 // kmalloc()
#include <linux/uaccess.h>              // copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/proc_fs.h>
#include <linux/err.h>

/* Kernel version macro for compatibility with API changes */
#define LINUX_KERNEL_VERSION  510

/* IOCTL commands */
#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)

/* Global variables */
int32_t value = 0;

char dms_array[20] = "";


dev_t dev = 0;
static struct class *dev_class;
static struct cdev dms_cdev;
static struct proc_dir_entry *parent;

/* Function prototypes */
static int __init dms_driver_init(void);
static void __exit dms_driver_exit(void);

/* Driver functions */
static int dms_open(struct inode *inode, struct file *file);
static int dms_release(struct inode *inode, struct file *file);
static ssize_t dms_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t dms_write(struct file *filp, const char *buf, size_t len, loff_t *off);
static long dms_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* Procfs functions */
static int open_proc(struct inode *inode, struct file *file);
static int release_proc(struct inode *inode, struct file *file);
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length, loff_t *offset);
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t *off);

/* File operation structure */
static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .read           = dms_read,
    .write          = dms_write,
    .open           = dms_open,
    .unlocked_ioctl = dms_ioctl,
    .release        = dms_release,
};

#if (LINUX_KERNEL_VERSION > 505)
/* Procfs operation structure for kernel versions > 5.5 */
static const struct file_operations proc_fops = {
    .owner = THIS_MODULE,
    .open = open_proc,
    .read = read_proc,
    .write = write_proc,
    .release = release_proc,
};

#else
/* Procfs operation structure for older kernel versions */
static struct file_operations proc_fops = {
    .open    = open_proc,
    .read    = read_proc,
    .write   = write_proc,
    .release = release_proc,
};
#endif

/* Procfs open function */
static int open_proc(struct inode *inode, struct file *file) {
    pr_info("Proc file opened\n");
    return 0;
}

/* Procfs release function */
static int release_proc(struct inode *inode, struct file *file) {
    pr_info("Proc file released\n");
    return 0;
}

/* Procfs read function */
static ssize_t read_proc(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
    size_t data_len = strlen(dms_array);  // Use strlen for correct data length

    if (*offset >= data_len) {
        return 0;  // EOF
    }

    pr_info("Reading data from proc file\n");

    if (copy_to_user(buffer, dms_array, data_len)) {
        pr_err("Failed to copy data to user space\n");
        return -EFAULT;  // Return error
    }

    *offset += data_len;
    return data_len;  // Return actual bytes read
}

/* Procfs write function */
static ssize_t write_proc(struct file *filp, const char *buff, size_t len, loff_t *off) {
    pr_info("Proc file written\n");
    if (copy_from_user(dms_array, buff, len)) {
        pr_err("Data write: error\n");
    }
    return len;
}

/* Device file open function */
static int dms_open(struct inode *inode, struct file *file) {
    pr_info("Device file opened\n");
    return 0;
}

/* Device file release function */
static int dms_release(struct inode *inode, struct file *file) {
    pr_info("Device file closed\n");
    return 0;
}

/* Device file read function */
static ssize_t dms_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    pr_info("Device read\n");
    return 0;
}

/* Device file write function */
static ssize_t dms_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    pr_info("Device write\n");
    return len;
}

/* IOCTL function */
static long dms_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case WR_VALUE:
            if (copy_from_user(&value, (int32_t *)arg, sizeof(value))) {
                pr_err("Data write: error\n");
            }
            pr_info("Value = %d\n", value);
            break;
        case RD_VALUE:
            if (copy_to_user((int32_t *)arg, &value, sizeof(value))) {
                pr_err("Data read: error\n");
            }
            break;
        default:
            pr_info("Invalid IOCTL command\n");
            break;
    }
    return 0;
}

/* Module init function */
static int __init dms_driver_init(void) {
    /* Allocating major number */
    if (alloc_chrdev_region(&dev, 0, 1, "dms_Dev") < 0) {
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d, Minor = %d\n", MAJOR(dev), MINOR(dev));

    /* Creating cdev structure */
    cdev_init(&dms_cdev, &fops);

    /* Adding character device to the system */
    if (cdev_add(&dms_cdev, dev, 1) < 0) {   //use MKDEV(MAJOR(dev_num), i) for multiple periperals for this divice if used .
        pr_info("Cannot add the device to the system\n");  
        goto r_class;
    }

    /* Creating struct class */
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "dms_class"))) {
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    /* Creating device */
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "dms_device"))) {
        pr_info("Cannot create the device\n");
        goto r_device;
    }

    /* Creating proc directory */
    parent = proc_mkdir("dms", NULL);
    if (!parent) {
        pr_info("Error creating proc entry\n");
        goto r_device;
    }

    /* Creating proc entry under "/proc/dms/" */
    proc_create("dms_proc", 0666, parent, &proc_fops);

    pr_info("Device driver loaded successfully\n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

/* Module exit function */
static void __exit dms_driver_exit(void) {
    proc_remove(parent);                // Remove proc directory
    device_destroy(dev_class, dev);     // Destroy device
    class_destroy(dev_class);           // Destroy class
    cdev_del(&dms_cdev);                // Delete cdev
    unregister_chrdev_region(dev, 1);   // Unregister major number
    pr_info("Device driver removed successfully\n");
}

/* Module metadata */
module_init(dms_driver_init);
module_exit(dms_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DeepakM <deepakshettyksd@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (procfs)");
MODULE_VERSION("1.6");

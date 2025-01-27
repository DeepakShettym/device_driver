#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>     // kmalloc()
#include <linux/uaccess.h>  // copy_to/from_user()
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/err.h>

volatile int dms_value = 0; //volatile since the value might change 
volatile int dms_value1 = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dms_cdev;
struct kobject *kobj_ref; 

/*
** Function Prototypes
*/
static int __init dms_driver_init(void);
static void __exit dms_driver_exit(void);

/*************** Driver functions **********************/
static int dms_open(struct inode *inode, struct file *file);
static int dms_release(struct inode *inode, struct file *file);
static ssize_t dms_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t dms_write(struct file *filp, const char *buf, size_t len, loff_t *off);

/*************** Sysfs functions **********************/
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

static ssize_t sysfs_show1(struct kobject *kobj, struct kobj_attribute *attr1, char *buf);
static ssize_t sysfs_store1(struct kobject *kobj, struct kobj_attribute *attr1, const char *buf, size_t count);

struct kobj_attribute dms_attr = __ATTR(dms_value, 0660, sysfs_show, sysfs_store);
struct kobj_attribute dms_attr1 = __ATTR(dms_value1, 0660, sysfs_show1, sysfs_store1);

/*
** File operation structure
*/
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dms_read,
    .write = dms_write,
    .open = dms_open,
    .release = dms_release,
};

/*
** Sysfs show/store functions
*/
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    pr_info("Sysfs - Read from dms_value\n");
    return sprintf(buf, "%d", dms_value);
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    pr_info("Sysfs - Write to dms_value\n");
    sscanf(buf, "%d", &dms_value);
    return count;
}

static ssize_t sysfs_show1(struct kobject *kobj, struct kobj_attribute *attr1, char *buf) {
    pr_info("Sysfs - Read from dms_value1\n");
    return sprintf(buf, "%d", dms_value1);
}

static ssize_t sysfs_store1(struct kobject *kobj, struct kobj_attribute *attr1, const char *buf, size_t count) {
    pr_info("Sysfs - Write to dms_value1\n");
    sscanf(buf, "%d", &dms_value1);
    return count;
}

/*
** Driver open/release/read/write functions
*/
static int dms_open(struct inode *inode, struct file *file) {
    pr_info("Device File Opened\n");
    return 0;
}

static int dms_release(struct inode *inode, struct file *file) {
    pr_info("Device File Closed\n");
    return 0;
}

static ssize_t dms_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    pr_info("Read Function\n");
    return 0;
}

static ssize_t dms_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    pr_info("Write Function\n");
    return len;
}

/*
** Module Init function
*/
static int __init dms_driver_init(void) {
    if (alloc_chrdev_region(&dev, 0, 1, "dms_Dev") < 0) {
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&dms_cdev, &fops);
    if (cdev_add(&dms_cdev, dev, 1) < 0) {
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }

    if (IS_ERR(dev_class = class_create(THIS_MODULE, "dms_class"))) {
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "dms_device"))) {
        pr_info("Cannot create the Device\n");
        goto r_device;
    }
	//this will create a directory under /sys/kernel : since the seccond 
	// arg is kernel_kobj
    kobj_ref = kobject_create_and_add("dms_sysfs", kernel_kobj);

    if (sysfs_create_file(kobj_ref, &dms_attr.attr)) {
        pr_err("Cannot create sysfs file for dms_value\n");
        goto r_sysfs;
    }
    if (sysfs_create_file(kobj_ref, &dms_attr1.attr)) {
        pr_err("Cannot create sysfs file for dms_value1\n");
        goto r_sysfs;
    }

    pr_info("Device Driver Inserted Successfully\n");
    return 0;

r_sysfs:
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &dms_attr.attr);
    sysfs_remove_file(kernel_kobj, &dms_attr1.attr);

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&dms_cdev);
    return -1;
}

/*
** Module Exit function
*/
static void __exit dms_driver_exit(void) {
    kobject_put(kobj_ref);
    sysfs_remove_file(kernel_kobj, &dms_attr.attr);
    sysfs_remove_file(kernel_kobj, &dms_attr1.attr);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&dms_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Removed Successfully\n");
}

module_init(dms_driver_init);
module_exit(dms_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DeepakM <deepakshettyksd@gmail.com>");
MODULE_DESCRIPTION("Simple Linux device driver (sysfs)");
MODULE_VERSION("1.8");

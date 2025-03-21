#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/kthread.h>             //kernel threads
#include <linux/sched.h>               //task_struct 
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/err.h>

struct mutex dms_mutex;
unsigned long dms_global_variable = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dms_cdev;

static int __init dms_driver_init(void);
static void __exit dms_driver_exit(void);

static struct task_struct *dms_thread1;
static struct task_struct *dms_thread2; 

/*************** Driver functions **********************/
static int dms_open(struct inode *inode, struct file *file);
static int dms_release(struct inode *inode, struct file *file);
static ssize_t dms_read(struct file *filp, 
                char __user *buf, size_t len,loff_t * off);
static ssize_t dms_write(struct file *filp, 
                const char *buf, size_t len, loff_t * off);
 /******************************************************/
 
int thread_function1(void *pv);
int thread_function2(void *pv);

/*
** Thread function 1
*/
void function(int num){
    pr_info("deepak : %d",num);
}

int thread_function1(void *pv)
{
    while(!kthread_should_stop()) {
        mutex_lock(&dms_mutex);
        dms_global_variable++;
        pr_info("In DeepakM Thread Function1 %lu\n", dms_global_variable);
        function(1);
        mutex_unlock(&dms_mutex);
        msleep(1000);
    }
    return 0;
}

/*
** Thread function 2
*/
int thread_function2(void *pv)
{
    while(!kthread_should_stop()) {
        mutex_lock(&dms_mutex);
        dms_global_variable++;
        pr_info("In DeepakM Thread Function2 %lu\n", dms_global_variable);
        function(2);
        mutex_unlock(&dms_mutex);
        msleep(1000);
    }
    return 0;
}

// File operation structure
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = dms_read,
        .write          = dms_write,
        .open           = dms_open,
        .release        = dms_release,
};

/*
** This function will be called when we open the Device file
*/ 
static int dms_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

/*
** This function will be called when we close the Device file
*/
static int dms_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}

/*
** This function will be called when we read the Device file
*/ 
static ssize_t dms_read(struct file *filp, 
                char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t dms_write(struct file *filp, 
                const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}

/*
** Module Init function
*/
static int __init dms_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "dms_Dev")) <0){
                pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

        /*Creating cdev structure*/
        cdev_init(&dms_cdev,&fops);

        /*Adding character device to the system*/
        if((cdev_add(&dms_cdev,dev,1)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }

        /*Creating struct class*/
        if(IS_ERR(dev_class = class_create(THIS_MODULE,"dms_class"))){
            pr_info("Cannot create the struct class\n");
            goto r_class;
        }

        /*Creating device*/
        if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"dms_device"))){
            pr_info("Cannot create the Device \n");
            goto r_device;
        }

        mutex_init(&dms_mutex);
        
        /* Creating Thread 1 */
        dms_thread1 = kthread_run(thread_function1,NULL,"DMS Thread1");
        if(dms_thread1) {
            pr_err("Kthread1 Created Successfully...\n");
        } else {
            pr_err("Cannot create kthread1\n");
             goto r_device;
        }

        /* Creating Thread 2 */
        dms_thread2 = kthread_run(thread_function2,NULL,"DMS Thread2");
        if(dms_thread2) {
            pr_err("Kthread2 Created Successfully...\n");
        } else {
            pr_err("Cannot create kthread2\n");
             goto r_device;
        }
        
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;

r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        cdev_del(&dms_cdev);
        return -1;
}

/*
** Module exit function
*/ 
static void __exit dms_driver_exit(void)
{
        kthread_stop(dms_thread1);
        kthread_stop(dms_thread2);
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&dms_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!\n");
}

module_init(dms_driver_init);
module_exit(dms_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DeepakM <deepakshettyksd@gmail.com>");
MODULE_DESCRIPTION("A simple device driver - Mutex");
MODULE_VERSION("1.17");

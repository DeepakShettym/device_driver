#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>

static struct tasklet_struct my_tasklet;

// Tasklet handler function
void my_tasklet_handler(unsigned long data)
{
    pr_info("Tasklet executed with data: %ld\n", data);
    pr_info("Tasklet completed\n");
}

// Initialization function for the module
static int __init tasklet_driver_init(void)
{
    pr_info("Tasklet driver initialized\n");

    // Initialize the tasklet
    tasklet_init(&my_tasklet, my_tasklet_handler, 0);
    
    // Schedule the tasklet to run immediately
    tasklet_schedule(&my_tasklet);

    return 0;
}

// Cleanup function for the module
static void __exit tasklet_driver_exit(void)
{
    pr_info("Tasklet driver exited\n");

    // Make sure the tasklet is not running before we clean up
    tasklet_kill(&my_tasklet);
}

module_init(tasklet_driver_init);
module_exit(tasklet_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DeepakM");
MODULE_DESCRIPTION("A simple tasklet-based Linux driver");

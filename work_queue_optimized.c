#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

// Declare the workqueue structure
static struct workqueue_struct *my_wq;

// Define the work structure
static struct work_struct my_work;

// Work function to be executed
static void work_handler(struct work_struct *work)
{
    pr_info("Workqueue Running in Process Context! 🛠️\n");
    ssleep(2); // Sleep allowed in workqueue (Process Context)
    pr_info("Workqueue Task Completed ✅\n");
}

// Init function (runs when module is loaded)
static int __init my_module_init(void)
{
    pr_info("Loading Workqueue Example Module...\n");

    // Allocate a single-threaded workqueue
    my_wq = alloc_workqueue("my_wq", WQ_UNBOUND, 1);
    if (!my_wq)
    {
        pr_err("Failed to allocate workqueue\n");
        return -ENOMEM;
    }

    // Initialize work structure
    INIT_WORK(&my_work, work_handler);

    // Queue work
    queue_work(my_wq, &my_work);
    pr_info("Work Queued Successfully! 🚀\n");

    return 0;
}

// Exit function (runs when module is removed)
static void __exit my_module_exit(void)
{
    if (my_wq)
    {
        flush_workqueue(my_wq); // Ensure work is completed
        destroy_workqueue(my_wq);
    }
    pr_info("Unloading Workqueue Example Module...\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("DeepakShettym");
MODULE_DESCRIPTION("Simple Linux Kernel Workqueue Example");

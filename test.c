#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("Dual BSD/GPL");
static int ko_test_init(void) 
{
    printk("This is a test ko!\n");
    return 0;
}
static void ko_test_exit(void) 
{
    printk("Bye Bye~\n");
}
module_init(ko_test_init);
module_exit(ko_test_exit);
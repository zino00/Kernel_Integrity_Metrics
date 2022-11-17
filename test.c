#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
MODULE_LICENSE("Dual BSD/GPL");

unsigned long * sys_call_table;
static int (*anything_saved)(void);	/*定义一个函数指针，用来保存一个系统调用*/
static void get_syscall_table(void){
    int i;
    printk("My syscall is starting。。。\n");
	sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");	/* 获取系统调用服务首地址 */
   	printk("sys_call_table: 0x%p\n", sys_call_table);
    anything_saved = (int(*)(void))(sys_call_table[1]);	/* 保存原始系统调用 */
    for(i=0;i<=5;i++){
        printk("sys_exit:0x%p\n",sys_call_table[i]);
    }
    
}
static int ko_test_init(void) 
{
    get_syscall_table();
    return 0;
}
static void ko_test_exit(void) 
{
    printk("Bye Bye~\n");
}
module_init(ko_test_init);
module_exit(ko_test_exit);

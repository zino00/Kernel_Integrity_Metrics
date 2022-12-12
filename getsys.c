#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/types.h>
#include <crypto/hash.h> //md5
MODULE_LICENSE("Dual BSD/GPL");

unsigned long * sys_call_table;
static int (*anything_saved)(void);	/*定义一个函数指针，用来保存一个系统调用*/


bool md5_hash(char *result, char *data, size_t len)
{
	size_t size = 0;
	struct shash_desc *desc;
	struct crypto_shash **shash = NULL;
	shash = kmalloc(sizeof(struct crypto_shash *), GFP_KERNEL);
	if (NULL == shash) {
		return false;
	}
	*shash = crypto_alloc_shash("md5", 0, CRYPTO_ALG_ASYNC);
	size = sizeof(struct shash_desc) + crypto_shash_descsize(*shash);
	desc = kmalloc(size, GFP_KERNEL);
	if (desc->tfm == NULL) {
		return false;
	}
	desc->tfm = *shash;
	crypto_shash_init(desc);
	crypto_shash_update(desc, data, len);
	crypto_shash_final(desc, result);
	crypto_free_shash(desc->tfm);
	kfree(shash);
	kfree(desc);
	return true;
}
static char* get_syscall_table(void){
    int i=0;
    char* table;
    table="12";
    char * t2="13";
    char *result;
    printk("My syscall is starting...\n");
	sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");	/* 获取系统调用服务首地址 */
   	printk("sys_call_table: 0x%x\n", sys_call_table);
    anything_saved = (int(*)(void))(sys_call_table[1]);	/* 保存原始系统调用 */
    // for(i=0;i<=5;i++){
        printk("sys_exit:0x%d\n",sys_call_table[436]);
        printk("sys_exit:0x%d\n",sys_call_table[435]);
    // }
    // for(i=0;i<436;i++){
        
    // }
    md5_hash(result,table,2);
    printk("%s",result);
    md5_hash(result,t2,2);
    printk("%s",result);
    return table;
    
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

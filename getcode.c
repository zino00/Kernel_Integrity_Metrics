#include <linux/init.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <crypto/hash.h> //md5

MODULE_LICENSE("Dual BSD/GPL");
bool md5_hash(char *resultHex, char *data, size_t len)
{
    char result[16];
    static char* hex_str = "0123456789abcdef";

	size_t size = 0;
	struct shash_desc *desc;
	struct crypto_shash **shash = NULL;
	shash = kmalloc(sizeof(struct crypto_shash *), GFP_KERNEL);
	if (NULL == shash) {
        printk("kmalloc shash failed\n");
		return false;
	}
	*shash = crypto_alloc_shash("md5", 0, CRYPTO_ALG_ASYNC);
	size = sizeof(struct shash_desc) + crypto_shash_descsize(*shash);
	desc = kmalloc(size, GFP_KERNEL);
	desc->tfm = *shash;
	if (desc->tfm == NULL) {
        printk("kmalloc desc failed\n");
		return false;
	}    
	if(crypto_shash_init(desc)<0)
    {
        printk("crypto_shash_init failed\n");
        return false;
    }
	if(crypto_shash_update(desc, data, len)<0)
    {
        printk("crypto_shash_update failed\n");
        return false;
    }
	if(crypto_shash_final(desc, result)<0)
    {
        printk("crypto_shash_final failed\n");
        return false;
    }
	crypto_free_shash(desc->tfm);
	kfree(shash);
	kfree(desc);
    
    resultHex[32] = 0;
    int i=0;
    for(i=0;i<16;i++)
    {   
        char tmp = result[i];
        resultHex[2 * i] = hex_str[tmp & 0xf];
        resultHex[2 * i + 1] = hex_str[(tmp >> 4) & 0xf];
    }
	return true;
}
static int __init kallsyms_init(void)
{
	unsigned long stext, etext, size = 0;
	char *ptr = NULL;
	stext = kallsyms_lookup_name("_stext");
	etext = kallsyms_lookup_name("_etext");
	printk("%s: the _stext address is %lx\n", __func__, stext);
	printk("%s: the _etext address is %lx\n", __func__, etext);
	if (!stext || !etext) {
		printk("%s: Get text start end failed\n", __func__);
		return -EISDIR;
	}
	size = etext - stext;
	ptr = (char *)stext;
    printk("sizeof code: %ld\n", size);
	bool ret = false;
	char result[32];
	ret = md5_hash(result, ptr, size);
    if(!ret)
        printk("%s: hash failed", __func__);
    printk("md5: 0x%s", result);
	printk("%s: Finish\n", __func__);
	return 0;
}
static void __exit kallsyms_exit(void)
{
	printk("%s: good bye\n", __func__);
}
module_init(kallsyms_init);
module_exit(kallsyms_exit);
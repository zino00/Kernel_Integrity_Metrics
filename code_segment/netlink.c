#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>

#define NETLINK_TEST     31
#define MSG_LEN            125
#define USER_PORT        100

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Get kernel code segment status");

struct sock *nlsk = NULL;
extern struct net init_net;

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

static char* get_code_segment(void)
{
	unsigned long stext, etext, size = 0;
	char *ptr = NULL;
	stext = kallsyms_lookup_name("_stext");
	etext = kallsyms_lookup_name("_etext");
	size = etext - stext;
    // printk("Check code segment...\n");
	ptr = (char *)stext;
	char *result=kmalloc(32,GFP_KERNEL);
	md5_hash(result, ptr, size);
	return result;
}

int send_usrmsg(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    int ret;

    /* 创建sk_buff 空间 */
    nl_skb = nlmsg_new(len, GFP_ATOMIC);
    if(!nl_skb)
    {
        printk("netlink alloc failure\n");
        return -1;
    }

    /* 设置netlink消息头部 */
    nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, len, 0);
    if(nlh == NULL)
    {
        printk("nlmsg_put failaure \n");
        nlmsg_free(nl_skb);
        return -1;
    }

    /* 拷贝数据发送 */
    memcpy(nlmsg_data(nlh), pbuf, len);
    ret = netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);

    return ret;
}



static void netlink_rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;
    char *kmsg;
    if(skb->len >= nlmsg_total_size(0))
    {
        nlh = nlmsg_hdr(skb);
        umsg = NLMSG_DATA(nlh);
        if(umsg)
        {
            // printk("kernel recv from user: %s\n", umsg);
            kmsg=get_code_segment();
            send_usrmsg(kmsg, strlen(kmsg));
        }
    }
}

struct netlink_kernel_cfg cfg = {
        .input  = netlink_rcv_msg, /* set recv callback */
};

int test_netlink_init(void)
{
    /* create netlink socket */
    nlsk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_TEST, &cfg);
    if(nlsk == NULL)
    {
        printk("netlink_kernel_create error !\n");
        return -1;
    }
    printk("netlink_init\n");

    return 0;
}

void test_netlink_exit(void)
{
    if (nlsk){
        netlink_kernel_release(nlsk); /* release ..*/
        nlsk = NULL;
    }
    printk("netlink_exit!\n");
}

module_init(test_netlink_init);
module_exit(test_netlink_exit);
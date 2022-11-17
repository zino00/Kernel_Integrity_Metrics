/****************************************
* Author: zhangwj
* Date: 2017-01-19
* Filename: netlink_test.c
* Descript: netlink of kernel
* Kernel: 3.10.0-327.22.2.el7.x86_64
* Warning:
******************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/selinux_netlink.h>
// #include <net/netlink.h>

#define NETLINK_TEST     30
#define MSG_LEN            125
#define USER_PORT        100

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangwj");
MODULE_DESCRIPTION("netlink example");

struct sock *nlsk = NULL;
// extern struct net init_net;

int netlink_send_msg(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    /* 创建sk_buff 空间 */
    // nl_skb = nlmsg_new(len, GFP_ATOMIC);
    nl_skb = alloc_skb(len, GFP_KERNEL);
    if(!nl_skb)
    {
        printk("netlink alloc failure\n");
        return -1;
    }

    /* 设置netlink消息头部 */
    // nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_TEST, len, 0);
    nlh = NLMSG_PUT(nl_skb, 0, 0, NETLINK_TEST, len);
    // if(nlh == NULL)
    // {
    //     printk("nlmsg_put failaure \n");
    //     // nlmsg_free(nl_skb);
    //     kfree_skb(nl_skb);
    //     return -1;
    // }

    /* 拷贝数据发送 */
    // memcpy(nlmsg_data(nlh), pbuf, len);
    memcpy(NLMSG_DATA(nlh), pbuf, len);

    return netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);
nlmsg_failure:
	kfree_skb(nl_skb);
	return -EINVAL;
}

static void netlink_rcv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;
    char *kmsg = "hello users!!!";

    // if(skb->len >= nlmsg_total_size(0))
    if(skb->len >= NLMSG_SPACE(0))
    {
        // nlh = nlmsg_hdr(skb);
        nlh = (struct nlmsghdr *)skb->data;
        umsg = NLMSG_DATA(nlh);
        if(umsg)
        {
            printk("kernel recv from user: %s\n", umsg);
            netlink_send_msg(kmsg, strlen(kmsg));
        }
    }
}

// struct netlink_kernel_cfg cfg = {
//         .input  = netlink_rcv_msg, /* set recv callback */
// };

static void cn_input(struct sock *sk, int len)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(&sk->sk_receive_queue)) != NULL)
		netlink_rcv_msg(skb);
}

int test_netlink_init(void)
{   
    printk("test_netlink_start......\n");
    /* create netlink socket */
    nlsk = netlink_kernel_create(NETLINK_TEST, 4 + 0xf, cn_input,THIS_MODULE);
    if(nlsk == NULL)
    {
        printk("netlink_kernel_create error !\n");
        return 0;
    }
    printk("test_netlink_init\n");

    return 0;
}

void test_netlink_exit(void)
{
    if (nlsk){
        // netlink_kernel_release(nlsk); /* release ..*/
        sock_release(nlsk->sk_socket);
        // nlsk = NULL;
    }
    printk("test_netlink_exit!\n");
}

module_init(test_netlink_init);
module_exit(test_netlink_exit);
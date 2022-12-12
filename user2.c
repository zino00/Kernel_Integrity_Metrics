#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#define NETLINK_TEST       31
#define MSG_LEN            125
#define MAX_PLOAD          125

typedef struct _user_msg_info
{
    struct nlmsghdr hdr;
    char  msg[MSG_LEN];
} user_msg_info;

int main(int argc, char **argv)
{
    int skfd;
    int ret;
    user_msg_info u_info;
    socklen_t len;
    struct nlmsghdr *nlh = NULL;
    struct sockaddr_nl saddr, daddr;
    char *umsg = "The kernel integrity metrics module starts working\n";

    /* 创建NETLINK socket */
    skfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TEST);
    if(skfd == -1)
    {
        perror("[INFO] create socket error\n");
        return -1;
    }

    memset(&saddr, 0, sizeof(saddr));
    saddr.nl_family = AF_NETLINK; //AF_NETLINK
    saddr.nl_pid = 100;  //端口号(port ID)
    saddr.nl_groups = 0;
    if(bind(skfd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
    {
        perror("[INFO] bind() error\n");
        close(skfd);
        return -1;
    }

    memset(&daddr, 0, sizeof(daddr));
    daddr.nl_family = AF_NETLINK;
    daddr.nl_pid = 0; // to kernel
    daddr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PLOAD));
    memset(nlh, 0, sizeof(struct nlmsghdr));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PLOAD);
    nlh->nlmsg_flags = 0;
    nlh->nlmsg_type = 0;
    nlh->nlmsg_seq = 0;
    nlh->nlmsg_pid = saddr.nl_pid; //self port

    memcpy(NLMSG_DATA(nlh), umsg, strlen(umsg));
    ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
    if(!ret)
    {
        perror("[INFO] sendto error\n");
        close(skfd);
        exit(-1);
    }
    printf("[INFO] %s\n", umsg);

    memset(&u_info, 0, sizeof(u_info));
    len = sizeof(struct sockaddr_nl);
    ret = recvfrom(skfd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
    if(!ret)
    {
        perror("[INFO] recv form kernel error\n");
        close(skfd);
        exit(-1);
    }
    char true_result[32];
    strcpy(true_result,u_info.msg);
    printf("[INFO] Initial hash of the code segment: %s\n", u_info.msg);
    
    int check_cnt=0;
    while(1){
        check_cnt++;
        sleep(10);
        time_t tmpcal_ptr;
	    time(&tmpcal_ptr);
	    printf("[TIME:%d] ", tmpcal_ptr);
        memcpy(NLMSG_DATA(nlh), umsg, strlen(umsg));
        ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&daddr, sizeof(struct sockaddr_nl));
        if(!ret)
        {
            perror("\n[INFO]: sendto error\n");
            close(skfd);
            exit(-1);
        }
        memset(&u_info, 0, sizeof(u_info));
        len = sizeof(struct sockaddr_nl);
        ret = recvfrom(skfd, &u_info, sizeof(user_msg_info), 0, (struct sockaddr *)&daddr, &len);
        if(!ret)
        {
            perror("\n[INFO]: recv form kernel error\n");
            close(skfd);
            exit(-1);
        }
        if(!strcmp(u_info.msg,true_result)){
            printf("The code segment has not changed,the kernel is safe.\n");
        }
        else {
            printf("waring!!!  The code segment has been changed,the kernel is unsafe.\n");
        }
    }
    close(skfd);
    free((void *)nlh);
    return 0;
}
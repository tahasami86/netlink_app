#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/string.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Taha sami <tahasami8@gmail.com>");
MODULE_DESCRIPTION("Kernel NETLINK Module for Commuincation with DPDK Application");

#define NETLINK_USER 31
#define MAX_PAYLOAD 800

struct sock *nl_sk=NULL;

typedef struct my_data{

    int num_val;
    char msg[MAX_PAYLOAD];

}my_data;

static void netlink_rcv(struct sk_buff *skb) 
{ 
    struct nlmsghdr *nl_hdr;
    struct sk_buff *skb_out;
    int val;
    char *msg;
    // char reply_msg[MAX_PAYLOAD];
    // int reply_msg_size;
    int pid;
    //static int counter=0;

    my_data *data;
    my_data *reply_data = (my_data *)kmalloc(sizeof(my_data),GFP_KERNEL);
    
    if (!reply_data) {
        pr_err("Failed to allocate reply_data\n");
        return;
    }

    /*Extract Recevied message*/
    nl_hdr = (struct nlmsghdr *)skb->data;
    //msg = (char *)NLMSG_DATA(nl_hdr);
    data = (my_data *)NLMSG_DATA(nl_hdr);
    val = data->num_val;
    msg = data->msg;    
    
    pid = nl_hdr->nlmsg_pid; // pid of sending process

    /*print the recevied message*/
    pr_info("Received message from user-space \n");
    pr_info(": the DATA is --->>> %d \n",val);
    pr_info(": the MESSAGE is --->>> %s \n",msg);

    // /*Prepare reply message*/
    reply_data->num_val = 800;
    strncpy(reply_data->msg, "Hello from kernel", sizeof(reply_data->msg));
    reply_data->msg[sizeof(reply_data->msg) - 1] = '\0';

    // /*Allocate skb for reply*/
    skb_out = nlmsg_new(sizeof(my_data),GFP_KERNEL);
    if(!skb_out) {
        pr_err("Failed to allocate skb for reply \n");
        return;
    }

    pr_info("Sending reply \n");
    pr_info("The Reply DATA is ---->> %d\n",reply_data->num_val);
    pr_info("The Reply MESSAGE is ---->> %s\n",reply_data->msg);


    /*Put the reply_data in skb*/   
    nl_hdr = nlmsg_put(skb_out,0,0,NLMSG_DONE,sizeof(my_data),0);
    if (!nl_hdr) {
        pr_err("Failed to put nlmsg header\n");
        kfree_skb(skb_out);
        kfree(reply_data);
        return;
    }

    NETLINK_CB(skb_out).dst_group = 0;
    memcpy(nlmsg_data(nl_hdr),reply_data,sizeof(my_data));

    /*Send the reply*/
    if(nlmsg_unicast(nl_sk, skb_out, pid)){
        pr_err("Failed to send reply \n");
        goto free_skb;  // Free skb_out if unicast fails
    }


free_reply_data:
    kfree(reply_data);
out:
    return;

free_skb:
    kfree_skb(skb_out);
    goto free_reply_data;
}

static int __init link_init(void)
{
    struct netlink_kernel_cfg cfg = {
        .input = netlink_rcv,
    };

    pr_info("Loading NETLINK Module \n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER,&cfg);

    if(!nl_sk) {
        pr_err("Failed to create NETLINK socket \n");
        return -1;
    }

    return 0;
}

static void __exit link_exit(void)
{
    netlink_kernel_release(nl_sk);
    pr_info("Exiting NETLINK Module \n");

}

module_init(link_init)
module_exit(link_exit)

#include <linux/string.h>
#include <linux/module.h>
#include <net/sock.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>
#include "com_kmodule.h"

#define NETLINK_USER 31



struct sock *nl_sk = NULL;
struct mailbox *usermailbox[1000];


int kernel_nl_send_msg(char *msg_forsend, uint16_t len, int pid)
{


    int res;
    uint16_t lentest = 100;
    struct nlmsghdr *nlh;
    struct sk_buff *nl_buff;



    /* set buff for netlink */
    nl_buff = nlmsg_new(lentest, 0);
    if(!nl_buff)
    {
        printk("ker:netlink alloc failure\n");
        return -1;
    }

    /* set header of nlh */
    nlh = nlmsg_put(nl_buff, 0, 0, NLMSG_DONE, sizeof(NLMSG_DATA(nlh)), 0);
    printk("ker:nlh :%s\n",(char *)NLMSG_DATA(nlh));
    if(nlh == NULL)
    {
        printk("ker:set header of nlh failure\n");
        nlmsg_free(nl_buff);
        return -1;
    }

    //NETLINK_CB(nl_buff).dst_group = 0; /* not for multicast */


    memset(NLMSG_DATA(nlh),0, sizeof(nlh));

    /* set msg to nlh */
    printk("ker:msg_forsend:%s\n",msg_forsend);
    memcpy(NLMSG_DATA(nlh), msg_forsend, strlen(msg_forsend));




    printk("ker:nlh---------------- :%s\n",(char *)NLMSG_DATA(nlh));
    res =  nlmsg_unicast(nl_sk, nl_buff, pid);
    if(res < 0 )
    {
        printk(KERN_INFO "Error while (through unicast) sending to user\n");
    }
    return res;



}
static void kernel_nl_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh;

    char *regis = "Registration.";
    char *send = "Send";
    char *recv = "Recv";

    char *msg= NULL;
    char *umsg = NULL;
    int pid;

    if(skb->len >= nlmsg_total_size(0))
    {
        int userID;
        char *uID = NULL;
        char *instr = NULL;
        nlh = (struct nlmsghdr *)skb->data;
        msg = (char *)nlmsg_data(nlh);
        printk(KERN_INFO "ker:netlink recv msg in recv_msg: %s\n", msg);
        pid = nlh->nlmsg_pid; /*pid of sending process */
        if(msg)
        {
            printk("k: umsg :%s\n", umsg);

            /* deal with different msg from user */
            instr = strsep(&msg, " ");
            printk("ker: \n instr = %s\n",instr);



            if(strcmp(instr, regis) == 0)
            {
                char *type = NULL;
                char *uq = "uq";
                char *q = "q";
                printk("ker: reg success\n");


                /*get uId for regis mailbox */
                strsep(&msg,"=");
                uID = strsep(&msg, ",");
                userID = simple_strtol(uID,NULL,0);
                printk("ker: \n uID = %d\n",userID);
                printk("ker: \n imsg = %s\n",msg);


                strsep(&msg,"=");
                type = strsep(&msg, "\0");
                printk("ker: \n type = %d\n",type);

                /** check if this ID exists */
                if(usermailbox[userID] != NULL)
                {
                    printk("ker:ID has been register\n");
                    umsg = "Fail";
                }
                else
                {

                    /* usermailbox[id] = new mailbox */
                    usermailbox[userID] = kmalloc(sizeof(struct mailbox), GFP_USER);
                    if(usermailbox[userID] == NULL)
                    {
                        printk("ker: fail to kmalloc mailbox\n");
                    }
                    else
                    {
                        /* Init mailbox */
                        usermailbox[userID]->type = '0';
                        usermailbox[userID]->msg_data_count = 0;
                        usermailbox[userID]->msg_data_head = NULL;
                        usermailbox[userID]->msg_data_tail = NULL;
                    }


                    /* set mailbox->type */
                    if(strcmp(type, q) == 0)
                    {
                        /* quened msg malloc */
                        usermailbox[userID]->type = '1';
                        usermailbox[userID]->msg_data_head = kmalloc(sizeof(struct msg_data), GFP_USER);
                        usermailbox[userID]->msg_data_head->next = kmalloc(sizeof(struct msg_data), GFP_USER);
                        usermailbox[userID]->msg_data_head->next->next = kmalloc(sizeof(struct msg_data), GFP_USER);
                        usermailbox[userID]->msg_data_head->next->next->next = usermailbox[userID]->msg_data_head;
                        usermailbox[userID]->msg_data_tail = usermailbox[userID]->msg_data_head;

                    }
                    else if(strcmp(type, uq) == 0)
                    {
                        /* unquened msg malloc */
                        usermailbox[userID]->type = '0';
                        usermailbox[userID]->msg_data_head = kmalloc(sizeof(struct msg_data), GFP_USER);
                    }
                    else
                    {
                        printk("ker:no this type\n");
                    }
                    printk("ker: mailbox->type = %d\n",usermailbox[userID]->type);
                }


            }
            else if(strcmp(instr, send) == 0)
            {
                umsg = NULL;
                printk("ker: send success~~~~%s\n",umsg);

                /*get uId to send to target mailbox */
                uID = strsep(&msg, " ");
                userID = simple_strtol(uID,NULL,0);
                printk("ker: \n senduID = %d\n",userID);
                printk("ker: \n imsg = %s\n",msg);
                printk("ker: \n imsgsize = %ld\n",strlen(msg));

                if(strlen(msg) <= 256 )
                {

                    if(usermailbox[userID] == NULL)
                    {
                        printk("ker:this mailbox does not exit\n");
                        umsg = "Fail";
                    }
                    else
                    {
                        //check if quene | unquened
                        printk("ker:this mailbox exits type = %c\n",usermailbox[userID]->type);
                        if(usermailbox[userID]->type == '0')
                        {
                            //unquened
                            strcpy(usermailbox[userID]->msg_data_head->buf,msg);

                        }
                        else
                        {
                            //quened
                            printk("ker:Q------------------- type = %c\n",usermailbox[userID]->type);
                            if(usermailbox[userID]->msg_data_count < 3)
                            {
                                if(usermailbox[userID]->msg_data_count == 0)
                                {
                                    /* count == 0 */
                                    strcpy(usermailbox[userID]->msg_data_head->buf,msg);
                                    usermailbox[userID]->msg_data_count ++;
                                }
                                else
                                {
                                    /* count == 1-2 */
                                    strcpy(usermailbox[userID]->msg_data_tail->next->buf,msg);
                                    printk("ker:buff = %s\n",usermailbox[userID]->msg_data_tail->next->buf);
                                    usermailbox[userID]->msg_data_count ++;
                                    usermailbox[userID]->msg_data_tail = usermailbox[userID]->msg_data_tail->next;
                                }
                            }
                            else
                            {
                                printk("ker:more than 3\n");
                                umsg = "Fail";
                            }

                        }

                    }

                }
                else
                {
                    /* msg > 255 */
                    umsg = "Fail";
                }

            }
            else if(strcmp(instr, recv) == 0)
            {
                printk("ker: recv success~~~~[%s]\n",msg);
                umsg = "recv success~~";

                /*get uId to recv from target mailbox */
                uID = strsep(&msg, "\0");
                userID = simple_strtol(uID,NULL,0);
                printk("ker: \n recvuID = %d\n",userID);
                printk("ker: \n imsg = %s\n",msg);



                if(usermailbox[userID] == NULL)
                {
                    /* this mailbox does not exist*/
                    umsg = "Fail";
                }
                else
                {
                    printk("ker:recv knew type = [%c]\n",usermailbox[userID]->type);
                    if(usermailbox[userID]->type == '0')
                    {
                        printk("ker:recv knew head = [%s]\n",usermailbox[userID]->msg_data_head->buf);
                        //unquened
                        if(usermailbox[userID]->msg_data_head->buf != NULL)
                        {
                            /* Read data from mailbox*/
                            umsg  = usermailbox[userID]->msg_data_head->buf;
                        }
                        else
                        {
                            /* no msg in mailbox*/
                            umsg = "Fail";
                        }
                    }
                    else
                    {
                        //quened
                        if(usermailbox[userID]->msg_data_count > 0 )
                        {
                            umsg = usermailbox[userID]->msg_data_head->buf;
                            usermailbox[userID]->msg_data_head = usermailbox[userID]->msg_data_head->next;
                            usermailbox[userID]->msg_data_count --;
                        }
                        else
                        {
                            /* No msg in mailbox Q*/
                            umsg = "Fail";

                        }
                    }
                }
            }
            else
            {
                umsg = "nothing match";
            }
            if(umsg == NULL)
            {
                umsg = "Success";
            }

            kernel_nl_send_msg(umsg, strlen(umsg),pid);



        }
    }
}

struct netlink_kernel_cfg cfg =
{
    .input = kernel_nl_recv_msg,
};

static int __init kernel_init(void)
{

    printk("Entering: %s\n",__FUNCTION__);
    //This is for 3.6 kernels and above.

    nl_sk = (struct sock *)netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if(nl_sk==NULL)
    {

        printk(KERN_ALERT "ler:Error creating socket.\n");
        return -1;

    }

    return 0;
}

static void __exit kernel_exit(void)
{
    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(kernel_init);
module_exit(kernel_exit);


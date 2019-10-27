#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "com_app.h"

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;

int main(int argc, char *argv[])
{
    int id;
    char umsg[50];


    /* get id */
    id = atoi(argv[1]);
    if(id > 1000 || id < 1)
    {
        printf("id should be 1-1000\n");
        return -1;
    }



    /* set registration string */
    strcpy(umsg, "Registration. id=");
    strncat(umsg, argv[1], 3);
    strncat(umsg, ", type=", 7);
    strncat(umsg, argv[2], 8);

    printf("umsg = %s\n", umsg);




    sock_fd=socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if(sock_fd<0)
    {
        perror("socket connect failure\n");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));

    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    strcpy(NLMSG_DATA(nlh), umsg);
    printf("Sending message to kernel: %s\n", (char *)NLMSG_DATA(nlh));
    sendmsg(sock_fd,&msg,0);
    printf("Waiting for message from kernel\n");

    /* clear nlh */
    memset(nlh,0, NLMSG_SPACE(MAX_PAYLOAD));

    /* Read message from kernel */
    recvmsg(sock_fd, &msg, 0);
    printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));

    if(strcmp((char *)NLMSG_DATA(nlh),"Fail") == 0)
    {
        printf("Fail reg\n");
        return -1;
    }




    /* while */
    int j;
    for(j = 0; j < 4 ; j++)
    {
        memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
        nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
        nlh->nlmsg_pid = getpid();
        nlh->nlmsg_flags = 0;

        iov.iov_base = (void *)nlh;
        iov.iov_len = nlh->nlmsg_len;
        msg.msg_name = (void *)&dest_addr;
        msg.msg_namelen = sizeof(dest_addr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;



        char str[500];
        if(fgets(str, sizeof(str), stdin) != NULL)
        {
            /* get user input at most 270*/
            if(str[strlen(str)-1] == '\n')
                str[strlen(str)-1] = '\0';
            else
            {
                char toomuchmsg[20];
                memset(&toomuchmsg,0, sizeof(toomuchmsg));
                while(toomuchmsg[strlen(toomuchmsg)-1] != '\n')
                {
                    fgets(toomuchmsg,sizeof(toomuchmsg), stdin);
                }
            }

            /* classfiied "Send" & "Recv"*/

            if(strcmp(str, "Recv") == 0)
            {
                strcat(str, " " );
                strcat(str, argv[1]);
            }

            strcpy(NLMSG_DATA(nlh), str);



        }
        else
        {
            strcpy(NLMSG_DATA(nlh), "no input");

        }

        printf("%ld  fgets = [%s]\n",sizeof(str),str);


        printf("Sending message to kernel: %s\n", (char *)NLMSG_DATA(nlh));
        sendmsg(sock_fd,&msg,0);
        printf("Waiting for message from kernel\n");

        /* clear nlh */
        memset(nlh,0, NLMSG_SPACE(MAX_PAYLOAD));

        printf("---------------befor Received message payload: %s\n", (char *)NLMSG_DATA(nlh));
        /* Read message from kernel */
        recvmsg(sock_fd, &msg, 0);
        printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));
        printf("Received message payload: %ld\n", nlh->NLMSG_LEN);




    }
    close(sock_fd);
}

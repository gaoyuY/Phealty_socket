/*************************************************************************
	> File Name: master_socket.c
	> Author: GaoYu
	> Mail: 
	> Created Time: 2018年12月16日 星期日 09时56分08秒
 ************************************************************************/
#include "common.h"

typedef struct Node {
    struct sockaddr_in client_addr;
    struct Node *next;
} Node;

typedef struct LinkedList {
    Node head;
    int length;
    int num; //进程号
} LinkedList;

/************************建立监听，等待client端主动连接********/
int socketfd_listen(char *port) {
    int serverfd;
    struct sockaddr_in addr_server;
    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed!\n");
        close(serverfd);
        return -1;
    }
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(atoi(port));
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(serverfd, (struct sockaddr* )&addr_server, sizeof(struct sockaddr)) < 0) {
        perror("bind");
        close(serverfd);
        return -1;
    }

    if (listen(serverfd, 20) < 0) {
        perror("listen!");
        close(serverfd);
        return -1;
    }
    return serverfd;
}

/*******主动建立与client的连接，连接失败就从链表中删除该结点************/
int socket_create(Node *p, char *port) {
    int sockfd;
    struct sockaddr_in dest_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        close (sockfd);
        perror("Socket Failed!\n");
        return -1;
    }
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(atoi(port)); 
    dest_addr.sin_addr.s_addr = p->client_addr.sin_addr.s_addr;
    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        close(sockfd);
        return -1;
    }
    return sockfd;
}

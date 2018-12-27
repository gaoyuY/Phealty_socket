/*************************************************************************
	> File Name: client_socket.c
	> Author: GaoYu
	> Mail: 
	> Created Time: 2018年12月12日 星期三 20时12分33秒
 ************************************************************************/

#include "common.h"

/*************************连接***************************/

int socket_connect(char *host1, char *Port) {
    int sockfd;
    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(atoi(Port));//本地转换为网络字节序
    dest_addr.sin_addr.s_addr = inet_addr(host1);
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket Failed!\n");
        close(sockfd);
        return -1;
    } 
    if (connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        perror ("connect Failed!\n"); 
        close(sockfd);
        return -1;
    }
    return sockfd;
}

/*****************************监听**********************/

int socketfd_listen(char* Port) {
    int serverfd;
    struct sockaddr_in addr_server;
    if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket failed!\n");
        close(serverfd);
        return -1;
    }
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(atoi(Port));
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




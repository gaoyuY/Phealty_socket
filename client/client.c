/*************************************************************************
> File Name: client.c
> Author: GaoYu
> Mail: 
> Created Time: 2018年11月17日 星期六 22时02分43秒
************************************************************************/

#include "client_file.c"

int main () {
    init();
    int para[INS + 1];
    pthread_t t[INS  + 1];
    for (int i = 1; i < INS; i++) {
        mypara[i].retcode = i;
        if (pthread_create (&t[i], NULL, (void *)func, (void *)&mypara[i]) == -1) {
            perror ("pthread create failed!");
            exit(0);
        }
    }

    if (pthread_create(&t[INS], NULL, (void *)connect_heart, NULL) == -1) {
        perror ("connect pthread create failed!\n");
        exit(0);
    }

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    char *port1 = get_conf ("./Client.conf", "port1");
    char *port2 = get_conf ("./Client.conf", "port2");
    int sock_long_listen, sock_short_listen;
    
    sock_long_listen = socketfd_listen(port1);  //监听
    sock_short_listen = socketfd_listen(port2);  //短监听
    free(port1);
    free(port2);
    while (1) {  
        int client_long_accept;
        if ((client_long_accept = accept(sock_long_listen, (struct sockaddr *)&client_addr, &len)) < 0) {
            perror("accept failed!");
            close(client_long_accept);
            continue;
        }
        send_file (client_long_accept, sock_short_listen);    
        close (client_long_accept);
    }
    close(sock_short_listen);
    close(sock_long_listen);
    return 0;
}

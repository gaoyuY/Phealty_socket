/*************************************************************************
> File Name: lian_master.c
> Author: GaoYu
> Mail: 
> Created Time: 2018年12月05日 星期三 12时13分15秒
************************************************************************/

#include "master_linked_operation.c"

int main () {
    int pid;
    pid = fork();
    if (pid == 0) { 
        init();    //初始化队列
        pthread_t t[INS + 2];
        for (int i = 0; i < INS; i++) {
            linkedlist[i].num = i;
            pthread_mutex_init(&mutex[i], NULL);     //初始化互斥锁
            if (pthread_create(&t[i], NULL, func, (void *)&linkedlist[i]) == -1) {  //创建线程
                perror("pthread create ERROR !");
                exit(1);
            }
        }
        char *port3 = get_conf ("/etc/pihealth_master.conf", "port3");
        int warning_listen = socketfd_listen (port3); 
        if (pthread_create (&t[INS], NULL, (void *)func1, (void *)&warning_listen) == -1) {
            perror("warning pthread creat failed!");
            exit(1);
        }
        accept_unrepeat_insert(); 
        //执行不到
        for (int i = 1; i <= INS; i++) {
            pthread_join(t[i], NULL);
        }
    } 
     if (pid > 0){
        char str[100];
        FILE *fp = fopen("/etc/gaoyu_pid", "w");
        fprintf (fp, "%d", pid);
        fclose(fp);
        system(str);
        exit(1);
    }
    return 0;
}



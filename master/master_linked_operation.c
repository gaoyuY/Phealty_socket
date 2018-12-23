/*************************************************************************
	> File Name: master_linked_operation.c
	> Author: GaoYu
	> Mail: 
	> Created Time: 2018年12月16日 星期日 10时21分55秒
 ************************************************************************/

#include "master_file.c"

#ifdef _DEBUG
#define DBG(fmt , args...)  printf(fmt, ##args) 
#else
#define DBG(fmt , args...)  
#endif

LinkedList linkedlist[INS + 1];
pthread_mutex_t mutex[INS + 1]; //互斥锁数组，一个线程对应一个互斥锁


/***********初始化链表（链表用来存放，所有的client端结点的信息）******************/
LinkedList *init_linkedlist() {
    LinkedList *l = (LinkedList *)malloc(sizeof(LinkedList));
    l->head.next = NULL;
    l->length = 0;
    return l;
}

/***********初始化结点（结点用来存放client端信息**********/
Node *init_node() {
    Node *node = (Node *)malloc(sizeof(Node));
    node->next = NULL;
    return node;
}

/****************将主动连接过来的client端的结点信息，插入到链表中***********/
void insert (LinkedList *l, Node *node, int index) {
    Node *p = &(l->head);
    while (index--) {
        p = p->next;
        if (p == NULL) return;
    }
    node->next = p->next;
    p->next = node;
    l->length++;
    return ;
}

/*****************输出*********************/
void output(LinkedList *l) {
    Node *p = l->head.next;
    while (p) {
        DBG ("%s:%d\n", inet_ntoa(p->client_addr.sin_addr), ntohs(p->client_addr.sin_port));
        p = p->next;
    }
    DBG ("\n");
}

/***************每次找到最短的链表进行插入**************/
int find_min(LinkedList *l, int n) {
    int min = 0x3ffffff, num = 0;
    for (int i = 0; i < n; i++) {
        if (l[i].length < min) {
            min = l[i].length;
            num = i;
        }
    }
    return num;
}

/*********************判断是否重复，不重复就插入链表****************/
int unrepeat(LinkedList *l,Node *node) {
    for (int i = 0; i < INS; i++) {
        Node *p = l[i].head.next;        
        while (p) {
            if (p->client_addr.sin_addr.s_addr == node->client_addr.sin_addr.s_addr) {
                return 0;
            }
            p = p->next;
        }     
    }
    return 1;
}

/****************************删除结点****************************/
void delete_node (LinkedList *l) {
    Node *p = &(l->head);
    Node *q = p->next;
    while (q) {
        int sockfd1;
        char *port1 = get_conf("/etc/pihealth_master.conf", "port1");
        
        if ((sockfd1 = socket_create(q, port1)) == -1) {
            DBG ("delete %s\n", inet_ntoa(q->client_addr.sin_addr));
            if (pthread_mutex_lock (&mutex[l->num]) == 0) {
                Node *x = q;
                p->next = q->next;
                free(x);
                q = p->next;
                l->length--;
                pthread_mutex_unlock (&mutex[l->num]); 
            }
        }
        else {
            //调用接收文件函数
            recv_file(q, sockfd1); 
            p = p->next;
            q = q->next;
        }
        free(port1);
    }
    return ;
}

/********************初始化队列***************/
void init () {
    char *ip = get_conf("/etc/pihealth_master.conf", "prename"); 
    char *start1 = get_conf("/etc/pihealth_master.conf", "start");
    char *finish1 = get_conf("/etc/pihealth_master.conf", "finish");
    char *port = get_conf("/etc/pihealth_master.conf", "port");
    int sub;
    int start = atoi(start1);
    int finish = atoi(finish1);
    for (int i = start; i <= finish; i++) {
        char buffer[30];
        sprintf(buffer,"%s.%d", ip, i);
        Node *p = init_node();
        p->client_addr.sin_addr.s_addr = inet_addr(buffer);
        p->client_addr.sin_port = htons(atoi(port));
        sub = find_min(linkedlist, INS);
        insert (&linkedlist[sub], p, linkedlist[sub].length); 
    }
    for (int i = 0; i < INS; ++i) {
        DBG ("init : ------>%d.log<-----------%d\n", i, linkedlist[i].length);
        output(&linkedlist[i]);
    }
}

/*************************警告信息线程**************************/

void *func1 (int *argv) {
    int warning_listen = *argv; 
    while (1) {
        int waring_sockfd_long; 
        struct sockaddr_in addr_client;
        socklen_t len= sizeof(addr_client);
        if ((waring_sockfd_long = accept (warning_listen, (struct sockaddr *)&addr_client, &len)) < 0) {
            perror ("warning accept failed!");
            close(waring_sockfd_long);
            continue;
        }
        char IP[50];
        strcpy(IP, inet_ntoa(addr_client.sin_addr));
        int retcode;
        if (recv(waring_sockfd_long, &retcode, 4, 0) < 0) {
            perror("recv warning failed!");
            continue;
        }
        
        FILE *fp = warning_mk_file (IP, retcode);         
        send (waring_sockfd_long, &retcode, 4, 0); 
        char buffer2[MAX_SIZE];
        int b;
        if ((b =recv (waring_sockfd_long, buffer2, MAX_SIZE, 0)) > 0) {
            buffer2[b] = '\0';
            DBG ("waring data %d 字节 %s\n", b, buffer2);
            if (fp == NULL) {
                DBG ("NULL\n");
            }
            fprintf (fp, "%s", buffer2);
            fflush (stdout);
            memset(buffer2, 0, sizeof(buffer2));
        }
        fclose(fp);
        close (waring_sockfd_long);
    }
}

/***********等待client端连接,并判重插入***********************/
void accept_unrepeat_insert() {
    int serverfd;
    char *port = get_conf("/etc/pihealth_master.conf", "port");
    serverfd = socketfd_listen(port);    //监听
    free (port);
    while (1) {
        Node *p = init_node();
        int client_fd;
        struct sockaddr_in addr_client;
        socklen_t len= sizeof(addr_client);
        if ((client_fd = accept (serverfd, (struct sockaddr *)&p->client_addr, &len)) < 0) {
            perror("accept ERROR");
            close(client_fd);
            continue;
        }
        close (client_fd);
        if (unrepeat(linkedlist, p) == 0) { //去重，链表中有就不插入
            DBG ("repeat!\n");
            continue;
        }
        int sub = find_min(linkedlist, INS); // 找到最短的链表进行插入
        if (pthread_mutex_lock(&mutex[sub]) == 0) { //加互斥锁
            insert (&linkedlist[sub], p, linkedlist[sub].length);    //插入到链表中
            for (int i = 0; i < INS; ++i) {
                DBG ("insert end : ----->%d.log<------------%d\n", i, linkedlist[i].length);
                output(&linkedlist[i]);
            }
            pthread_mutex_unlock(&mutex[sub]);//解开互斥锁
        }

    }
    close(serverfd);
    return ;
}

/***************************链表操作线程***************************/

void *func(void *argv) {
    LinkedList *l = (LinkedList *)argv;
    while (1){
        if (linkedlist[l->num].length == 0) { 
            sleep(2); continue;
        }
        delete_node(&linkedlist[l->num]); 
        DBG ("delete : ------>%d.log<--------%d\n", l->num, linkedlist[l->num].length);
        output(&linkedlist[l->num]);
        sleep(2);   
    }
    pthread_exit(NULL);
    return NULL;
}

/*************************************************************************
	> File Name: master_file.c
	> Author: GaoYu
	> Mail: 
	> Created Time: 2018年11月25日 星期日 16时34分59秒
 ************************************************************************/
#include "master_socket.c"
#define INS 5
#define file_num 6 


#ifdef _DEBUG
#define DBG(fmt , args...)  printf(fmt, ##args) 
#else
#define DBG(fmt , args...)  
#endif

/***********读取配置文件********************/

char * get_conf(const char *pathname, const char *key_name){
    char *line;
    size_t len = 0;
    ssize_t read;
    char *value = (char *)calloc(sizeof(char), 100);
    FILE *fp = NULL;
    fp = fopen(pathname, "r");
    if (fp == NULL) {
        perror("fopen:");
        return NULL;
    }

    while ((read = getline(&line,&len,fp)) > 0) {
        char *ptr = strstr(line,key_name);
        if (ptr == NULL) continue;
        ptr += strlen(key_name);
        if (*ptr != '=') continue;
        strncpy(value, (ptr+1), strlen(ptr+2));//strlen(per+2) 少获取一个长度，代表换行
        int tempvalue = strlen(value);
        value[tempvalue] = '\0';
    }
    fclose(fp);
    return value;
}


/****************************建立对应文件夹和文件*********************/

FILE *mk_file(FILE *fp, char *IP, int retcode) {
    char IP1[50] = "./";
    strcat(IP1, IP);
    if (access(IP1, 0) == -1) {
        mkdir(IP1, 0777);
    }
    if (retcode == 1) {
        strcat(IP1,"/CPU.log");
        fp = fopen(IP1, "a+");
    }else if (retcode == 2) {
        strcat(IP1, "/disk.log");
        fp = fopen (IP1, "a+");
    } else if (retcode == 3) {
        strcat(IP1, "/mem.log");
        fp = fopen (IP1, "a+");
    } else if (retcode == 4) {
        strcat(IP1, "/OS.log");
        fp = fopen (IP1, "a+");
    } else if (retcode == 5) {
        strcat(IP1, "/user.log");
        fp = fopen (IP1, "a+");
    } else if (retcode == 6) {
        strcat (IP1, "/Malicious.log");
        fp = fopen (IP1, "a+");
    }
    return fp;
}

FILE *warning_mk_file(char *IP, int retcode) {
    char IP1[50] = "./";
    FILE *fp;
    strcat(IP1, IP);
    strcat(IP1, "warning");
    if (access(IP1, 0) == -1) {
        mkdir(IP1, 0777);
    }
    if (retcode == 1) {
        strcat(IP1,"/cpuWarning.log");
        fp = fopen(IP1, "a+");
    } else if (retcode == 4) {
        strcat(IP1, "/osWarning.log");
        fp = fopen (IP1, "a+");
    }  else if (retcode == 6) {
        strcat (IP1, "/MaliciousWarning.log");
        fp = fopen (IP1, "a+");
    }
    return fp;
}


/***************************接收文件函数*********************/
void recv_file (Node *q, int sockfd1) {
    char *port2 = get_conf("/etc/pihealth_master.conf", "port2");
    char IP[50];
    strcpy(IP, inet_ntoa(q->client_addr.sin_addr));
    for (int i = 1; i<= file_num; i++) {
        int retcode;
        if ((recv (sockfd1, &retcode, 4, 0)) <= 0){
            perror ("recv failed!\n");
            break;
        }
        DBG ("recv %d end\n", retcode);
        FILE *fp = NULL;
        fp = mk_file(fp, IP, retcode);
        DBG ("IP = %s\n",IP);
        if ((send (sockfd1, &retcode, 4, 0)) <= 0) {
            perror ("send failed!\n");
            break;
        }
        DBG ("send %d end\n", retcode);
        int sock_fd1 = socket_create (q, port2);
        if (sock_fd1 == 0) break;
        int b;
        char buffer1[MAX_SIZE];
        while ((b = recv (sock_fd1, buffer1, MAX_SIZE, 0)) > 0) {
            buffer1[b] = '\0';
            DBG ("recv %d 字节 %s\n", b, buffer1);
            if (fp == NULL) {
                DBG ("NULL\n");
            }
            fprintf (fp, "%s", buffer1);
            fflush (stdout);
            memset(buffer1, 0, sizeof(buffer1));
        }
        close (sock_fd1);
        fclose(fp);
    }
    close(sockfd1);
    return ;
}

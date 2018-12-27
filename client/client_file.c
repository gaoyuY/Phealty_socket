/*************************************************************************
> File Name: client_operation.c
> Author: GaoYu
> Mail: 
> Created Time: 2018年12月02日 星期日 20时33分27秒
************************************************************************/

#include "client_socket.c"
char Buffer[MAX_SIZE];
char buffer1[MAX_SIZE];//警告所用


struct Command {
    int retcode;
    char sleep[10];
    char command[100];
    char Logfile[100];
};
struct Command mypara[10];

pthread_mutex_t mutex[INS];
/*************************读取配置信息*****************/
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
    if (value == NULL) {
        printf ("%s no exist %s\n", pathname, key_name);
    }
    return value;
}


/*****************初始化配置信息*****************************/
void init() {
    char command[] ="command";
    char sleep[]="sleep";
    char Logfile[]="Logfile";
    char str1[20], str2[20], str3[20]; 
    for (int i = 1; i < INS; i++) { 
        sprintf (str1, "%s%d", command, i);
        sprintf (str2, "%s%d", sleep, i);
        sprintf (str3, "%s%d", Logfile, i);
        char *command = get_conf("./Client.conf", str1);
        char *sleep = get_conf("./Client.conf", str2);
        char *Logfile = get_conf("./Client.conf", str3);
        strcpy(mypara[i].command, command);
        strcpy(mypara[i].sleep, sleep);
        strcpy(mypara[i].Logfile, Logfile);
        free(command); free(sleep); free (Logfile);
        memset(str1, strlen(str1), 0);
        memset(str2, strlen(str2), 0);
        memset(str3, strlen(str3), 0);
    }
    for (int i = 1; i < INS; i++) {
        pthread_mutex_init (&mutex[i], NULL);
    }
    
}


/************************创建相应的脚本日志文件*******************/
void mk_file (int retcode) {
    FILE *fp;
    char IP[20] = "./Logfile";
    if (retcode == 1) {
        strcat(IP,"/CPU.log");
        fp = fopen(IP, "a+");
    }else if (retcode == 2) {
        strcat(IP, "/disk.log");
        fp = fopen (IP, "a+");
    } else if (retcode == 3) {
        strcat(IP, "/mem.log");
        fp = fopen (IP, "a+");
    } else if (retcode == 4) {
        strcat (IP, "/OS.log");
        fp = fopen (IP, "a+");
    } else if (retcode == 5) {
        strcat (IP, "/user.log");
        fp = fopen (IP, "a+");
    } else if (retcode == 6) {
        strcat (IP, "/Malicious.log");
        fp = fopen (IP, "a+");
    }
    fclose(fp);
}


/**************判断是否为警报信息，并发送警告信息*******************/

void warning_data(struct Command *para) {
    char buffer1[MAX_SIZE];
    FILE *fp1;
    FILE *fp2;
    fp1 = popen (para->command, "r");
    fp2 = fopen(para->Logfile, "a");
    int len = fread(buffer1, sizeof(char), MAX_SIZE, fp1);
    fprintf (fp2, "%s", buffer1);
    fclose(fp1);
    fclose(fp2);
    if ((para->retcode == 6) || (strstr(buffer1, "warning") != NULL)) {
        if (strlen(buffer1) == 0) return;
        char *host = get_conf ("./Client.conf", "host");
        char *port3 = get_conf ("./Client.conf", "port3");
        int socket_fd = socket_connect(host, port3);
        free(host); free(port3);
        if (send (socket_fd, &para->retcode, 4, 0) <= 0) {
            perror("send retcode failed!");
            return ;
        }
        int flag;
        if(recv(socket_fd, &flag, 4, 0) <= 0){
            perror("recv retcode failed!\n");
            return;
        }
        printf ("send warning data %d start!\n", para->retcode);
        send (socket_fd, buffer1, len, 0);
        printf ("send warning data %d end!\n", para->retcode);
        memset(buffer1, 0, sizeof(buffer1));
        fflush(stdout);
        close(socket_fd);            
    }
    
    return ;
}

/*******************运行脚本，并写入相应的日志文件**********************/

void get_file(struct Command *para) { 
    warning_data(para);
    return;
}

/*void warning_data(struct Command *para) {
    FILE *fp1;
    FILE *fp2;
    if (para->retcode == 1) {
        fp1 = popen (para->command, "r");
        fp2 = fopen(para->Logfile, "a");
    } else if (para->retcode == 4) {
        fp1 = popen (para->command, "r");
        fp2 = fopen(para->Logfile, "a");
    } else {
        fp1 = popen (para->command, "r");
        fp2 = fopen(para->Logfile, "a");
    }
    char buffer1[MAX_SIZE];
    int len = fread(buffer1, sizeof(char), MAX_SIZE, fp1);
    fprintf (fp2, "%s", buffer1);
    fclose(fp1);
    fclose(fp2);
    if ((para->retcode == 6 && fp1 != NULL) || (strstr(buffer1, "warning") != NULL)) {
        char *host = get_conf ("./Client.conf", "host");
        char *port3 = get_conf ("./Client.conf", "port3");
        int socket_fd = socket_connect(host, port3);
        //free(host);free(port3);
        if (send (socket_fd, &para->retcode, 4, 0) < 0) {
            perror("send retcode failed!");
            return ;
        }
        int flag;
        if(recv(socket_fd, &flag, 4, 0) < 0){
            perror("recv retcode failed!\n");
            return;
        }
        printf ("send warning data start!\n");
        if (send (socket_fd, buffer1, len, 0) <= 0) {
            perror("send file failed !\n");
            return;
        }
        printf ("send warning data end!\n");
        memset(buffer1, 0, sizeof(buffer1));
        fflush(stdout);
        close(socket_fd);            
    }
    
    return ;
}
*/

/*******************运行脚本，并写入相应的日志文件**********************/

/*void get_file(struct Command *para) { 
    char IP[20] = "./";
    if (para->retcode == 1) {
        warning_data(para);
    } else if (para->retcode == 2) {
        system(para->command);
    } else if (para->retcode == 3) {
        system(para->command);
    } else if (para->retcode == 4) { 
        warning_data(para);
    } else if (para->retcode == 5) {
        system (para->command);
    } else if (para->retcode == 6) {
        warning_data(para);
    } 
    return;
}
*/
/*******************根据标识码找相应的日志文件，并删除文件**************/

FILE* find_file (int retcode) {
    FILE *fp;
    fp = fopen(mypara[retcode].Logfile, "r");
    return fp;
}

/********************线程函数**创建文件****运行脚本*******************/

void *func (void *argv) {
    struct Command *para = (struct Command *)argv;
    while (1) { 
        mk_file (para->retcode);//创建文件
        pthread_mutex_lock(&mutex[para->retcode]);
        get_file (para);//运行脚本
        pthread_mutex_unlock(&mutex[para->retcode]);
        sleep (atoi(para->sleep));
    } 
    pthread_exit(NULL);
    return NULL;
}

/*********************心跳连接**************************************/

void *connect_heart (void *argv) {
    char *host_1 = get_conf ("./Client.conf", "host");
    char *port_1 = get_conf ("./Client.conf", "port");
    char host[20], port[10];
    strcpy(host, host_1);
    strcpy(port, port_1);
    free(host_1);  free(port_1);
    while (1) {
        int socket_fd = socket_connect(host, port);
        if (socket_fd > 0) {
            printf ("connect success!!!!!!\n");
            close(socket_fd);
        }
        sleep(15);
    }
}

/**************************发送文件*********************************/

void send_file(int client_long_accept, int sock_short_listen) {
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    for (int i = 1; i < INS; i++) {
        if (send (client_long_accept, &i, 4, 0) <= 0) {
            perror ("send1 failed!\n");
            break;
        }
        printf ("send %d end1\n", i);
        int flag; 
        if (recv(client_long_accept, &flag, 4, 0) <= 0) {
            perror ("recv failed!\n");
            break;
        }
        printf ("%d recv end !\n", i);
            int client_short;
        if ((client_short = accept (sock_short_listen, (struct sockaddr *)&client_addr, &len)) < 0) {
            perror("accept1 failed!");
            close(client_short);
            continue;
        }
        pthread_mutex_lock (&mutex[i]);
        FILE *fp1 = find_file (i);
        if (fp1 == NULL) {
            pthread_mutex_unlock (&mutex[i]);
            continue;
        }
        int len_fp;
        while((len_fp = fread (Buffer, sizeof(char), MAX_SIZE, fp1)) > 0) {
            if ((send (client_short, Buffer, len_fp, 0)) > 0){
                memset(Buffer, 0, sizeof(Buffer));
            }
        }
        
        printf ("recv data end %d\n", i);
        fclose (fp1);
        char str[100];
        sprintf (str, "> %s",mypara[i].Logfile);
        system (str);
        pthread_mutex_unlock (&mutex[i]);
        close(client_short);
        sleep(2);
    }
}

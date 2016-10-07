#ifndef  SHTTPD_H
#define SHTTPD_H
#ifdef __cplusplus
extern "C"{
#endif
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <semaphore.h>
#include <string.h>
#include <assert.h>
//#include <pthread.h>
#include "pool.h"
//#include "timer.h"
#define SERVER_STRING "Server: shaitool first sever httpd/0.1.0\r\n"
//debug flag
#define SERVER_STRING "Server: shaitool first sever httpd/0.1.0\r\n"
#define DEBUG 0
//the sleep time for distribute and manage
#define SLEEP_TIME 2

//SUCCESS OR FAILURE
#define SUCCESS 1
#define FAILURE -1

/*the events type*/
#define  DATA_IN 0
#define DATA_OUT 1
//config sturct
struct server_conf{
    char CGIRoot[128];
    char DefaultFile[128];
    char DocumentRoot[128];
    char ConfigFile[128];
    int ListenPort;
    int MaxClient;   
    int TimeOut;
    int InitWorkerNum;
    int MaxWorkerNum;
};
//declare sever config structure
extern struct server_conf server_para;
void *accept_request(void *);
//desplay the config
int Displayconf();

//get the para from conf file
int Getpara_file(char * file);

//get the para from cmd
int Getpara_cmd(int argc,char *argv[]);

//init the phread pool
int Gthread_pool_init(struct Gthread_pool * pool, int max_tasks, int max_workers, int min_workers);

//close the phread pool
int close_pool(struct Gthread_pool * pool);

//add a new task to the task list
int add_job(struct Gthread_pool * pool, void * (* job)(void * arg), void * arg);
//add event to the epoll queue
void add_event(int epoll_fd,int fd,int event_type);
//delet event to the epoll queue
void del_event(int epoll_fd,int fd,int event_type);
#ifdef __cplusplus
}
#endif
#endif

















#include <netinet/in.h>
#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include "server.h"
#include <unistd.h>
#include <sys/epoll.h>
#include "timer.h"
#include <assert.h>
#include <signal.h>
#include "Glist.h" 
#include "config.h"
#define METHOD_LEN 255
#define PATH_LEN 255
#define URL_LEN 255
#define VERSION_LEN 50
#define MAXEVENTS 255
static int BACKLOG = 100000;
static int pipefd[2];
static server_conf server_config;
static struct Gthread_pool pool;
static int get_line(int sock, char *buf, int size);
static void * client_service(void * arg);
static void doGetMethod(int client_fd, char * url, char * version);
static void unimplemented(int client);
static void not_found(int client);
static void headers(int client);
static void cat(int client, FILE *resource);
static void file_serve(int client_fd, char * filename);
static void cannot_execute(int client);
static void execute_cgi(int client, const char *path, const char *method, const char *query_string);
static int setNoBlock(int fd);
static void sig_alarm_handle(int sig);
void sig_int_handle(int sig);
void close_client(int client_fd);
static list_head timer_list;

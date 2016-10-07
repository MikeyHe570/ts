#ifndef HTTP_POOL
#define HTTP_POOL
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>
#include "config.h"
#define ISspace(x) isspace((int)(x))

#define SERVER_STRING "Server: jdbhttpd/0.1.0 by shaitool\r\n"
#define STDIN   0
#define STDOUT  1
#define STDERR  2

#define METHOD_LEN 255
#define PATH_LEN 255
#define URL_LEN 255
#define VERSION_LEN 50
#define MAXEVENTS 255
//extern void accept_request(void *);
void bad_request(int);
void doGetMethod(int client_fd, char * url, char * version);
void cat(int, FILE *);
void cannot_execute(int);
void error_die(const char *);
void execute_cgi(int, const char *, const char *, const char *);
int get_line(int, char *, int);
void headers(int, const char *);
void not_found(int);
void serve_file(int, const char *);
int startup(u_short *);
void unimplemented(int);
void close_client(int client_fd);
#endif

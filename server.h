#ifndef _SERVER
#define _SERVER
#include "config.h"
//#define SERVER_STRING 

bool server_init(struct Gthread_pool * pool);
//bool server_close(struct Gthread_pool * pool);
void request_handle(struct Gthread_pool * pool);

#endif

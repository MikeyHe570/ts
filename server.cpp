#include "server.h"
bool server_init(struct Gthread_pool *pool){
    printf("server.h\n");
    Gthread_pool_init(pool,server_para.MaxClient, server_para.MaxWorkerNum, server_para.InitWorkerNum);
    void request_handle(struct Gthread_pool *pool);
return true;
}

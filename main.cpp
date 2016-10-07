#include "server.h"
//#include "config.h"

int main(int argc,char *argv[])
{

    Getpara_cmd(argc,argv);
    Getpara_file(server_para.ConfigFile);
    Displayconf();
    Gthread_pool pool;
    server_init(&pool);    
    request_handle(&pool);

    return 0;
}

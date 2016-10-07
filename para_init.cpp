#include "config.h"

// config struct data for server
struct server_conf server_para={
                                "doc/cgi-progm",
                                "index.html",
                                "doc",
                                "doc/config.conf",
                                55555,
                                10000,
                                5,
                                10,
                                2048,
};


//the short cmd string
static char * short_cmd ="c:d:f:o:l:m:t:i:w:h";

static int ConfGetLine(int fd,char *buffer,int len);
//the long cmd struct
static struct option long_cmd []={
                                 {"CGIRoot",        required_argument, NULL, 'c'},
                                 {"DefaultFile",    required_argument, NULL, 'd'},
                                 {"DocumentRoot",   required_argument, NULL, 'o'},
                                 {"ConfigFile",     required_argument, NULL, 'f'},
                                 {"ListenPort",     required_argument, NULL, 'l'},
                                 {"MaxClient",      required_argument, NULL, 'm'},
                                 {"TimeOut",        required_argument, NULL, 't'},
                                 {"InitWorkerNum",  required_argument, NULL, 'i'},
                                 {"MaxWoerkerNum",  required_argument, NULL, 'w'},  
                                 {"help",           no_argument, NULL, 'h'},
                                };

static int ConfGetLine(int fd,char *buffer,int len){
    int n=0, i=0, begin=0;
    memset(buffer,0,len);
    for(;i<len;begin ? ++i : i){
        n=read(fd,buffer+i,1);
        if(0==n){
            *(buffer+i)='\0';
            break;
        }
        else if('\n'==*(buffer+i)){
            if(begin != 0){
                *(buffer+i) = '\0';
                break;
            }
            else
                continue;
        }
        else
            begin = 1;
    }
    return i;
}

int Getpara_file(char *file){
#define LINELEN BUFSIZ
    int fd =-1,n=0;
    char *name = NULL;
    char *value =NULL;
    char *pos =NULL;
    char line[LINELEN];

    if((fd =open(file,O_RDONLY))=-1){
#if 1==DEBUG
        perror("Open conffile error!");
#endif
        return -1;
    }
    while(n= ConfGetLine(fd,line,LINELEN)){
        pos = line;
        if(*pos == '#')
            continue;
        //get the name
        while(!isspace(*pos))
            pos++;
        name = pos;
        while(!isspace(*pos)&&*pos != '=')
            pos++;
        *pos++='\0';
        //get the value
        while(isspace(*pos)&&*pos =='=')
            pos++;
        value = pos;
        while(!isspace(*pos)&& *pos != '\0')
            pos++;
        while(!isspace(*pos)&&*pos !='\0')
            pos++;
        *pos='\0';
            
        //set the config by file
        if(strcmp(name,"CGIRoot")==0)
            strcpy(server_para.CGIRoot,value);
            
        if(strcmp(name,"DefauleFile")==0)
            strcpy(server_para.CGIRoot,value);
            
        if(strcmp(name,"DocumentRoot")==0)
            strcpy(server_para.CGIRoot,value);
            
        if(strcmp(name,"ConfigFile")==0)
            strcpy(server_para.CGIRoot,value);
            
        if(strcmp(name,"ListenPort")==0)
            server_para.ListenPort =atoi(value);
            
        if(strcmp(name,"MaxClient")==0)
            server_para.MaxClient =atoi(value);
            
        if(strcmp(name,"TimeOut")==0)
            server_para.TimeOut =atoi(value);
            
        if(strcmp(name,"InitWorkerNum")==0)
            server_para.InitWorkerNum =atoi(value);
            
        if(strcmp(name,"MaxWorkerNum")==0)
            server_para.MaxWorkerNum=atoi(value);
            
    }       
    return SUCCESS; 
}

//get para from cmd
int Getpara_cmd(int argc,char *argv[]){
    if(argc<2||argv==NULL)
        return SUCCESS;
    int c=-1, value =-1;
    while((c = getopt_long(argc,argv,short_cmd,long_cmd,NULL))!=-1){
        switch(c){
            case 'c': printf("set CGIRoot: %s\n",optarg);
                        strcpy(server_para.CGIRoot,optarg);
                        break;                              
                        
            case 'd': printf("set DefaultFile: %s\n",optarg);
                        strcpy(server_para.DefaultFile,optarg);
                        break;                              
                        
            case 'o': printf("set DocumentRoot: %s\n",optarg);
                        strcpy(server_para.DocumentRoot,optarg);
                        break;                              
                        
            case 'f': printf("set ConfigFile: %s\n",optarg);
                        strcpy(server_para.ConfigFile,optarg);
                        break;                              
                        
            case 'l': value= atoi(optarg);
                        printf("set ListenPort: %d\n",value);
                        server_para.ListenPort = value;
                        break;                              
                        
            case 't': value= atoi(optarg);
                        printf("set TimeOut: %d\n",value);
                        server_para.TimeOut = value;
                        break;                              
                        
            case 'i': value= atoi(optarg);
                        printf("set InitWorkerNum: %d\n",value);
                        server_para.InitWorkerNum = value;
                        break;                              
                        
            case 'm': value= atoi(optarg);
                        printf("set MaxClient: %d\n",value);
                        server_para.MaxClient = value;
                        break;                              
                        
            case 'w': value= atoi(optarg);
                        printf("set MaxWorkerNum: %d\n",value);
                        server_para.MaxWorkerNum = value;
                        break;                              
            case 'h': printf("no help by youself!hahaha!");
                        break;
            default :break; 
        
        }
    }
    return SUCCESS;
}

//desplay the config
int Displayconf(){
    printf("http sever CGIRoot: %s\n", server_para.CGIRoot);
    printf("http sever DefaultFile: %s\n", server_para.DefaultFile);
    printf("http sever DocumentRoot: %s\n", server_para.DocumentRoot);
    printf("http sever ConfigFile: %s\n", server_para.ConfigFile);
    printf("http sever ListenPort: %d\n", server_para.ListenPort);
    printf("http sever MaxClient: %d\n", server_para.MaxClient);
    printf("http sever TimeOut: %d\n", server_para.TimeOut);
    printf("http sever InitWorkerNum: %d\n", server_para.InitWorkerNum);
    printf("http sever MaxWorkerNum: %d\n", server_para.MaxWorkerNum);

    return SUCCESS;
}








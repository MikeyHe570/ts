#include "config.h"
#include "pool.h"
static struct timeval delay = {0, SLEEP_TIME}; 
static const float POOL_GATE = 0.2;
//static int Gthread_pool_init(struct Gthread_pool *pool,int max_tasks,int max_worker,int min_workers);

static void sig_usr1_handler(int signum);

static void *worker_routline(void *arg);

static int add_worker(struct Gthread_pool_worker *new_worker,struct Gthread_pool *pool);

static int del_worker_from_pool(struct Gthread_pool_worker *del_worker, struct Gthread_pool *pool);

float get_pool_now_using(struct Gthread_pool *pool);
    
static struct Gthread_pool_worker * search_idle_worker(Gthread_pool *pool);

static void *worker_manage(void *arg);

static void *distribute_task(void * arg);
    
//static int add_job(struct Gthread_pool *pool,void *(*job)(void *arg),void *arg);

static int add_task(struct Gthread_pool_task *task,struct Gthread_pool *pool,void*(*proccess)(void *arg),void *arg);

//Init the pthread pool: worker_manage and distrubute_task
int Gthread_pool_init(struct Gthread_pool *pool, int max_tasks, int max_workers, int min_workers){
    assert(pool);
    pool->max_tasks = max_tasks;
    pool->max_workers = max_workers;
    pool->min_workers = min_workers;
    pool->mutex_data.worker_num =pool->min_workers;
    pool->mutex_data.task_num=0;

    pthread_mutex_init(&(pool->info_lock),NULL);
#if DEBUG  == 1
    pthread_mutex_init(&(pool->IO_lock),NULL);
    printf("into pool init~\n");
#endif
    sem_init(&(pool->surplus_task_num),0,0);

    INIT_LIST_HEAD(&(pool->task_list));
    INIT_LIST_HEAD(&(pool->workers));

    for(int i=0;i < pool->min_workers;++i){   //init the min_worker
        struct Gthread_pool_worker *temp_worker = (struct Gthread_pool_worker *)malloc(sizeof(struct Gthread_pool_worker));
        pthread_mutex_lock(&(pool->info_lock));
        add_worker(temp_worker,pool);
        pthread_mutex_unlock(&(pool->info_lock));
    }

    pthread_create(&(pool->manage_worker),NULL,worker_manage,(void *)pool); //create the worker(phread) manage
    pthread_create(&(pool->task_distribute_worker),NULL,distribute_task,(void *)pool);//create the task manage

    return SUCCESS;
}
//the sigusr1 handler fuctong
void sig_usr1_handler(int signum)
{
        pthread_exit(NULL);
}
//worker(phread) routline 
void *worker_routline(void *arg){
    pthread_detach(pthread_self()); //set unjoinable
    struct Gthread_pool *pool =(*((struct Gthread_pool_worker_routline_args *)arg)).pool;
    struct Gthread_pool_worker *this_worker = (*((struct Gthread_pool_worker_routline_args *)arg)).this_worker;

    sigset_t block_set;
    sigemptyset(&block_set);
    sigaddset(&block_set,SIGUSR1);
    signal(SIGUSR1,sig_usr1_handler);
    printf("woker :%ld\n",pthread_self());
#if DEBUG ==1
    pthread_sigmask(SIG_BLOCK, &block_set, NULL);
    pthread_mutex_lock(&(pool->IO_lock));
    printf("Now we have in the worker and have %d worker\n", pool->mutex_data.worker_num);
    pthread_mutex_unlock(&(pool->IO_lock));
    pthread_sigmask(SIG_UNBLOCK, &block_set, NULL);
#endif  

    if(this_worker == NULL){
#if DEBUG == 1
    pthread_sigmask(SIG_BLOCK, &block_set, NULL);
    pthread_mutex_lock(&(pool->IO_lock));
    printf("a thread can not get his info by id, his id is: %ld\n",pthread_self());
    pthread_mutex_unlock(&(pool->IO_lock));
    pthread_sigmask(SIG_UNBLOCK, &block_set, NULL);
#endif
    exit(16);
    }
    
    while(1){
        pthread_mutex_lock(&(this_worker->worker_lock));//the worker is ready to accept the task so tell add_work by sigle      
        if(this_worker->state == BOOTING){
            pthread_mutex_lock(&(this_worker->boot_lock));
            this_worker->state = READY;
            pthread_cond_signal(&(this_worker->boot_cond));//tell the add_work by sigle
            pthread_mutex_unlock(&(this_worker->boot_lock));
        }
        pthread_cond_wait(&(this_worker->worker_cond),&(this_worker->worker_lock)); //wait the sigal to process task
        pthread_mutex_unlock(&(this_worker->worker_lock));
        
        if(pool->flag == SHUTDOWN){//pool is shutting
#if DEBUG == 1
        pthread_mutex_lock(&(pool->IO_lock));
        printf("the worker thread, id: %ld will eixt!\n",pthread_self()); 
        pthread_mutex_unlock(&(pool->IO_lock));
#endif
        
        pthread_mutex_lock(&(pool->info_lock));
        pthread_mutex_destroy(&(this_worker->boot_lock));
        pthread_mutex_destroy(&(this_worker->worker_lock));
        pthread_cond_destroy(&(this_worker->boot_cond));
        pthread_cond_destroy(&(this_worker->worker_cond));
        
        list_del(&(this_worker->link_node));
        free(this_worker);
        pool->mutex_data.worker_num--;
        pthread_mutex_unlock(&(pool->info_lock));
        pthread_exit(NULL);
        }
        pthread_sigmask(SIG_BLOCK,&block_set,NULL);
        (*(this_worker->worker_task))(this_worker->worker_task_arg);//process task;
        pthread_sigmask(SIG_UNBLOCK,&block_set,NULL);
        
        pthread_mutex_lock(&(this_worker->worker_lock));  // this work have done this task so he will be READY to other task
        this_worker->state = READY;

        pthread_mutex_unlock(&(this_worker->worker_lock));
#if DEBUG == 1
        pthread_sigmask(SIG_BLOCK, &block_set, NULL);
        pthread_mutex_lock(&(pool->IO_lock));
        printf("The worker %ld has finish a task!\n", pthread_self()); 
        pthread_mutex_unlock(&(pool->IO_lock));
        pthread_sigmask(SIG_UNBLOCK, &block_set, NULL);
#endif
    }
}

int add_worker(struct Gthread_pool_worker *new_worker,struct Gthread_pool *pool){
    int error;
    assert(new_worker);
    assert(pool);
    new_worker->state = BOOTING;
    new_worker->routline_args.pool = pool;
    new_worker->routline_args.this_worker = new_worker;
    pthread_mutex_init(&(new_worker->worker_lock), NULL);
    pthread_cond_init(&(new_worker->worker_cond), NULL); 
    pthread_mutex_init(&(new_worker->boot_lock), NULL);
    pthread_cond_init(&(new_worker->boot_cond), NULL); 

    pthread_mutex_lock(&(new_worker->boot_lock));
    if((error= pthread_create(&(new_worker->id),NULL,worker_routline,&(new_worker->routline_args)))!=0){
        printf("create worker phread error!");
        return FAILURE;
    }
    
    while(new_worker->state == BOOTING)
        pthread_cond_wait(&(new_worker->boot_cond),&(new_worker->boot_lock));
    pthread_mutex_unlock(&(new_worker->boot_lock));
  //  pthread_mutex_lock(&(pool->info_lock));
    list_add_tail(&(new_worker->link_node),&(pool->workers));
   // pthread_mutex_unlock(&(pool->info_lock));

   return SUCCESS;
}


//delete the worker(pthread) from the pool
int del_worker_from_pool(struct Gthread_pool_worker *del_worker, struct Gthread_pool *pool){
   // pthread_kill(del_worker->id,0);  //!!!!
    pthread_kill(del_worker->id,SIGUSR1);
    while(0 == pthread_kill(del_worker->id,0))
        select(0,NULL,NULL,NULL,&delay);
    pthread_mutex_destroy(&(del_worker->boot_lock));
    pthread_mutex_destroy(&(del_worker->worker_lock));
    pthread_cond_destroy(&(del_worker->boot_cond));
    pthread_cond_destroy(&(del_worker->worker_cond));
#if DEBUG ==1
    pthread_mutex_lock(&(pool->IO_lock));
    printf("The delete  worker:  %ld \n", del_worker->id); 
    pthread_mutex_unlock(&(pool->IO_lock));
#endif
    list_del(&(del_worker->link_node));
    return SUCCESS;
}

//get the using worker and delete some mutex worker if need
float get_pool_now_using(struct Gthread_pool *pool){
    int total, busy_num = 0;
    struct list_head *pos;
    struct Gthread_pool_worker *a_worker;
    pthread_mutex_lock(&pool->info_lock);
    total = pool->mutex_data.worker_num;

    if(total<=pool->min_workers){
        pthread_mutex_unlock(&(pool->info_lock));
        return (float)1;
    }
    
    list_for_each(pos,&(pool->workers)){
        a_worker = list_entry(pos,struct Gthread_pool_worker,link_node);
        pthread_mutex_lock(&(a_worker->worker_lock));
        if(a_worker->state == BUSY){
           printf("buy worker id %ld\n",a_worker->id);
            busy_num++;
        }
        pthread_mutex_unlock(&(a_worker->worker_lock));
    }
    pthread_mutex_unlock(&(pool->info_lock));
    printf("the busy work%d\n",busy_num);
    return ((float)busy_num)/((float)total);

}
//search the idle work pthread from the pool
struct Gthread_pool_worker * search_idle_worker(Gthread_pool *pool){
    assert(pool);
    struct list_head *pos;
    struct Gthread_pool_worker *temp_worker;

    pthread_mutex_lock(&(pool->info_lock));
    list_for_each(pos,&(pool->workers)){
        temp_worker = list_entry(pos,struct Gthread_pool_worker,link_node);
        pthread_mutex_lock(&(temp_worker->worker_lock));
        if(temp_worker->state == READY){
            temp_worker->state = BUSY; //this worker will be BUSY or be del
            pthread_mutex_unlock(&(temp_worker->worker_lock));
            pthread_mutex_unlock(&(pool->info_lock));
            return temp_worker;
        }
        else
            pthread_mutex_unlock(&(temp_worker->worker_lock));
    }
    pthread_mutex_unlock(&(pool->info_lock));
    return NULL;
}

//manage the worker(pthread) in the pool
void *worker_manage(void *arg){
    assert(arg);
    pthread_detach(pthread_self()); //set pthread to unjoinable  when return don't need phread_join 
    struct Gthread_pool *pool =(struct Gthread_pool *)arg;
    struct Gthread_pool_worker *del_worker;
    struct Gthread_pool_task   *del_task;
    struct list_head *pos;
    sleep(1);//don't need to use the cpu everytime

    while(1){
        if(pool->flag ==SHUTDOWN){
            sem_post(&(pool->surplus_task_num));
            while(0 == pthread_kill(pool->task_distribute_worker,0))
                select(0,NULL,NULL,NULL,&delay);
            pthread_mutex_lock(&(pool->info_lock));
            while((pos = pool->workers.next)!=&pool->workers){
                del_worker = list_entry(pos,struct Gthread_pool_worker,link_node);
                del_worker_from_pool(del_worker,pool);
                free(del_worker);
                pool->mutex_data.worker_num--;
            }
            
            list_for_each(pos,&(pool->task_list)){
                del_task = list_entry(pos,struct Gthread_pool_task,link_node);
                list_del(pos);
                free(del_task);
                pool->mutex_data.task_num--;
            }
            pthread_mutex_unlock(&(pool->info_lock));
            pthread_exit(NULL);
        }
        if(get_pool_now_using(pool)<POOL_GATE){
            del_worker = search_idle_worker(pool);
            if(del_worker==NULL){
                sleep(1);
                continue;
            }
            pthread_mutex_lock(&(pool->info_lock));
            del_worker_from_pool(del_worker,pool);
            free(del_worker);
            pool->mutex_data.worker_num--;
            pthread_mutex_unlock(&(pool->info_lock));
        }
        sleep(1);
    }
}

// the task distibute 
void *distribute_task(void * arg){
    pthread_detach(pthread_self());  //to be unjoinable
    assert(arg);
    struct Gthread_pool *pool = (struct Gthread_pool *)arg;
    pthread_detach(pthread_self());
    struct Gthread_pool_worker *idle_worker;
    struct Gthread_pool_task   *distribute_task;
    struct list_head *pos;
#if DEBUG == 1
    pthread_mutex_lock(&(pool->IO_lock));
    printf("NOW start task distribute\n");
    pthread_mutex_unlock(&(pool->IO_lock));
#endif
    while(1){
        sem_wait(&(pool->surplus_task_num)); // wait a task,if there no task will blocked
        if(pool->flag == SHUTDOWN)
            pthread_exit(NULL);
        pos = pool->task_list.next;
        distribute_task = list_entry(pos ,struct Gthread_pool_task,link_node);
        idle_worker = search_idle_worker(pool);
        if(idle_worker != NULL){
            pthread_mutex_lock(&(pool->info_lock));
            pool->mutex_data.task_num--;
            list_del(pos);//this task have a worker for it so delete from the task list
            pthread_mutex_unlock(&(pool->info_lock));
            
            pthread_mutex_lock(&(idle_worker->worker_lock));
            idle_worker->worker_task = distribute_task->proccess;
            idle_worker->worker_task_arg = distribute_task->arg;
            pthread_cond_signal(&(idle_worker->worker_cond));//send a sigle to the READY worker
            pthread_mutex_unlock(&(idle_worker->worker_lock));
            free(distribute_task);
            continue;            
        }
        else{
            printf("need new worker\n");
            if(pool->mutex_data.worker_num <pool->max_workers){//  add a new worker
                idle_worker = (struct Gthread_pool_worker *)malloc(sizeof(struct Gthread_pool_worker));
                if(NULL == idle_worker){
#if DEBUG == 1         
                    pthread_mutex_lock(&(pool->IO_lock));
                    printf("malloc a new worker in distribute_task function failed!");
                    pthread_mutex_unlock(&(pool->IO_lock));
#endif                  
                    exit(15);
                }
                pthread_mutex_lock(&(pool->info_lock));
                if(FAILURE == add_worker(idle_worker,pool)){
                    pthread_mutex_unlock(&(pool->info_lock));
                    sem_post(&(pool->surplus_task_num));
                    free(idle_worker);
                    continue;
                }
                pool->mutex_data.worker_num++;
                pool->mutex_data.task_num--;
                list_del(pos);
                pthread_mutex_unlock(&(pool->info_lock));
                pthread_mutex_lock(&(idle_worker->worker_lock));
                idle_worker->worker_task = distribute_task->proccess;
                idle_worker->worker_task_arg = distribute_task->arg;
                idle_worker->state =BUSY;
                pthread_cond_signal(&(idle_worker->worker_cond));
                pthread_mutex_unlock(&(idle_worker->worker_lock));
                free(distribute_task);
              //  sem_post(&(pool->surplus_task_num));
                continue;
            }
            sem_post(&(pool->surplus_task_num));//can't proccess this task now wait for next time
            select(0,NULL,NULL,NULL,&delay);
            continue;
        }
    }
}

//add a task to the task_list
int add_job(struct Gthread_pool *pool,void *(*job)(void *arg),void *arg){
    assert(pool);
    assert(arg);
    struct Gthread_pool_task *add_to_task;
    if(pool->flag == SHUTDOWN)
        return FAILURE;
    pthread_mutex_lock(&(pool->info_lock));
    add_to_task=(struct Gthread_pool_task *)malloc(sizeof(struct Gthread_pool_task));
    if(NULL == add_to_task)
        exit(15);
    add_task(add_to_task,pool,job,arg);
    pool->mutex_data.task_num++;
    sem_post(&(pool->surplus_task_num));
    pthread_mutex_unlock(&(pool->info_lock));
    return SUCCESS;
}

//add a task
int add_task(struct Gthread_pool_task *task,struct Gthread_pool *pool,void*(*proccess)(void *arg),void *arg){
    assert(task);
    assert(arg);

    task->proccess=proccess;
    task->arg=arg;
    list_add_tail(&(task->link_node),&(pool->task_list));
    return SUCCESS;

}





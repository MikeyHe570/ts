#ifndef _MY_TIMER
#define _MY_TIMER
#include <time.h>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <unistd.h>
#include "Glist.h" 

#define MAX_FD 10000
using namespace std; 
#define LIFE_TIME 10
#define SLOT_TIME 1

typedef struct timer{
    time_t over_time;
    int fd;
    void (*callback_func)(void * arg);
    list_head node; 
} timer_type;
extern timer_type *timers[MAX_FD];
extern pthread_mutex_t timer_lock;
bool timer_cmp(timer_type *timer_l,timer_type *timer_r);
void tick_handle(struct list_head *head);
void over_time_handle(void *fd);
void timer_add(timer_type *new_timer,struct list_head *head);
void timer_del(timer_type *temer_del,struct list_head *head);
#endif

#include "timer.h"
timer_type *timers[MAX_FD];
pthread_mutex_t timer_lock;
//a compare function for two times
bool timer_cmp(timer_type *timer_l,timer_type *timer_r){
    if(timer_l->over_time > timer_r->over_time)
        return true;
    else
        return false;
}
//the hanle to the overtime client
void tick_handle(struct list_head *head){
    timer_type *entry;
    time_t cur_time =time(NULL);
    
    while(!list_empty(head)){
        entry =list_entry(head->next,timer_type,node);
        if(entry->over_time>cur_time)
            break;
        else{
            entry->callback_func((void *)&entry->fd);
            list_del(head->next);
            free(entry);
        }
    }
}

//this client has overtime so get out
void over_time_handle(void *fd){
    close(*(int *)fd);
    timers[*(int *)fd] =NULL;
}

//add a timer to cheak the client
void timer_add(timer_type *new_timer,struct list_head *head){
    if(new_timer == NULL)
        return;
    if(list_empty(head)){
        list_add(&(new_timer->node),head);
        timers[new_timer->fd] = new_timer;
        return;
    }
    struct list_head *pos;
    timer_type *entry;
    list_for_each(pos,head){
        entry =list_entry(pos,timer_type,node);
        if(timer_cmp(entry,new_timer)){  //find the righte place to insert the timer,this list is a uper list
            __list_add(&(new_timer->node),pos->prev,pos);
            timers[new_timer->fd] = new_timer;
            return;
        }
    }
    if(pos == head){//this timer is long than the other time so insert it in the tail
        timers[new_timer->fd]=new_timer;
        list_add_tail(&(new_timer->node),head);
    }
}

//this timer is no usage so delete and free 
void timer_del(timer_type *timer_del,struct list_head *head){
    if(timer_del == NULL || list_empty(head))
        return;
    timers[timer_del->fd]=NULL;
    list_del(&(timer_del->node));
    free(timer_del);
}





#include "task_manager.h"

static struct task_details* task[MAX_TASKS];
static pthread_t analyze_thread[MAX_TASKS];
pthread_mutex_t status_mutex[MAX_TASKS];
pthread_mutex_t permission[MAX_TASKS];

void init_task_manager()
{   
    for(int i = 0; i < MAX_TASKS;i++){
        task[i] = (struct task_details*) malloc(sizeof(struct task_details));
        task[i]->status = PENDING;
    }
}

int get_new_task_id()
{
    for(int i = 0; i < MAX_TASKS;i++)
        if(get_task_status(i) == PENDING){
            return i;
        } 
    return -1;
}

pthread_t* get_task_thread(int task_id)
{
    return &analyze_thread[task_id];
}

void set_task_status(int task_id,int new_task_status)
{
    pthread_mutex_lock(&status_mutex[task_id]);
    syslog(LOG_NOTICE,"Changed task: %d from %d to %d",task_id,task[task_id]->status,new_task_status);
    task[task_id]->status = new_task_status;
    syslog(LOG_NOTICE, "New task status is: %d ",task[task_id]->status);
    pthread_mutex_unlock(&status_mutex[task_id]);
}

int get_task_status(int task_id)
{
    int task_status;
    pthread_mutex_lock(&status_mutex[task_id]);
    task_status = task[task_id]->status;
    pthread_mutex_unlock(&status_mutex[task_id]);
    return task_status;
}

void set_task_details(struct task_details* info)
{
    task[info->task_id] = info;
}

// Any paused thread will get stuck here until it's resumed
void permission_to_continue(int task_id)
{
    pthread_mutex_lock(&permission[task_id]);
    pthread_mutex_unlock(&permission[task_id]);
}

void suspend_task(int task_id,int pause_type)
{
    assert(pause_type == PAUSED || pause_type == PRIORITY_WAITING);
    set_task_status(task_id,pause_type);
    pthread_mutex_lock(&permission[task_id]);
}

void resume_task(int task_id)
{
    // task manager will resume this task if it has high enough priority
    set_task_status(task_id,PRIORITY_WAITING);
}

void notify_task_done(int task_id)
{   
    syslog(LOG_NOTICE,"Job %d done\n", task_id);
    set_task_status(task_id,DONE);
}

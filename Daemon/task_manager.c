#include "task_manager.h"

static int task_status[MAX_TASKS];
static pthread_t analyze_thread[MAX_TASKS];

void init_task_manager()
{
    for(int i = 0; i < MAX_TASKS;i++)
        task_status[i] = PENDING;
}

int get_new_task_id()
{
    for(int i = 0; i < MAX_TASKS;i++)
        if(task_status[i] == PENDING || task_status[i] == REMOVED){
            set_task_status(i,PROCESSING);
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
    task_status[task_id] = new_task_status;
}

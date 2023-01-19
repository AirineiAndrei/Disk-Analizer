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

int prefix(const char *new_path, const char *old_path)
{
    if(strlen(new_path) < strlen(old_path))
        return 0;

    return strncmp(new_path, old_path, strlen(old_path)) == 0;
}

struct task_details *get_task_info(int task_id)
{
    return task[task_id];
}

char *get_task_path(int task_id)
{
    return task[task_id]->path; 
}

int get_new_task_id(const char *new_path)
{
    //  returneaza id-ul taskului care contine deja noul path
    for(int i = 0; i < MAX_TASKS; i++)
        if(get_task_status(i) != PENDING && prefix(new_path, get_task_path(i)))
            return -i - 1;

    for(int i = 0; i < MAX_TASKS;i++)
        if(get_task_status(i) == PENDING){
            return i;
        }
    return MAX_TASKS;
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
    int current_status = get_task_status(task_id);
    if(current_status != PAUSED && current_status != PRIORITY_WAITING)
        pthread_mutex_lock(&permission[task_id]);
    set_task_status(task_id,pause_type);
}

void resume_task(int task_id)
{
    // task manager will resume this task if it has high enough priority

    set_task_status(task_id,PRIORITY_WAITING);
}

void remove_task(int task_id)
{
    if(get_task_status(task_id) != DONE)
    {
        if(pthread_cancel(*get_task_thread(task_id)))
        {
            syslog(LOG_ERR, "The da_daemon: cancel thread with ID: %d failed. Error: %s\n",task_id, strerror(errno));
            exit(EXIT_FAILURE);
        }

        void *res;
        if(pthread_join(*get_task_thread(task_id), &res))
        {
            syslog(LOG_ERR, "The da_daemon: joined thread with ID: %d failed. Error: %s\n",task_id, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if(res == PTHREAD_CANCELED)
            syslog(LOG_ERR, "The da_daemon: cancel thread with ID: %d done.\n",task_id);
        else
        {
            syslog(LOG_ERR, "The da_daemon: joined thread with ID: %d failed. Error: %s\n",task_id, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    //  this task can take new work
    set_task_status(task_id, PENDING);
}
void notify_task_done(int task_id)
{   
    syslog(LOG_NOTICE,"Job %d done\n", task_id);
    set_task_status(task_id,DONE);
    priority_compute(); // check if we need to work on less important tasks
}

void priority_compute()
{
    syslog(LOG_NOTICE,"RECOMPUTING TASK PRIORITIES");
    int max_priority = 0;
    for(int i = 0; i < MAX_TASKS;i++)
    {
        int current_status = get_task_status(i);
        if(current_status == PROCESSING || current_status == PRIORITY_WAITING)
        {
            if(task[i]->priority > max_priority)
                max_priority = task[i]->priority;
        }
    }

    for(int i = 0; i < MAX_TASKS;i++)
    {
        if(task[i]->priority == max_priority)
        {
            if(get_task_status(i) == PRIORITY_WAITING)
            {
                pthread_mutex_unlock(&permission[i]);
                set_task_status(i,PROCESSING);
            }
        }
        else
        {
            if(get_task_status(i) == PROCESSING)
                suspend_task(i,PRIORITY_WAITING);
        }
    }
}

int get_task_files_no(int task_id)
{
    int _files;
    pthread_mutex_lock(&status_mutex[task_id]);
    _files = task[task_id]->files;
    pthread_mutex_unlock(&status_mutex[task_id]);
    return _files;
}

void set_task_files_no(int task_id, int no_files)
{
    pthread_mutex_lock(&status_mutex[task_id]);
    task[task_id]->files = no_files;
    pthread_mutex_unlock(&status_mutex[task_id]);
}

int get_task_dirs_no(int task_id)
{
    int _dirs;
    pthread_mutex_lock(&status_mutex[task_id]);
    _dirs = task[task_id]->dirs;
    pthread_mutex_unlock(&status_mutex[task_id]);
    return _dirs;
}

void set_task_dirs_no(int task_id, int no_dirs)
{
    pthread_mutex_lock(&status_mutex[task_id]);
    task[task_id]->dirs = no_dirs;
    pthread_mutex_unlock(&status_mutex[task_id]);
}

int get_task_priority(int task_id)
{
    int _priority;
    pthread_mutex_lock(&status_mutex[task_id]);
    _priority = task[task_id]->priority;
    pthread_mutex_unlock(&status_mutex[task_id]);
    return _priority;
}

void set_task_progress(int task_id, double progress)
{
    pthread_mutex_lock(&status_mutex[task_id]);
    task[task_id]->progress = progress;
    pthread_mutex_unlock(&status_mutex[task_id]);
}

double get_task_progress(int task_id)
{
    double _progress;
    pthread_mutex_lock(&status_mutex[task_id]);
    _progress = task[task_id]->progress;
    pthread_mutex_unlock(&status_mutex[task_id]);
    return _progress;
}
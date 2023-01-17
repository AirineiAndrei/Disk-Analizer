#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

// Task manager handles tasks id-s and priorities as well as the start stop pause remove functionality

#define MAX_TASKS 20

#include <syslog.h>
#include <stdlib.h>
#include<pthread.h>
#include "../Shared/shared.h"

void init_task_manager();

int get_new_task_id();

pthread_t* get_task_thread(int task_id);

void set_task_status(int task_id,int new_task_status);

int get_task_status(int task_id);

void permission_to_continue(int task_id);

void set_task_details(struct task_details* info);

void suspend_task(int task_id);

void resume_task(int task_id);

#endif // TASK_MANAGER_H

#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

// Task manager handles tasks id-s and priorities as well as the start stop pause remove functionality

#define MAX_TASKS 20

#include<pthread.h>
#include "../Shared/shared.h"

void init_task_manager();

int get_new_task_id();

pthread_t* get_task_thread(int task_id);

void set_task_status(int task_id,int new_task_status);

#endif // TASK_MANAGER_H

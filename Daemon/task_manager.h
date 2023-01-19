#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

// Task manager handles tasks id-s and priorities as well as the start stop pause remove functionality

#define MAX_TASKS 20

#include <assert.h>
#include <syslog.h>
#include <stdlib.h>
#include<pthread.h>
#include<errno.h>
#include "../Shared/shared.h"

void init_task_manager();

int prefix(const char *new_path, const char *old_path);

struct task_details *get_task_info(int task_id);

char *get_task_path(int task_id);

int get_new_task_id(const char *new_path);

pthread_t* get_task_thread(int task_id);

void set_task_status(int task_id,int new_task_status);

int get_task_status(int task_id);

void permission_to_continue(int task_id);

void set_task_details(struct task_details* info);

void suspend_task(int task_id,int pause_type);

void resume_task(int task_id);

void remove_task(int task_id);

void notify_task_done(int task_id);

void priority_compute();

#endif // TASK_MANAGER_H

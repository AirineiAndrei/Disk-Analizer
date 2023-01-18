#ifndef DISKANALYZER_SHARED_H
#define DISKANALYZER_SHARED_H

#define MAX_PATH_LENGTH 512
#define MAX_PROGRESS_LENGTH 20

// instructions - da sends

#define ADD 1
#define SUSPEND 2
#define RESUME 3
#define REMOVE 4
#define INFO 5
#define LIST 6
#define PRINT 7
#define TERMINATE 8

struct request_details{
    int id, priority, arg_pid;
    char* path;
};

// tasks - daemon executes

#define PENDING 1 // free task 
#define PROCESSING 2 
#define PAUSED 3
#define DONE 4
#define PRIORITY_WAITING 5

#define OUT_DIRECTORY "/tmp/da_daemon/"

struct task_details{
    int task_id, status, priority;
    char path[MAX_PATH_LENGTH];
    char progress_details[MAX_PROGRESS_LENGTH];
};

#include <string.h>
void add_to_path(const char* current_path,const char* add_path,char *ans);

#endif // DISKANALYZER_SHARED_H
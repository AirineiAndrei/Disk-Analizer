#ifndef DISKANALYZER_SHARED_H

#define MAX_PATH_LENGTH 512

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

#define PENDING 1
#define PROCESSING 2
#define PAUSED 3
#define DONE 4
#define REMOVED 5;

struct task_details{
    int task_id, status, priority, worker_id;
    char path[MAX_PATH_LENGTH];
};

#endif // DISKANALYZER_SHARED_H
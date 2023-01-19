#ifndef ANALYZER_H
#define ANALYZER_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <dirent.h>
#include <string.h>
#include "../Shared/shared.h"
#include "task_manager.h"

void* analyze(void* info);

off_t fsize(const char *filename);

long long dfs_find_size_on_disk(const char *path,int task_id);

long long write_report(const char *path,const char* relative_path, FILE * out_fd, long long total_size,int depth,int task_id);

#endif // ANALYZER_H

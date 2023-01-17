#ifndef ANALYZER_H
#define ANALYZER_H

#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <dirent.h>
#include <string.h>
#include "../Shared/shared.h"
#include "task_manager.h"

void* analyze(void* info);

off_t fsize(const char *filename);

long long dfs_find_size_on_disk(const char *path);


#endif // ANALYZER_H

#include "analyzer.h"

off_t fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

void* analyze(void* info)
{
    struct task_details* current_task = (struct task_details*) info;
    syslog(LOG_NOTICE,"Starting analyzing job from  %s\n", current_task->path);
    int total_size = dfs_find_size_on_disk(current_task->path);

    syslog(LOG_NOTICE,"Total size of %s task id %d is %d\n", current_task->path,current_task->task_id,total_size);
    return NULL;
}

long long dfs_find_size_on_disk(const char *path)
{
    // syslog(LOG_NOTICE,"DFS in %s\n", path);
    DIR *dir = opendir(path);
    if(dir == NULL) 
    {
        return 0; // we are not in a directory
    }
    char sub_path[MAX_PATH_LENGTH] = "";

    struct dirent *sub_dir;
    long long size = 0;
    while(1) // for every file in current folder
    {
        sub_dir = readdir(dir);
        if(!sub_dir)break;
        if (strcmp(sub_dir->d_name, ".") != 0 && strcmp(sub_dir->d_name, "..") != 0) {
            strcpy(sub_path,path);
            if (sub_path[strlen(sub_path) - 1] != '/') {
                strcat(sub_path, "/");
            }
            strcat(sub_path,sub_dir->d_name);
            // if(sub_dir->d_type != 4)// just in case we want to consider only containing files
            size += (long long)fsize(sub_path);
            size += dfs_find_size_on_disk(sub_path);
        }
    }
    closedir(dir);
    return size;
}

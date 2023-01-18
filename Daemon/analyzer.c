#include "analyzer.h"

off_t fsize(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}
const char *out_directory = "/tmp/da_daemon/";

void* analyze(void* info)
{
    struct task_details* current_task = (struct task_details*) info;
    syslog(LOG_NOTICE,"Starting analyzing job from  %s\n", current_task->path);
    int total_size = dfs_find_size_on_disk(current_task->path,current_task->task_id); // Need this to estimate progress
    syslog(LOG_NOTICE,"Total size of %s task id %d is %d\n", current_task->path,current_task->task_id,total_size);

    char output_path[MAX_PATH_LENGTH];
    snprintf(output_path, MAX_PATH_LENGTH, "/tmp/da_daemon/%d", current_task->task_id);

    struct stat st = {0};

    if (stat(out_directory, &st) == -1) {
        mkdir(out_directory, 0777);
    }

    remove(output_path);
    FILE * out_fd = fopen(output_path, "w");
    syslog(LOG_NOTICE,"Output path is %s",output_path);

    write_report(current_task->path,"/",out_fd,total_size,0,current_task->task_id);

    fclose(out_fd);

    notify_task_done(current_task->task_id);
    
    return NULL;
}

long long write_report(const char *path,const char* relative_path, FILE * out_fd, int total_size,int depth,int task_id)
{
    permission_to_continue(task_id);
    DIR *dir = opendir(path);
    if(dir == NULL) 
    {
        return 0; // we are not in a directory
    }
    char sub_path[MAX_PATH_LENGTH] = "";
    char sub_relative_path[MAX_PATH_LENGTH] = "";

    struct dirent *sub_dir;
    long long size = 0;
    while(1) // for every file in current folder
    {
        sub_dir = readdir(dir);
        if(!sub_dir)break;
        if (strcmp(sub_dir->d_name, ".") != 0 && strcmp(sub_dir->d_name, "..") != 0) {

            add_to_path(path,sub_dir->d_name,sub_path);
            add_to_path(relative_path,sub_dir->d_name,sub_relative_path);

            // if(sub_dir->d_type != 4)// just in case we want to consider only containing files not folder size (4096)
            size += (long long)fsize(sub_path);
            size += write_report(sub_path,sub_relative_path,out_fd,total_size,depth+1,task_id);
        }
    }
    closedir(dir);

    double percent = (double) size / (double) total_size * 100;

    char show_depth[MAX_PATH_LENGTH] = "";
    for(int i = 0;i<2*depth;i++)
        show_depth[i] = ' ';

    fprintf(out_fd,"%s|-%s %f%% %lldB \n",show_depth,relative_path,percent,size);
    // size is the size of this subdir
    return size;
}

long long dfs_find_size_on_disk(const char *path,int task_id)
{
    permission_to_continue(task_id);
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
            add_to_path(path,sub_dir->d_name,sub_path);
            // if(sub_dir->d_type != 4)// just in case we want to consider only containing files
            size += (long long)fsize(sub_path);
            size += dfs_find_size_on_disk(sub_path,task_id);
        }
    }

    closedir(dir);
    return size;
}

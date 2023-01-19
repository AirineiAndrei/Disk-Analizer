#include "daemon.h"

int SocketFD, returnFD;
struct sockaddr_in sa, return_sa;
static struct request_details *current_request = NULL;

void create_socket()
{
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(SocketFD == -1)
    {
        syslog(LOG_ERR, "The da_daemon: cannot create socket. Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) == -1)
    {
        syslog(LOG_ERR, "The da_daemon: reuse address socket failed. Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    syslog(LOG_NOTICE, "Socket address is: %d\n", sa.sin_addr.s_addr);

    if(bind(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1)
    {
        syslog(LOG_ERR, "The da_daemon: bind failed. Error: %s\n", strerror(errno));
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if(listen(SocketFD, 10) == -1)
    {
        syslog(LOG_ERR, "The da_daemon: listen failed. Error: %s\n", strerror(errno));
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

}

void open_return_socket()
{
	returnFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(returnFD == -1)
	{
		syslog(LOG_ERR, "The da_daemon: cannot create return socket. Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
	}

	memset(&return_sa, 0, sizeof return_sa);

	return_sa.sin_family = AF_INET;
	return_sa.sin_port = htons(1200);
	return_sa.sin_addr.s_addr = 0;

	if(connect(returnFD, (struct sockaddr *)&return_sa, sizeof return_sa) == -1)
	{
		syslog(LOG_ERR, "The da_daemon: cannot connect to return socket. Error: %s\n", strerror(errno));
		close(returnFD);
		exit(EXIT_FAILURE);
	}

	syslog(LOG_NOTICE, "The da_daemon connected to return socket.");
}

void return_response(const char *const buffer)
{
    open_return_socket();
    write(returnFD, buffer, strlen(buffer));
    close(returnFD);
}

static void skeleton_daemon()
{

    pid_t pid;

    pid = fork();

    if(pid < 0)
        exit(EXIT_FAILURE);

    if(pid > 0)
        exit(EXIT_SUCCESS);

    if(setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if(pid < 0)
        exit(EXIT_FAILURE);

    if(pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    for(int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
    {
        close(x);
    }

    openlog("da_daemon", LOG_PID, LOG_DAEMON);
}

struct request_details * get_request_details(const char *const buff){
    struct request_details * incoming_request = (struct request_details*)malloc(sizeof(struct request_details));

    if(sscanf(buff, "ID %d\n", &incoming_request->id) == 1)
    {

        incoming_request->path = malloc(sizeof(char) * MAX_PATH_LENGTH);

        if(incoming_request->id == ADD)
        {
            if(sscanf(buff, "ID %d\nPRIORITY %d\nPATH %s\n", &incoming_request->id, &incoming_request->priority, incoming_request->path) != 3)
            {
                syslog(LOG_ERR, "The da_daemon: request read failed. Error: %s\n", strerror(errno));
                return NULL;
            }
        }

        if(incoming_request->id == SUSPEND || incoming_request->id == RESUME 
            || incoming_request->id == REMOVE || incoming_request->id == INFO
            || incoming_request->id == PRINT)
        {
            if(sscanf(buff, "ID %d\nPID %d\n", &incoming_request->id, &incoming_request->arg_pid) != 2)
            {
                syslog(LOG_ERR, "The da_daemon: request read failed. Error: %s\n", strerror(errno));
                return NULL;
            }

            if(incoming_request->arg_pid >= MAX_TASKS)
            {
                syslog(LOG_NOTICE, "Daemon received wrong task ID.\n");

                char response[1024]= "";
                sprintf(response, "The highest task ID is %d\n", MAX_TASKS - 1);
                return_response(response);

                return NULL;
            }
        }
        
        return incoming_request;
    }
    
    return NULL;
}

_Noreturn int run_daemon()
{

    syslog(LOG_NOTICE, "The da_daemon started.");

    while(1)
    {

        int ConnectFD = accept(SocketFD, NULL, NULL);

        if(ConnectFD == -1)
        {
            syslog(LOG_ERR, "The da_daemon: accept failed. Error: %s\n", strerror(errno));
            close(SocketFD);
            exit(EXIT_FAILURE);
        }

        // Daemon reads instructions from the da here

        char instructions[512];
        int size = 512;
        int nr_read = read(ConnectFD, instructions, size);

        syslog(LOG_NOTICE, "Daemon read %d\n", nr_read);

        if(nr_read < 1)
        {
            syslog(LOG_ERR, "The da_daemon: read failed. Error: %s\n", strerror(errno));
            continue;
        }
        else
        {

            syslog(LOG_NOTICE, "Daemon received %s\n", instructions);

            // execute the instructions

            if(current_request != NULL && current_request->path != NULL) 
            {
                free(current_request->path);
            }

            if(current_request != NULL)
            {
                free(current_request);
            }

            current_request = get_request_details(instructions);

            if(current_request == NULL)
            {
                syslog(LOG_NOTICE, "Daemon failed to process instructions.\n");    
                continue;
            }

            syslog(LOG_NOTICE, "Daemon processed instructions.\n");

            char response[1024]= "";

            if(current_request->id == ADD)
            {
                // add task
                syslog(LOG_NOTICE, "ADD task is: %d, %s\n", current_request->priority, current_request->path);
                int current_task_id = get_new_task_id(current_request->path);
                if(current_task_id >= 0 && current_task_id != MAX_TASKS)
                {
                    // We start this task
                    struct task_details* current_task = (struct task_details*) malloc(sizeof(struct task_details));
                    current_task->task_id = current_task_id;
                    current_task->status = PROCESSING;
                    current_task->priority = current_request->priority;

                    for(int i = 0;i < MAX_PATH_LENGTH; i++)
                        current_task->path[i] = current_request->path[i];

                    current_task->files = 0;
                    current_task->dirs = 0;
                    
                    set_task_details(current_task);
                    
                    if(pthread_create(get_task_thread(current_task_id) , NULL , analyze , current_task)) {
                        perror ("da_daemon cannot create thread");
                        exit(-1);
                    }

                    suspend_task(current_task_id,PRIORITY_WAITING);
                    syslog(LOG_NOTICE, "TASK with id: %d thread created.\n", current_task_id);
                    sprintf(response,"Created analysis task with ID %d for %s and priority %d.\n",current_task_id,current_task->path,current_task->priority);
                    return_response(response);
                }
                // TO DO: send back the task id to da / a message if we can not start another task
                else if(current_task_id == MAX_TASKS)
                    return_response("The daemon can't take more tasks\n");
                else
                {
                    sprintf(response, "Directory ’%s’ is already included in analysis with ID ’%d’.\n", current_request->path, -(current_task_id + 1));
                    return_response(response);
                }
            }

            if(current_request->id == SUSPEND || current_request->id == RESUME || current_request->id == REMOVE)
            {
                syslog(LOG_NOTICE, "SRR task received\n");

                // modify task status

                if(current_request->id == SUSPEND)
                {
                    int id = current_request->arg_pid;

                    syslog(LOG_NOTICE, "Suspend: %d",id);
                
                    switch(get_task_status(id)){
                    case PENDING:
                        return_response("Task doesn't exist.\n");
                        break;
                    case PROCESSING:
                        suspend_task(id,PAUSED);

                        syslog(LOG_NOTICE, "Paused analysis task with ID %d.\n", id);
                        return_response("Task paused succesfully.\n");
                        break;
                    case PAUSED:
                        return_response("Task already paused.\n");
                        break;
                    case DONE:
                        return_response("Task is already done.\n");
                        break;
                    case PRIORITY_WAITING:
                        suspend_task(id, PAUSED);

                        syslog(LOG_NOTICE, "Paused analysis task with ID %d.\n", id);
                        return_response("Task paused succesfully.\n");
                        break;
                    }
                }

                if(current_request->id == RESUME)
                {
                    int id = current_request->arg_pid;

                    syslog(LOG_NOTICE, "Resume: %d",id);

                    switch(get_task_status(id)){
                    case PENDING:
                        return_response("Task doesn't exist.\n");
                        break;
                    case PROCESSING:
                        return_response("Task not paused.\n");
                        break;
                    case PAUSED:
                        resume_task(id);

                        syslog(LOG_NOTICE, "Resumed analysis task with ID %d\n", id);
                        return_response("Task resumed.\n");
                        break;
                    case DONE:
                        return_response("Task not paused.\n");
                        break;
                    case PRIORITY_WAITING:
                        return_response("Task not paused.\n");
                        break;
                    }
                }

                if(current_request->id == REMOVE)
                {
                    int id = current_request->arg_pid;

                    char response[1024]= "";

                    syslog(LOG_NOTICE, "Remove: %d",id);

                    switch(get_task_status(id)){
                    case PENDING:
                        return_response("Task doesn't exist.\n");
                        break;
                    case PROCESSING:
                        remove_task(id);
                        syslog(LOG_NOTICE, "Removed analysis task with ID %d.\n", id);
            

                        sprintf(response, "Removed analysis task with ID ’%d’, status ’processing’ for ’%s’.\n", id, get_task_path(id));
                        return_response(response);
                        break;
                    case PAUSED:
                        remove_task(id);
                        syslog(LOG_NOTICE, "Removed analysis task with ID %d\n", id);

                        sprintf(response, "Removed analysis task with ID ’%d’, status ’paused’ for ’%s’.\n", id, get_task_path(id));
                        return_response(response);
                        break;
                    case DONE:
                        remove_task(id);
                        syslog(LOG_NOTICE, "Removed analysis task with ID %d.\n", id);

                        sprintf(response, "Removed analysis task with ID ’%d’, status ’done’ for ’%s’.\n", id, get_task_path(id));
                        return_response(response);
                        break;
                    case PRIORITY_WAITING:
                        remove_task(id);
                        syslog(LOG_NOTICE, "Removed analysis task with ID %d.\n", id);

                        sprintf(response, "Removed analysis task with ID ’%d’, status ’priority_waiting’ for ’%s’.\n", id, get_task_path(id));
                        return_response(response);
                        break;
                    }
                }
                
            }

            if(current_request->id == INFO)
            {
                // print info about a task
                syslog(LOG_NOTICE, "INFO task received\n");

                int id = current_request->arg_pid;

                struct task_details* info = get_task_info(id);
    
                char response[1024]= "";

                syslog(LOG_NOTICE, "INFO status: %d\n", info->status);

                switch(info->status){
                case PENDING:
                    return_response("Task doesn't exist.\n");
                    break;
                case PROCESSING:
                    sprintf(response, "ID  Path  Priority  Done  Status  Details\n%d  %s  %d  %0.2lf%%  processing.  %d files, %d dirs\n", id, info->path, get_task_priority(id), (double)0, get_task_files_no(id), get_task_dirs_no(id));
                    return_response(response);
                    break;
                case PAUSED:
                    sprintf(response, "ID  Path  Priority  Done  Status  Details\n%d  %s  %d  %0.2lf%%  processing.  %d files, %d dirs\n", id, info->path, get_task_priority(id), (double)0, get_task_files_no(id), get_task_dirs_no(id));
                    return_response(response);
                    break;
                case DONE:
                    sprintf(response, "ID  Path  Priority  Done  Status  Details\n%d  %s  %d  %0.2lf%%  processing.  %d files, %d dirs\n", id, info->path, get_task_priority(id), get_task_progress(id), get_task_files_no(id), get_task_dirs_no(id));
                    return_response(response);
                    break;
                case PRIORITY_WAITING:
                    sprintf(response, "ID  Path  Priority  Done  Status  Details\n%d  %s  %d  %0.2lf%%  processing.  %d files, %d dirs\n", id, info->path, get_task_priority(id), get_task_progress(id), get_task_files_no(id), get_task_dirs_no(id));
                    return_response(response);
                    break;
                }
            }

            if(current_request->id == LIST)
            {
                // list all tasks
                syslog(LOG_NOTICE, "LIST task received\n");

                char response[10240] = "ID  Path  Done Status         Details\n";
                char curr[512] = "";

                for(int i = 0; i < MAX_TASKS; i++)
                    if(get_task_status(i) != PENDING)
                    {
                        sprintf(curr, "%d  %s    %0.2lf%% in progress   %d files, %d dirs\n", i, get_task_path(i), get_task_progress(i), get_task_files_no(i), get_task_dirs_no(i));
                        strcat(response, curr);
                    }

                return_response(response);
            }

            if(current_request->id == PRINT)
            {
                // print a task
                syslog(LOG_NOTICE, "PRINT task received\n");

                int id = current_request->arg_pid;

                if(get_task_status(id) == DONE)
                    return_response("1\n");
                else
                    return_response("0\n");
            }

            if(current_request->id == TERMINATE)
            {
                if(shutdown(ConnectFD, SHUT_RDWR) == -1)
                {
                    syslog(LOG_ERR, "The da_daemon: shutdown failed. Error: %s\n", strerror(errno));
                    close(ConnectFD);
                    close(SocketFD);
                    exit(EXIT_FAILURE);
                }

                close(ConnectFD);
                close(SocketFD);

                syslog(LOG_NOTICE, "The da_daemon terminated.");

                closelog();        
                exit(EXIT_SUCCESS);
            }
        }

        if(shutdown(ConnectFD, SHUT_RDWR) == -1)
        {
            syslog(LOG_ERR, "The da_daemon: shutdown failed. Error: %s\n", strerror(errno));
            close(ConnectFD);
            close(SocketFD);
            exit(EXIT_FAILURE);
        }

        close(ConnectFD);
        priority_compute();
    }

    syslog(LOG_NOTICE, "The da_daemon terminated.");

    closelog();
}

int main()
{
    skeleton_daemon();
    create_socket();
    init_task_manager();

    int error = run_daemon();

    if(error)
        syslog(LOG_NOTICE, "Failed to start da_daemon, error code: %d\n", error);

    return 0;
}

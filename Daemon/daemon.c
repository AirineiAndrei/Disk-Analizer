#include "daemon.h"

int SocketFD;
struct sockaddr_in sa;
static struct request_details *current_request = NULL;

void create_socket()
{
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1)
    {
        syslog(LOG_ERR, "The da_daemon: cannot create socket. Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (setsockopt(SocketFD, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) == -1)
    {
        syslog(LOG_ERR, "The da_daemon: reuse address socket failed. Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&sa, 0, sizeof sa);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    syslog(LOG_NOTICE, "Socket address is: %d\n", sa.sin_addr.s_addr);

    if (bind(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1)
    {
        syslog(LOG_ERR, "The da_daemon: bind failed. Error: %s\n", strerror(errno));
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (listen(SocketFD, 10) == -1)
    {
        syslog(LOG_ERR, "The da_daemon: listen failed. Error: %s\n", strerror(errno));
        close(SocketFD);
        exit(EXIT_FAILURE);
    }


}

static void skeleton_daemon()
{

    pid_t pid;

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; --x)
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
        }

        
        return incoming_request;
    }
    
    return NULL;
}

_Noreturn int run_daemon()
{

    syslog(LOG_NOTICE, "The da_daemon started.");

    while (1)
    {
        int ConnectFD = accept(SocketFD, NULL, NULL);

        if (ConnectFD == -1)
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

        if (nr_read < 1)
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

            syslog(LOG_NOTICE, "Daemon crreq reset\n");

            current_request = get_request_details(instructions);

            syslog(LOG_NOTICE, "Daemon received crreq\n");

            if(current_request == NULL)
            {
                syslog(LOG_NOTICE, "Daemon failed to process instructions.\n");    
                continue;
            }

            syslog(LOG_NOTICE, "Daemon processed instructions.\n");

            if(current_request->id == ADD)
            {
                // add task
                syslog(LOG_NOTICE, "ADD task is: %d, %s\n", current_request->priority, current_request->path);
                int current_task_id = get_new_task_id();
                if(current_task_id != -1)
                {
                    // We start this task
                    struct task_details* current_task = (struct task_details*) malloc(sizeof(struct task_details));
                    current_task->task_id = current_task_id;
                    current_task->status = PENDING;
                    current_task->priority = current_request->priority;
                    for(int i=0;i<MAX_PATH_LENGTH;i++)
                        current_task->path[i] = current_request->path[i];
                    
                    if ( pthread_create (get_task_thread(current_task_id) , NULL , analyze , current_task)) {
                        perror ("da_daemon cannot create thread");
                        exit(-1);
                    }
                    syslog(LOG_NOTICE, "TASK with id: %d thread created\n", current_task_id);
                }
                // TO DO: send back the task id to da / a message if we can not start another task
            }

            if(current_request->id == SUSPEND || current_request->id == RESUME || current_request->id == REMOVE)
            {
                // modify task status
                syslog(LOG_NOTICE, "SRR task received\n");
            }

            if(current_request->id == INFO)
            {
                // print info about a task
                syslog(LOG_NOTICE, "INFO task received\n");
            }

            if(current_request->id == LIST)
            {
                // list all tasks
                syslog(LOG_NOTICE, "LIST task received\n");
            }

            if(current_request->id == PRINT)
            {
                // print a task
                syslog(LOG_NOTICE, "PRINT task received\n");
            }

            if(current_request->id == TERMINATE)
            {
                if (shutdown(ConnectFD, SHUT_RDWR) == -1)
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

        

        if (shutdown(ConnectFD, SHUT_RDWR) == -1)
        {
            syslog(LOG_ERR, "The da_daemon: shutdown failed. Error: %s\n", strerror(errno));
            close(ConnectFD);
            close(SocketFD);
            exit(EXIT_FAILURE);
        }

        close(ConnectFD);
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

    if (error)
        syslog(LOG_NOTICE, "Failed to start da_daemon, error code: %d\n", error);

    return 0;
}

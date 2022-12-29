#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>

static void skeleton_daemon(){

    pid_t pid;

    pid = fork();

    if(pid < 0)
        exit(EXIT_FAILURE);

    if(pid > 0)
        exit(EXIT_SUCCESS);

    if(setsid() < 0)
        exit(EXIT_FAILURE);

    pid = fork();

    if(pid < 0)
        exit(EXIT_FAILURE);

    if(pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    for(int x = sysconf(_SC_OPEN_MAX); x >= 0; --x){
        close(x);
    }

    openlog("da_daemon", LOG_PID, LOG_DAEMON);
}

_Noreturn int run_daemon(){

    syslog(LOG_NOTICE, "The da_daemon started.");

    while(1) {

        sleep(1);
        break;
    }

    syslog(LOG_NOTICE, "The da_daemon terminated.");

    closelog();
    
}

int main(){

    skeleton_daemon();

    int error = run_daemon();

    if(error)
        printf("Failed to start da_daemon, error code: %d\n", error);  

    return 0;
}

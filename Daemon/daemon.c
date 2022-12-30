#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int SocketFD;
struct sockaddr_in sa;

void create_socket()
{
    SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (SocketFD == -1) {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
  
    memset(&sa, 0, sizeof sa);
  
    sa.sin_family = AF_INET;
    sa.sin_port = htons(1100);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("adresa este: %d",sa.sin_addr.s_addr);
  
    if (bind(SocketFD,(struct sockaddr *)&sa, sizeof sa) == -1) {
      perror("bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
  
    if (listen(SocketFD, 10) == -1) {
      perror("listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
}

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

        int ConnectFD = accept(SocketFD, NULL, NULL);

        if (ConnectFD == -1) {
            perror("accept failed");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }

        char buff[10]="";
        int size = 5;
        read(ConnectFD, buff, size);

        if(size != 0)
        {
            syslog(LOG_NOTICE, "Daemon received %s",buff);
        }
        

        if (shutdown(ConnectFD, SHUT_RDWR) == -1) {
            perror("shutdown failed");
            close(ConnectFD);
            close(SocketFD);
            exit(EXIT_FAILURE);
        }
        close(ConnectFD);
    }

    syslog(LOG_NOTICE, "The da_daemon terminated.");

    closelog();
    
}

int main(){

    skeleton_daemon();
    create_socket();

    int error = run_daemon();

    if(error)
        printf("Failed to start da_daemon, error code: %d\n", error);  

    return 0;
}

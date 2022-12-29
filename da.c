#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <unistd.h>

#define ADD 1
#define SUSPEND 2
#define RESUME 3
#define REMOVE 4
#define INFO 5
#define LIST 6
#define PRINT 7

int check_error_strtol(const char* nptr, char *endptr, long int n){
    if(nptr == endptr)
        return 1;
    else if(errno == ERANGE || errno == EINVAL)
        return 1;
    else if(errno != 0 && n == 0)
        return 1;
    
    return 0;
}

int is_option(const char *option, const char *str1, const char *str2){
	return strcmp(option, str1) == 0 || strcmp(option, str2) == 0;
}

int main(int argc, char **argv){

	if(argc == 1){
		printf("Invalid number of arguments.\n");
		return 0;
	}

	/// --------------- Add ---------------
	if(is_option(argv[1], "-a", "--add")){
		if(argc < 3){
			printf("Invalid number of arguments.\n");
			return -1;
		}

		int priority = 1;
		char *path = argv[2];

		if(argc == 5 && is_option(argv[3], "-p", "--priority")){
			const char* nptr = argv[4];
            char *endptr = NULL;
            priority = strtol(nptr, &endptr, 10);

            if(check_error_strtol(nptr, endptr, priority)){
                printf("Invalid number.\n");
                return -1;
            }

			if(!(1 <= priority  && priority <= 3)){
				printf("Priority out of valid range.\n");
				return -1;
			}
		}



		return 0;
	}

	/// --------------- Suspend ---------------
	if(is_option(argv[1], "-S", "--suspend")){
		if(argc != 3){
			printf("Invalid number of arguments.\n");
			return -1;
		}

		const char* nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if(check_error_strtol(nptr, endptr, pid) || pid < 1){
			printf("Invalid number.\n");
			return -1;
		}
		


		return 0;
	}

	/// --------------- Resume ---------------
	if(is_option(argv[1], "-R", "--resume")){
		if(argc != 3){
			printf("Invalid number of arguments.\n");
			return -1;
		}

		const char* nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if(check_error_strtol(nptr, endptr, pid) || pid < 1){
			printf("Invalid number.\n");
			return -1;
		}

		

		return 0;
	}

	/// --------------- Remove ---------------
	if(is_option(argv[1], "-r", "--remove")){
		if(argc != 3){
			printf("Invalid number of arguments.\n");
			return -1;
		}

		const char* nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if(check_error_strtol(nptr, endptr, pid) || pid < 1){
			printf("Invalid number.\n");
			return -1;
		}

		

		return 0;
	}

	/// --------------- Info ---------------
	if(is_option(argv[1], "-i", "--info")){
		if(argc != 3){
			printf("Invalid number of arguments.\n");
			return -1;
		}

		const char* nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if(check_error_strtol(nptr, endptr, pid) || pid < 1){
			printf("Invalid number.\n");
			return -1;
		}
		


		return 0;
	}

	/// --------------- List ---------------
	if(is_option(argv[1], "-l", "--list")){
		if(argc != 2){
			printf("Invalid command.\n");
			return -1;
		}

		

		return 0;
	}

	/// --------------- Print ---------------
	if(is_option(argv[1], "-p", "--print")){
		if(argc != 3) {
			printf("Invalid number of arguments.\n");
			return -1;
		}
		
		const char* nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if(check_error_strtol(nptr, endptr, pid) || pid < 1){
			printf("Invalid number.\n");
			return -1;
		}
		
		

		return 0;
	}

	/// --------------- Terminate ---------------
	if(is_option(argv[1], "-t", "--terminate")){
		if(argc != 2){
			printf("Invalid command.\n");
			return -1;
		}



		return 0;
	}

	/// --------------- Helper ---------------
	if(is_option(argv[1], "-h", "--help")){
		printf("Usage: da [OPTION]... [DIR]...\n"
			   "Analyze the space occupied by the directory at [DIR]\n\n"
			   "-a, --add           analyze a new directory path for disk usage\n"
			   "-p, --priority      set priority for the new analysis (works only with -a argument)\n"
			   "-S, --suspend <id>  suspend task with <id>\n"
			   "-R, --resume <id>   resume task with <id>\n"
			   "-r, --remove <id>   remove the analysis with the given <id>\n"
			   "-i, --info <id>     print status about the analysis with <id> (pending, progress, d\n"
			   "-l, --list          list all analysis tasks, with their ID and the corresponding root p\n"
			   "-p, --print <id>    print analysis report for those tasks that are \"done\"\n"
			   "-t, --terminate     terminates daemon\n\n"
		);
		return 0;
	}

	printf("Invalid command.\n");
	return 0;

}

#include "da.h"

int SocketFD, returnFD;
struct sockaddr_in sa, return_sa;

void create_return_socket()
{
    returnFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (returnFD == -1)
    {
        perror("cannot create return socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(returnFD, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) == -1)
    {
		perror("cannot reuse address socket");
        exit(EXIT_FAILURE);
    }

    memset(&return_sa, 0, sizeof return_sa);

    return_sa.sin_family = AF_INET;
    return_sa.sin_port = htons(1200);
    return_sa.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(returnFD, (struct sockaddr *)&return_sa, sizeof return_sa) == -1)
    {
        perror("bind failed");
        close(returnFD);
        exit(EXIT_FAILURE);
    }

    if (listen(returnFD, 10) == -1)
    {
		perror("listen failed");
        close(returnFD);
        exit(EXIT_FAILURE);
    }


}

void open_socket()
{
	SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (SocketFD == -1)
	{
		perror("cannot open socket");
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof sa);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(1100);
	sa.sin_addr.s_addr = 0;
	// printf("done port\n");

	if (connect(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1)
	{
		printf("connect failed\n");
		perror("connect failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	// printf("done connect\n");
}

void close_socket()
{
	close(SocketFD);
	close(returnFD);
}

int check_error_strtol(const char *nptr, char *endptr, long int n)
{
	if (nptr == endptr)
		return 1;
	else if (errno == ERANGE || errno == EINVAL)
		return 1;
	else if (errno != 0 && n == 0)
		return 1;

	return 0;
}

int is_option(const char *option, const char *str1, const char *str2)
{
	return strcmp(option, str1) == 0 || strcmp(option, str2) == 0;
}

void send_request(const char *const buff, const int size)
{
	// send instructions to daemon

	write(SocketFD, buff, size);
}

void print_daemon_response()
{
	// da reads response from Daemon here

	int ConnectFD = accept(returnFD, NULL, NULL);

	if (ConnectFD == -1)
	{
		perror("accept failed");
		close_socket();
		exit(EXIT_FAILURE);
	}

	char response[1024]="";

	int nr_read = read(ConnectFD, response, 1024);

	if(nr_read < 1)
	{
		perror("return read failed");
		close_socket();
		exit(EXIT_FAILURE);
	}

	printf("%s", response);

	if(shutdown(ConnectFD, SHUT_RDWR) == -1)
	{
		close(ConnectFD);
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	close(ConnectFD);
}

void print_tmp(int task_id)
{
	char print_command[MAX_PATH_LENGTH];
    snprintf(print_command, MAX_PATH_LENGTH, "tac /tmp/da_daemon/%d", task_id);

	system(print_command);
}

void print_task_response(int task_id)
{
	
	int ConnectFD = accept(returnFD, NULL, NULL);

	if (ConnectFD == -1)
	{
		perror("accept failed");
		close_socket();
		exit(EXIT_FAILURE);
	}

	char response[1024]="";

	int nr_read = read(ConnectFD, response, 1024);

	if(nr_read < 1)
	{
		perror("return read failed");
		close_socket();
		exit(EXIT_FAILURE);
	}

	char *endptr = NULL;
	int done = strtol(response, &endptr, 10);

	if (check_error_strtol(response, endptr, done))
	{
		printf("Invalid response for print task\n");
		close_socket();
		//printf("closed socket\n");
		return;
	}

	if(done == 0)
		printf("Analysis task with ID '%d' is not done yet\n", task_id);
	else
		print_tmp(task_id);
}

int main(int argc, char **argv)
{
	create_return_socket();
	open_socket();
	
	// printf("opened socket\n");

	if (argc == 1)
	{
		printf("Invalid number of arguments.\n");
		close_socket();
		//printf("closed socket\n");
		return 0;
	}

	char instructions[512];

	/// --------------- Add ---------------
	if (is_option(argv[1], "-a", "--add"))
	{
		if (argc < 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		int priority = 1;
		char *path = argv[2];

		if (argc == 5 && is_option(argv[3], "-p", "--priority"))
		{
			const char *nptr = argv[4];
			char *endptr = NULL;
			priority = strtol(nptr, &endptr, 10);

			if (check_error_strtol(nptr, endptr, priority))
			{
				printf("Invalid number.\n");
				close_socket();
				//printf("closed socket\n");
				return -1;
			}

			if (!(1 <= priority && priority <= 3))
			{
				printf("Priority out of valid range.\n");
				close_socket();
				//printf("closed socket\n");
				return -1;
			}
		}

		sprintf(instructions, "ID %d\nPRIORITY %d\nPATH %s\n", ADD, priority, path);
		send_request(instructions, strlen(instructions));

		print_daemon_response();

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- Suspend ---------------
	if (is_option(argv[1], "-S", "--suspend"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 0)
		{
			printf("Invalid number.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", SUSPEND, pid);
		send_request(instructions, strlen(instructions));

		print_daemon_response();

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- Resume ---------------
	if (is_option(argv[1], "-R", "--resume"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 0)
		{
			printf("Invalid number.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", RESUME, pid);
		send_request(instructions, strlen(instructions));

		print_daemon_response();

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- Remove ---------------
	if (is_option(argv[1], "-r", "--remove"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 0)
		{
			printf("Invalid number.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", REMOVE, pid);
		send_request(instructions, strlen(instructions));

		print_daemon_response();

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- Info ---------------
	if (is_option(argv[1], "-i", "--info"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 0)
		{
			printf("Invalid number.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", INFO, pid);
		send_request(instructions, strlen(instructions));

		print_daemon_response();

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- List ---------------
	if (is_option(argv[1], "-l", "--list"))
	{
		if (argc != 2)
		{
			printf("Invalid command.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\n", LIST);
		send_request(instructions, strlen(instructions));

		print_daemon_response();

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- Print ---------------
	if (is_option(argv[1], "-p", "--print"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 0)
		{
			printf("Invalid number.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", PRINT, pid);
		send_request(instructions, strlen(instructions));

		print_task_response(pid);

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- Terminate ---------------
	if (is_option(argv[1], "-t", "--terminate"))
	{
		if (argc != 2)
		{
			printf("Invalid command.\n");
			close_socket();
			//printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\n", TERMINATE);
		send_request(instructions, strlen(instructions));

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	/// --------------- Helper ---------------
	if (is_option(argv[1], "-h", "--help"))
	{
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
			   "-t, --terminate     terminates daemon\n\n");

		close_socket();
		//printf("closed socket\n");

		return 0;
	}

	printf("Invalid command.\n");

	close_socket();
	//printf("closed socket\n");

	return 0;
}

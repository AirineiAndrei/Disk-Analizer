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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Shared/shared.h"

struct sockaddr_in sa;
int SocketFD;

void open_socket()
{
	SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (SocketFD == -1)
	{
		perror("cannot create socket");
		exit(EXIT_FAILURE);
	}

	memset(&sa, 0, sizeof sa);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(1100);
	sa.sin_addr.s_addr = 0;
	printf("done port\n");

	if (connect(SocketFD, (struct sockaddr *)&sa, sizeof sa) == -1)
	{
		printf("connect failed\n");
		perror("connect failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}

	printf("done connect\n");
}

void close_socket()
{
	close(SocketFD);
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

int main(int argc, char **argv)
{

	open_socket(); // TODO close socket
	printf("opened socket\n");

	if (argc == 1)
	{
		printf("Invalid number of arguments.\n");
		close_socket();
		printf("closed socket\n");
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
			printf("closed socket\n");
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
				printf("closed socket\n");
				return -1;
			}

			if (!(1 <= priority && priority <= 3))
			{
				printf("Priority out of valid range.\n");
				close_socket();
				printf("closed socket\n");
				return -1;
			}
		}

		sprintf(instructions, "ID %d\nPRIORITY %d\nPATH %s\n", ADD, priority, path);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

		return 0;
	}

	/// --------------- Suspend ---------------
	if (is_option(argv[1], "-S", "--suspend"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 1)
		{
			printf("Invalid number.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", SUSPEND, pid);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

		return 0;
	}

	/// --------------- Resume ---------------
	if (is_option(argv[1], "-R", "--resume"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 1)
		{
			printf("Invalid number.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", RESUME, pid);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

		return 0;
	}

	/// --------------- Remove ---------------
	if (is_option(argv[1], "-r", "--remove"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 1)
		{
			printf("Invalid number.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", REMOVE, pid);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

		return 0;
	}

	/// --------------- Info ---------------
	if (is_option(argv[1], "-i", "--info"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 1)
		{
			printf("Invalid number.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", INFO, pid);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

		return 0;
	}

	/// --------------- List ---------------
	if (is_option(argv[1], "-l", "--list"))
	{
		if (argc != 2)
		{
			printf("Invalid command.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\n", LIST);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

		return 0;
	}

	/// --------------- Print ---------------
	if (is_option(argv[1], "-p", "--print"))
	{
		if (argc != 3)
		{
			printf("Invalid number of arguments.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		const char *nptr = argv[2];
		char *endptr = NULL;
		int pid = strtol(nptr, &endptr, 10);

		if (check_error_strtol(nptr, endptr, pid) || pid < 1)
		{
			printf("Invalid number.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\nPID %d\n", PRINT, pid);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

		return 0;
	}

	/// --------------- Terminate ---------------
	if (is_option(argv[1], "-t", "--terminate"))
	{
		if (argc != 2)
		{
			printf("Invalid command.\n");
			close_socket();
			printf("closed socket\n");
			return -1;
		}

		sprintf(instructions, "ID %d\n", TERMINATE);
		send_request(instructions, strlen(instructions));

		close_socket();
		printf("closed socket\n");

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
		printf("closed socket\n");

		return 0;
	}

	printf("Invalid command.\n");

	close_socket();
	printf("closed socket\n");

	return 0;
}

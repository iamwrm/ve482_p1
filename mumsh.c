#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void clear_buffer(char* line, char** argv)
{
	*line = '\0';
	int position = 0;
	while (argv[position] != NULL) {
		argv[position] = NULL;
		position++;
	}
}

int parse_cmd(char* line, char** argv)
{
	int position = 0;
	char* arg;
	if (!argv) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	arg = strtok(line, " \t\r\n\a");

	while (arg != NULL) {
		argv[position] = arg;
		position++;
		arg = strtok(NULL, " \t\r\n\a");
	}
	argv[position] = '\0';
	return 0;
}

// return if_esc
int process_cmd(char** argv, char* line)
{
	// size_t size = strlen(line_input);
	int size = 1;
	int i = 0;
	if (argv[0] == NULL) {
		return 0;
	}
	if (strcmp(argv[0], "exit") == 0) {
		free(line);
		exit(0);
	}

	// =================
	pid_t pid;
	int status;

	pid = fork();

	if (pid == 0) {  // child
		if (execvp(*argv, argv) < 0) {
			printf("system func failed\n");
			exit(1);
		}
	} else {  // parent
		while (wait(&status) != pid) {
		}
	}
	// =================
	return 0;
}

int main()
{
	int bufsize = 1024;
	char* line = malloc(sizeof(char) * bufsize);
	// size_t len = 0;
	size_t capacity = 1024;
	char argv[64][1024];

	int if_esc = 0;

	char* sh_name = "mumsh $ ";
	while (1) {
		printf("%s", sh_name);
		getline(&line, &capacity, stdin);

		parse_cmd(line, argv);
		if_esc = process_cmd(argv, line);

		clear_buffer(line, argv);

		if (if_esc) {
			break;
		}
	}

	free(line);

	return 0;
}

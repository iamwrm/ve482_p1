#include "mainlib.h"


int parse_cmd(char* line, char** argv, struct Cmd_status* cmd_io_status)
{
	int position = 0;

	int later = 0;
	// arrow_sep(line);

	// count_real_pipe(line, cmd_io_status);

	char* arg;

	if (!argv) {
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}

	char* sep_er = " \t\n";
	// char* sep_er = " \t\r\n\a";
	arg = strtok(line, sep_er);

	while (arg != NULL) {
		argv[position] = arg;
		position++;
		arg = strtok(NULL, sep_er);
	}

	argv[position] = NULL;

	// printf("pipnum: %d", cmd_io_status->pipe_number);
	int gg = 0;
	while (argv[gg] != NULL) {
		printf("█%s", argv[gg]);
		gg++;
	}
	printf("pipe num%d\n", cmd_io_status->pipe_number);
	return 0;
}
#include "mainlib.h"

void parse_cmd_insert_sep(char* line, struct Cmd_status* cmd_io_status,
			  char full_block)
{
	cmd_io_status->pipe_number = 0;
	cmd_io_status->init_pipe_number = 0;
	int i = 0;
	while (line[i] != '\0') {
		if (line[i] == ' ') {
			line[i] = full_block;
			i++;
			continue;
		}
		if (line[i] == '<') {
			insert_blank(line, i);
			insert_blank(line, i + 2);
			line[i] = full_block;
			line[i + 2] = full_block;
			i = i + 2;
			i++;
			continue;
		}
		i++;
	}
}

int parse_cmd(char* line, char** argv, struct Cmd_status* cmd_io_status)
{
	char full_block = '[';
	int position = 0;

	// int later = 0;

	cmd_io_status->pipe_number = 0;
	cmd_io_status->init_pipe_number = 0;

	arrow_sep(line);

	count_real_pipe(line, cmd_io_status);

	// parse_cmd_insert_sep(line, cmd_io_status, full_block);

	char* arg;

	if (!argv) {
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}

	// char sep_er[20];

	// char* sep_er = "[";
	char* sep_er = " \t\r\n\a";
	arg = strtok(line, sep_er);

	while (arg != NULL) {
		argv[position] = arg;
		position++;
		arg = strtok(NULL, sep_er);
	}

	argv[position] = NULL;

	// printf("pipnum: %d", cmd_io_status->pipe_number);
	if (DEBUG_MODE) {
		int gg = 0;
		while (argv[gg] != NULL) {
			printf("%c%s", full_block, argv[gg]);
			gg++;
		}
		printf("pipe num%d\n", cmd_io_status->pipe_number);
	}
	return 0;
}

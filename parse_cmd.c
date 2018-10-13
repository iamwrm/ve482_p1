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
		if (line[i] == '>') {
			// case: a > b
			if ((line + i + 1 != NULL) && (line[i + 1] != '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 2);
				line[i] = full_block;
				line[i + 2] = full_block;
				i = i + 2;
				i++;
				continue;
			}
			// case: a>>b
			if ((line + i + 1 != NULL) && (line[i + 1] == '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 3);
				line[i] = full_block;
				line[i + 3] = full_block;
				i += 2;
			}
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

	/*
	arrow_sep(line);
	count_real_pipe(line, cmd_io_status);
	*/

	parse_cmd_insert_sep(line, cmd_io_status, full_block);

	char* arg;

	if (!argv) {
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}

	// char sep_er[20];

	char* sep_er = "[\n";
	// char* sep_er = " \t\r\n\a";
	arg = strtok(line, sep_er);

	while (arg != NULL) {
		argv[position] = arg;
		position++;
		arg = strtok(NULL, sep_er);
	}

	argv[position] = NULL;

	if (!DEBUG_MODE) {
		int gg = 0;
		while (argv[gg] != NULL) {
			printf("|%s", argv[gg]);
			gg++;
		}
		printf("\npipe num%d\n", cmd_io_status->pipe_number);
	}
	return 0;
}

void arrow_sep(char* line)
{
	int i = 0;
	while (line[i] != '\0') {
		if (line[i] == '<') {
			insert_blank(line, i);
			insert_blank(line, i + 2);
			i++;
		}
		if (line[i] == '|') {
			insert_blank(line, i);
			insert_blank(line, i + 2);
			i++;
		}
		if (line[i] == '>') {
			// case: a>b
			if ((line + i + 1 != NULL) && (line[i + 1] != '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 2);
				i++;
			}
			// case: a>>b
			if ((line + i + 1 != NULL) && (line[i + 1] == '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 3);
				i += 2;
			}
		}
		i++;
	}
}

void insert_blank(char* line, int pos)
{
	char temp[1024];
	strcpy(temp, line + pos);
	line[pos] = ' ';
	strcpy(line + pos + 1, temp);
}

void count_real_pipe(const char* line, struct Cmd_status* cmd_status)
{
	int pipe_count = 0;
	// TODO: provide support for quotes
	int i = 0;
	while (*(line + i++) != '\0') {
		if (line[i] == '|') {
			pipe_count++;
		}
	}
	cmd_status->pipe_number = pipe_count;
	cmd_status->init_pipe_number = pipe_count;
	return;
}

#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

int parse_cmd(char* line, char** argv, struct Cmd_status* cmd_io_status)
{
	int position = 0;

	arrow_sep(line);

	count_real_pipe(line, cmd_io_status);


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
	return;
}

int first_pipe_position(char** argv)
{
	int i = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], "|") == 0) {
			return i;
		}
		i++;
	}
	return -1;
}

void clear_buffer(char* line, char** argv)
{
	*line = '\0';
	int position = 0;
	while (argv[position] != NULL) {
		argv[position] = NULL;
		position++;
	}
}

int read_line(char* line_input, int line_length)
{
	int position = 0;
	int c;
	while (1) {
		c = getchar();

		if ((c == EOF) && (position == 0)) {
			return 0;
		}
		if ((isatty(fileno(stdin))) && (c == EOF) && (position > 0)) {
			continue;
		}

		// FIXED: by isatty()
		// 1. ./mumsh < task.sh
		// there are mumsh $ mumsh $mumsh $mumsh $ and exit
		// ================
		if (c == EOF || c == '\n') {
			// if (c == '\n') {
			line_input[position] = '\0';
			return 1;
		}

		// FIXED: echo ctrl-d ctrl-d will print ctrl-d
		// if (c == EOF) {
		// continue;
		//}

		line_input[position] = c;
		position++;
		assert(position < line_length);
	}
}


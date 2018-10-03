#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

int parse_cmd(char* line, char** argv)
{
	int position = 0;

	arrow_sep(line);
	char* arg;
	if (!argv) {
		fprintf(stderr, "lsh: allocation error\n");
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

void clear_buffer(char* line, char** argv)
{
	*line = '\0';
	int position = 0;
	while (argv[position] != NULL) {
		argv[position] = NULL;
		position++;
	}
}

void insert_blank(char* line, int pos)
{
	char temp[1024];
	strcpy(temp, line + pos);
	line[pos] = ' ';
	strcpy(line + pos + 1, temp);
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
		if (line[i] == '>') {
			if ((line + i + 1 != NULL) && (line[i + 1] != '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 2);
				i++;
			}
			if ((line + i + 1 != NULL) && (line[i + 1] == '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 3);
				i += 2;
			}
		}
		i++;
	}
}

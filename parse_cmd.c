#include "mainlib.h"

void my_strcpy(char* des, char* ori)
{
	int i = 0;
	while (ori[i] != '\0') {
		des[i] = ori[i];
		i++;
	}
	des[i] = '\0';
}

void parse_cmd_insert_sep(char* line, struct Cmd_status* cmd_io_status,
			  char full_block)
{
	cmd_io_status->pipe_number = 0;
	cmd_io_status->init_pipe_number = 0;
	int i = 0;
	int in_quote = 0;
	while (1) {
		if (line[i] == '\0') {
			if (!in_quote) {
				break;
			} else {
				char* templine = malloc(1024 * sizeof(char));
				printf("> ");
				read_line(templine);
				delete_char_at(line, strlen(line) - 1);
				strcat(line, "\n");
				strcat(line, templine);
				free(templine);
			}
		}
		if (in_quote > 0) {
			if ((in_quote == 2) && (line[i] == '"')) {
				in_quote = 0;
				delete_char_at(line, i);
				continue;
			}
			if ((in_quote == 1) && (line[i] == '\'')) {
				in_quote = 0;
				delete_char_at(line, i);
				continue;
			}
			i++;
			continue;

		} else {  // in_quote < 0
			if (line[i] == '"') {
				in_quote = 2;
				delete_char_at(line, i);
				continue;
			}
			if (line[i] == '\'') {
				in_quote = 1;
				delete_char_at(line, i);
				continue;
			}
			if (line[i] == ' ') {
				line[i] = full_block;
				i++;
				continue;
			}
			if (line[i] == '|') {
				insert_blank(line, i);
				insert_blank(line, i + 2);
				line[i] = full_block;
				line[i + 1] = ']';
				line[i + 2] = full_block;
				i = i + 2;
				cmd_io_status->pipe_number++;
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
				if ((line + i + 1 != NULL) &&
				    (line[i + 1] != '>')) {
					insert_blank(line, i);
					insert_blank(line, i + 2);
					line[i] = full_block;
					line[i + 2] = full_block;
					i = i + 2;
					i++;
					continue;
				}
				// case: a>>b
				if ((line + i + 1 != NULL) &&
				    (line[i + 1] == '>')) {
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
	delete_char_at(line, strlen(line) - 1);
	strcat(line, "\0");
	return;
}

int parse_cmd(char* line, char** argv, struct Cmd_status* cmd_io_status,
	      char* extra_space)
{
	char full_block = '[';

	int position = 0;

	cmd_io_status->pipe_number = 0;
	cmd_io_status->init_pipe_number = 0;

	parse_cmd_insert_sep(line, cmd_io_status, full_block);

	if (DEBUG_MODE) {
		printf("DEBUG:parsed line:%s\n", line);
	}

	char* arg;

	if (!argv) {
		fprintf(stderr, "allocation error\n");
		exit(EXIT_FAILURE);
	}

	// char sep_er[20];

	char* sep_er = "[";
	// char* sep_er = " \t\r\n\a";
	arg = strtok(line, sep_er);

	while (arg != NULL) {
		argv[position] = arg;
		position++;
		arg = strtok(NULL, sep_er);
	}

	argv[position] = NULL;

	if (position > 1) {
		if ((strcmp(argv[position - 1], "<") == 0) ||
		    (strcmp(argv[position - 1], ">") == 0)) {
			printf("> ");
			read_line(extra_space);
			arg = strtok(extra_space, " \n");

			argv[position] = arg;
			argv[position + 1] = NULL;
		}
	}

	if (DEBUG_MODE) {
		int gg = 0;
		printf("DEBUG:");
		while (argv[gg] != NULL) {
			printf("|%s", argv[gg]);
			gg++;
		}
		printf("\n");
		// printf("\npipe num%d\n", cmd_io_status->pipe_number);
	}
	return 1;
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

void delete_char_at(char* word, int idxToDel)
{
	memmove(&word[idxToDel], &word[idxToDel + 1], strlen(word) - idxToDel);
}

void print_argv(char** argv)
{
	int i = 0;
	printf("DEBUG:argv:");
	while (argv[i] != NULL) {
		printf("█%s", argv[i]);
		i++;
	}
	printf("\n");
	return;
}

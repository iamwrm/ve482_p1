#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

int my_execvp(char* cmdhead, char** cmd)
{
	if (DEBUG_MODE) {
		int gg = 0;
		while (cmd[gg] != NULL) {
			printf("_%s", cmd[gg]);
			gg++;
		}
		printf("%d", gg);
	}
	if (strcmp(cmd[0], "pwd") == 0) {
		char buf[1024];
		getcwd(buf, sizeof(buf));
		printf("%s\n", buf);
		fflush(stdin);
		fflush(stdout);
		exit(0);
		return 0;
	}

	return execvp(cmdhead, cmd);
}

int find_the_nth_pipe(char** argv, int n)
{
	int i = 0;
	int j = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], "|") == 0) {
			j++;
		}
		if (j == n) {
			return i;
		}
		i++;
	}
	return -1;
}

void dup_and_exc(struct Cmd_status* cmd_io_status, char** argv)
{
	pid_t pid_d;
	pid_d = fork();
	signal(SIGINT, process_sig_handler);
	if (pid_d == 0) {
		set_redirect_status(cmd_io_status, argv);

		if (my_execvp(*argv, argv)) {
			fprintf(stderr,
				"Error: no such file or "
				"directory\n");
			exit(0);
		}
		return;

	} else {  // parent
		wait(NULL);
	}
}

void set_redirect_status(struct Cmd_status* cmd_io_status, char** argv)
{
	cmd_io_status->i_redirected = 0;
	cmd_io_status->o_redirected = 0;

	find_redirect_symbols(argv, cmd_io_status);

	if (cmd_io_status->o_redirected == 1) {
		int outfile = open(cmd_io_status->temp_out_file_name,
				   FLAGS_WRITE, MODE_WR);

		dup2(outfile, STDOUT_FILENO);
	}
	if (cmd_io_status->o_redirected == 2) {
		int outfile = open(cmd_io_status->temp_out_file_name,
				   FLAG_APPEND, MODE_WR);

		dup2(outfile, STDOUT_FILENO);
	}
	if (cmd_io_status->i_redirected == 1) {
		int in_file =
		    open(cmd_io_status->temp_in_file_name, FLAG_READ, MODE_WR);

		dup2(in_file, STDIN_FILENO);
	}
}

// BEFORE: echo 123 > 1.txt
// AFTER : echo 123
// cmd_io_status->o_redirected are modified
// cmd_io_status->i_redirected are modified
// start from argv
// end till argv[i]==NULL
void find_redirect_symbols(char** argv, struct Cmd_status* cmd_io_status)
{
	int i = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], "|") == 0) {
			break;
		}
		if (strcmp(argv[i], ">") == 0) {
			cmd_io_status->o_redirected = 1;
			strcpy(cmd_io_status->temp_out_file_name, argv[i + 1]);
			*(argv[i]) = ' ';
			*(argv[i + 1]) = ' ';
			i++;
			continue;
		}
		if (strcmp(argv[i], ">>") == 0) {
			cmd_io_status->o_redirected = 2;
			strcpy(cmd_io_status->temp_out_file_name, argv[i + 1]);
			*(argv[i]) = ' ';
			*(argv[i + 1]) = ' ';
			i++;
			continue;
		}
		if (strcmp(argv[i], "<") == 0) {
			cmd_io_status->i_redirected = 1;
			strcpy(cmd_io_status->temp_in_file_name, argv[i + 1]);
			*(argv[i]) = ' ';
			*(argv[i + 1]) = ' ';
			i++;
			continue;
		}
		i++;
	}
	remove_blank_in_argv(argv);
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

int read_line(char* line_input)
{
	if (fgets(line_input, BUFF_SIZE, stdin) == NULL) {
		if (feof(stdin)) {
			printf("exit\n");
			fflush(stdout);
			exit(0);
		} else
			return 1;
	}
	return 0;
}

void remove_blank_in_argv(char** argv)
{
	char** temp_argv = malloc(64 * sizeof(char*));
	int temp_argv_i = 0;

	int new_position = 0;
	while (argv[new_position] != NULL) {
		if (*(argv[new_position]) == ' ') {
			new_position++;
			continue;
		}
		temp_argv[temp_argv_i] = argv[new_position];
		temp_argv_i++;

		new_position++;
	}
	temp_argv[temp_argv_i] = NULL;

	new_position = 0;
	while (temp_argv[new_position] != NULL) {
		argv[new_position] = temp_argv[new_position];
		new_position++;
	}
	argv[new_position] = NULL;
	free(temp_argv);
}

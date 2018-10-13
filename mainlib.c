#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

int my_execvp(char* cmdhead, char** cmd)
{
	int gg = 0;
	while (cmd[gg] != NULL) {
		printf("_%s", cmd[gg]);
		gg++;
	}
	printf("%d", gg);
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

// fds_1-> input , fds-> output
void cmd_mid(struct Cmd_status* cmd_io_status, char** cmd2, int* fds_1,
	     int* fds)
{
	dup2(fds_1[0], 0);
	close(fds_1[1]);
	dup2(fds[1], 1);
	close(fds[0]);
	set_redirect_status(cmd_io_status, cmd2);
	if (my_execvp(cmd2[0], cmd2)) {
		fprintf(stderr,
			"Error: no such file or "
			"directory\n");
		exit(EXIT_FAILURE);
	}
}

// fds_1-> output
void cmd_head(struct Cmd_status* cmd_io_status, char** cmd1, int* fds_1)
{
	dup2(fds_1[1], 1);
	close(fds_1[0]);
	set_redirect_status(cmd_io_status, cmd1);
	if (my_execvp(cmd1[0], cmd1)) {
		fprintf(stderr,
			"Error: no such file or "
			"directory\n");
		exit(EXIT_FAILURE);
	}
}

// fds-> input
void cmd_tail(struct Cmd_status* cmd_io_status, char** cmd3, int* fds)
{
	dup2(fds[0], 0);
	close(fds[1]);
	set_redirect_status(cmd_io_status, cmd3);
	if (my_execvp(cmd3[0], cmd3)) {
		fprintf(stderr,
			"Error: no such file or "
			"directory\n");
		exit(EXIT_FAILURE);
	}
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

void pipe_helper(char** argv, struct Cmd_status* cmd_io_status, int init_depth,
		 int depth, int* input_p)
{
	if (depth == init_depth) {
		pid_t pid_out;
		pid_out = fork();

		if (pid_out == 0) {
			int fileds_1[2];  // file descriptors
			pipe(fileds_1);
			pid_t pid_2;

			pid_2 = fork();
			if (pid_2 == 0) {
				// cmd1
				pipe_helper(argv, cmd_io_status, init_depth,
					    depth - 1, fileds_1);
			} else {
				// cmd2
				// TODO: make sure argv points right place
				// wait(NULL);
				int deviation = find_the_nth_pipe(argv, depth);
				cmd_tail(cmd_io_status, argv + deviation + 1,
					 fileds_1);
			}
		} else {
			wait(NULL);
		}
		return;
	}

	if (depth > 1) {
		int fileds_1[2];  // file descriptors
		pipe(fileds_1);
		pid_t pid_2;

		pid_2 = fork();
		if (pid_2 == 0) {
			// cmd1
			pipe_helper(argv, cmd_io_status, init_depth, depth - 1,
				    fileds_1);

		} else {
			// cmd2
			int deviation = find_the_nth_pipe(argv, depth);
			argv[find_the_nth_pipe(argv, depth + 1)] = NULL;
			cmd_mid(cmd_io_status, argv + deviation + 1, fileds_1,
				input_p);
			wait(NULL);
			return;
		}
	}

	if (depth == 1) {
		int fds_1[2];  // file descriptors
		pipe(fds_1);
		pid_t pid_2;
		pid_2 = fork();
		if (pid_2 == 0) {
			// cmd1
			argv[find_the_nth_pipe(argv, depth)] = NULL;
			cmd_head(cmd_io_status, argv, fds_1);
		} else {
			// cmd2
			argv[find_the_nth_pipe(argv, depth + 1)] = NULL;
			cmd_mid(cmd_io_status,
				argv + find_the_nth_pipe(argv, 1) + 1, fds_1,
				input_p);
			wait(NULL);
			return;
		}
	}
	return;
}

void pipe_command_3(char** argv, struct Cmd_status* cmd_io_status)
{
	char** cmd1 = argv;

	int i1 = first_pipe_position(cmd1);
	char** cmd2 = cmd1 + i1 + 1;
	cmd1[i1] = NULL;

	int i2 = first_pipe_position(cmd2);
	char** cmd3 = cmd2 + i2 + 1;
	cmd2[i2] = NULL;

	int i3 = first_pipe_position(cmd3);
	char** cmd4 = cmd3 + i3 + 1;
	cmd3[i3] = NULL;

	pid_t pid_0;
	pid_0 = fork();

	// else -> wait(NULL) the shell process
	if (pid_0 == 0) {
		int fds[2];  // file descriptors
		pipe(fds);
		pid_t pid_1;

		pid_1 = fork();

		if (pid_1 == 0) {
			int fds_1[2];  // file descriptors
			pipe(fds_1);
			pid_t pid_2;

			pid_2 = fork();
			if (pid_2 == 0) {
				// cmd1

				int fds_2[2];  // file descriptors
				pipe(fds_2);
				pid_t pid_3;

				pid_3 = fork();
				if (pid_3 == 0) {
					cmd_head(cmd_io_status, cmd1, fds_2);
				} else {
					cmd_mid(cmd_io_status, cmd2, fds_2,
						fds_1);
				}
			} else {
				// cmd2
				cmd_mid(cmd_io_status, cmd3, fds_1, fds);
			}
		} else {
			// cmd3
			cmd_tail(cmd_io_status, cmd4, fds);
		}
	} else {
		wait(NULL);
	}
	return;
}

void pipe_command(char** cmd1, char** cmd2, struct Cmd_status* cmd_io_status)
{
	pid_t pid_0;
	pid_0 = fork();

	if (pid_0 == 0) {
		int fds[2];  // file descriptors
		pipe(fds);
		pid_t pid;

		pid = fork();

		// former cmd
		if (pid == 0) {
			set_redirect_status(cmd_io_status, cmd1);
			dup2(fds[1], 1);
			close(fds[0]);
			if (my_execvp(cmd1[0], cmd1)) {
				fprintf(stderr,
					"Error: no such file or "
					"directory\n");
				exit(EXIT_FAILURE);
			}
			close(fds[1]);
		} else {
			// later cmd
			set_redirect_status(cmd_io_status, cmd2);
			dup2(fds[0], 0);
			close(fds[1]);
			if (my_execvp(cmd2[0], cmd2)) {
				fprintf(stderr,
					"Error: no such file or "
					"directory\n");
				exit(EXIT_FAILURE);
			}

			close(fds[0]);
		}
	} else {
		wait(NULL);
	}
	return;
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

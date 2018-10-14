#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif
int check_between_token(char** argv)
{
	int i = 0;
	int exist = 0;
	while (argv[i] != NULL) {
		if ((strcmp(argv[i], ">") == 0) ||
		    (strcmp(argv[i], "<") == 0) ||
		    (strcmp(argv[i], "]") == 0)) {
			if (!exist) {
				exist = 1;
			}
			if (exist) {
				exist = 0;
				if ((strcmp(argv[i - 1], ">") == 0) ||
				    (strcmp(argv[i - 1], "<") == 0) ||
				    (strcmp(argv[i - 1], "]") == 0)) {
					if (strcmp(argv[i], "]") == 0) {
						argv[i] = "|";
					}
					fprintf(stderr,
						"syntax error near unexpected "
						"token `%s'\n",
						argv[i]);
					return -1;
				}
			}
		}
		i++;
	}
	return 0;
}

int check_missing_program(char** argv)
{
	int i = 0;
	while (argv[i] != NULL) {
		if ((i > 1) && (strcmp(argv[i], "]") == 0)) {
			if (strcmp(argv[i - 1], argv[i]) == 0) {
				fprintf(stderr, "error: missing program\n");
				return -1;
			}
		}
		i++;
	}
	return 0;
}
void check_em_re(char** argv)
{
	int i = 0;
	int red_pos = 0;

	while (strcmp(argv[i], "]") == 0) {
		if (strcmp(argv[i], ">") == 0) {
			red_pos = i;
		}
		i++;
	}
	if (red_pos == i - 1) {
		fprintf(stderr, "syntax error near unexpected token '>'\n");
	}

	return;
}

int my_execvp(char* cmdhead, char** cmd)
{
	if (DEBUG_MODE) {
		int gg = 0;
		printf("DEBUG:");
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
		if (strcmp(argv[i], "]") == 0) {
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
	signal(SIGINT, process_sig_handler);
	pid_t pid_d;
	pid_d = fork();

	if (pid_d == 0) {
		check_em_re(argv);
		check_du_re(argv);
		set_redirect_status(cmd_io_status, argv);

		if (my_execvp(*argv, argv)) {
			fprintf(stderr, "%s: command not found\n", argv[0]);
			exit(0);
		}
		return;

	} else {  // parent
		if (WAIT_IN_PARENT) {
			wait(NULL);
		}
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
		if ((outfile == -1) && (1)) {
			printf("%s: Permission denied\n",
			       cmd_io_status->temp_out_file_name);
			exit(0);
		}
		dup2(outfile, STDOUT_FILENO);
	}
	if (cmd_io_status->o_redirected == 2) {
		int outfile = open(cmd_io_status->temp_out_file_name,
				   FLAG_APPEND, MODE_WR);
		if ((outfile == -1) && (1)) {
			printf("%s: Permission denied\n",
			       cmd_io_status->temp_out_file_name);
			exit(0);
		}
		dup2(outfile, STDOUT_FILENO);
	}
	if (cmd_io_status->i_redirected == 1) {
		int in_file =
		    open(cmd_io_status->temp_in_file_name, FLAG_READ, MODE_WR);
		if ((in_file == -1) && (errno == 1)) {
			printf("%s: Permission denied\n",
			       cmd_io_status->temp_in_file_name);
			exit(0);
		}
		if ((in_file == -1) && (errno == 2)) {
			printf("%s: No such file or directory\n",
			       cmd_io_status->temp_in_file_name);
			exit(0);
		}

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
		if (strcmp(argv[i], "]") == 0) {
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

int first_pipe_position(char** argv)
{
	int i = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], "]") == 0) {
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

int check_du_re(char** argv)
{
	int i = 0;
	int out_r = 0;
	int in_r = 0;

	while (argv[i] != NULL) {
		if (strcmp(argv[i], ">") == 0) {
			out_r++;
			i++;
			continue;
		}
		if (strcmp(argv[i], ">>") == 0) {
			out_r++;
			i++;
			continue;
		}
		if (strcmp(argv[i], "<") == 0) {
			in_r++;
			i++;
			continue;
		}
		i++;
	}
	if (out_r > 1) {
		fprintf(stderr, "error: duplicated output redirection\n");
		exit(0);
		return 1;
	}
	if (in_r > 1) {
		fprintf(stderr, "error: duplicated input redirection\n");
		exit(0);
		return 1;
	}
	return 0;
}

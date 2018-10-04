#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

void dup_and_exc(struct Cmd_status* cmd_io_status, pid_t pid, char** argv,
		 int status, int* pipefd)
{
	if (pid == 0) {
		// child
		if ((cmd_io_status->init_pipe_number > 0) &&
		    (cmd_io_status->init_pipe_number ==
		     cmd_io_status->pipe_number)) {
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[0]);
		}
		if ((cmd_io_status->init_pipe_number > 0) &&
		    (cmd_io_status->init_pipe_number >
		     cmd_io_status->pipe_number) &&
		    (cmd_io_status->pipe_number > 0)) {
			dup2(pipefd[1], STDOUT_FILENO);
			dup2(pipefd[0], STDIN_FILENO);
		}
		if ((cmd_io_status->init_pipe_number > 0) &&
		    (0 == cmd_io_status->pipe_number)) {
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[1]);
		}

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
			int in_file = open(cmd_io_status->temp_in_file_name,
					   FLAG_READ, MODE_WR);

			dup2(in_file, STDIN_FILENO);
		}

		int exe_return_value = execvp(*argv, argv);

		if (exe_return_value < 0) {
			fprintf(stderr,
				"Error: no such file or "
				"directory\n");
			exit(0);
		}

	} else {  // parent
		while (wait(&status) != pid) {
		}
	}
}

void find_redirect_symbols(char** argv, struct Cmd_status* cmd_io_status)
{
	int i = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], "|") == 0) {
			break;
		}
		if ((strcmp(argv[i], ">") == 0) ||
		    (strcmp(argv[i], ">>") == 0)) {
			cmd_io_status->o_redirected = 1;
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

// return if_esc
int process_cmd(char** argv, struct Cmd_status* cmd_io_status)
{
	int pipefd[2];
	pipe(pipefd);

	while (1) {
		if (argv[0] == NULL) {
			return 0;
		}
		if (strcmp(argv[0], "exit") == 0) {
			return 1;
		}

		cmd_io_status->i_redirected = 0;
		cmd_io_status->o_redirected = 0;

		find_redirect_symbols(argv, cmd_io_status);

		pid_t pid;
		int status = 0;

		pid = fork();

		if (cmd_io_status->pipe_number > 0) {
			int fpp = first_pipe_position(argv);
			argv[fpp] = NULL;
			dup_and_exc(cmd_io_status, pid, argv, status, pipefd);
			argv = argv + fpp + 1;
			cmd_io_status->pipe_number--;
			continue;
		} else {
			dup_and_exc(cmd_io_status, pid, argv, status, pipefd);
			break;
		}
	}  // while (1)
	return 0;
}

int main()
{
	int bufsize = 1024;
	char* line = malloc(sizeof(char) * bufsize);

	int arg_num = 128;
	int arg_length = 1024;
	char* arg = malloc(arg_num * (arg_length + 1) * sizeof(char));
	char** argv;
	argv = malloc(arg_num * sizeof(char*));
	for (int i = 0; i < arg_num; i++) {
		argv[i] = arg + i * (arg_length + 1) * sizeof(char);
	}

	char* sh_name = "mumsh $ ";
	if (isatty(fileno(stdin))) {
		printf("%s", sh_name);
		fflush(stdout);
		fflush(stderr);
	}

	struct Cmd_status cmd_io_status;
	cmd_io_status.temp_in_file_name = malloc(1024 * sizeof(char));
	cmd_io_status.temp_out_file_name = malloc(1024 * sizeof(char));

	while (read_line(line, bufsize)) {
		parse_cmd(line, argv, &cmd_io_status);

		if (process_cmd(argv, &cmd_io_status)) {
			break;
		}

		if (isatty(fileno(stdin))) {
			printf("%s", sh_name);
			fflush(stdout);
			fflush(stderr);
		}
	}

	if (isatty(fileno(stdin))) {
		printf("exit\n");
		fflush(stdout);
		fflush(stderr);
	}

	free(cmd_io_status.temp_in_file_name);
	free(cmd_io_status.temp_out_file_name);
	free(arg);
	free(argv);
	free(line);

	return 0;
}

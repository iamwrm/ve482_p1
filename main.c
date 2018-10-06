#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

int my_execvp(char* cmdhead, char** cmd)
{
	return execvp(cmdhead, cmd);
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
				set_redirect_status(cmd_io_status, cmd1);
				dup2(fds_1[1], 1);
				close(fds_1[0]);
				if (my_execvp(cmd1[0], cmd1)) {
					fprintf(stderr,
						"Error: no such file or "
						"directory\n");
					exit(EXIT_FAILURE);
				}
			} else {
				set_redirect_status(cmd_io_status, cmd2);

				dup2(fds_1[0], 0);
				close(fds_1[1]);

				dup2(fds[1], 1);
				close(fds[0]);
				if (my_execvp(cmd2[0], cmd2)) {
					fprintf(stderr,
						"Error: no such file or "
						"directory\n");
					exit(EXIT_FAILURE);
				}
			}
		} else {
			// later cmd
			set_redirect_status(cmd_io_status, cmd3);
			dup2(fds[0], 0);
			close(fds[1]);
			if (my_execvp(cmd2[0], cmd2)) {
				fprintf(stderr,
					"Error: no such file or "
					"directory\n");
				exit(EXIT_FAILURE);
			}
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
		}
	} else {
		wait(NULL);
	}
	return;
}

void dup_and_exc(struct Cmd_status* cmd_io_status, char** argv)
{
	pid_t pid_d;
	int status_d = 0;

	pid_d = fork();
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

// return if_esc
int process_cmd(char** argv, struct Cmd_status* cmd_io_status)
{
	if (argv[0] == NULL) {
		return 0;
	}
	if (strcmp(argv[0], "exit") == 0) {
		return 1;
	}

	if (cmd_io_status->pipe_number > 0) {
		// int fpp = first_pipe_position(argv);
		// argv[fpp] = NULL;
		// pipe_command(argv, argv + fpp + 1, cmd_io_status);
		pipe_command_3(argv, cmd_io_status);

		// continue;
	} else {
		dup_and_exc(cmd_io_status, argv);
		// break;
	}
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
	printf("%s", sh_name);
	fflush(stdout);
	fflush(stderr);

	struct Cmd_status cmd_io_status;
	cmd_io_status.temp_in_file_name = malloc(1024 * sizeof(char));
	cmd_io_status.temp_out_file_name = malloc(1024 * sizeof(char));

	while (read_line(line, bufsize)) {
		parse_cmd(line, argv, &cmd_io_status);

		if (process_cmd(argv, &cmd_io_status)) {
			break;
		}
		printf("%s", sh_name);
		fflush(stdout);
		fflush(stderr);
	}

	printf("exit\n");
	fflush(stdout);
	fflush(stderr);

	free(cmd_io_status.temp_in_file_name);
	free(cmd_io_status.temp_out_file_name);
	free(arg);
	free(argv);
	free(line);

	return 0;
}

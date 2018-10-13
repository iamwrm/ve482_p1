#include "mainlib.h"

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



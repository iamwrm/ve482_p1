#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

void new_set_redirect_status(struct Cmd_status* cmd_io_status, char** argv)
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
void process_sig_handler(int sig)
{
	if (sig == SIGINT) {
		printf("\n");
		signal(SIGINT, process_sig_handler);
	}
}

pid_t fork_and_exec(struct Cmd_status* cmd_io_status, char** argv, int in,
		    int out)
{

	pid_t new_pid;
	new_pid = fork();
	if (new_pid == -1) {
		fprintf(stderr, "Fork error\n");
	}

	if (new_pid == 0) {
		// child
		set_redirect_status(cmd_io_status, argv);
		my_execvp(argv[0], argv);
	} else {
		// parent
		close(in);
		close(out);
	}
	return new_pid;
}

void iterate_pipe_helper(char** argv, struct Cmd_status* cmd_io_status)
{
	int pipe_num = cmd_io_status->pipe_number + 1;

	pid_t* pid_array = malloc((pipe_num + 1) * sizeof(pid_t));
	for (int i = 0; i < pipe_num; i++) {
		pid_array[i] = 0;
	}
	int* pipe_arg_position = malloc((pipe_num + 1) * sizeof(int));
	for (int i = 0; i < pipe_num; i++) {
		pipe_arg_position[i] = find_the_nth_pipe(argv, i + 1);
	}
	for (int i = 0; i < pipe_num; i++) {
		if (pipe_arg_position[i] > 0) {
			argv[pipe_arg_position[i]] = NULL;
		}
	}

	int prev = 0;

	for (int i = 0; i < pipe_num; i++) {
		int pipe_df[2];
		pipe(pipe_df);

		int deviation = 0;
		if (i > 0) {
			deviation = pipe_arg_position[i - 1] + 1;
		}

		int in = prev;
		int out = 0;
		if (i == 0) {
			in = -1;
		}
		if (i == pipe_num - 1) {
			out = -1;
		} else {
			out = pipe_df[1];
		}

		int iiii = 0;

		pid_array[i] =
		    fork_and_exec(cmd_io_status, argv + deviation , in, out);

		prev = pipe_df[0];
	}

	int status;
	// TODO: waitpid
	for (int i = 0; i < pipe_num; i++) {
		if (pid_array[i] > 0) {
			waitpid(pid_array[i], &status, 0);
		}
	}

	free(pipe_arg_position);
	free(pid_array);
	return;
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
	if (strcmp(argv[0], "cd") == 0) {
		if (chdir(argv[1]) != 0) {
			printf(
			    "mumsh: cd: %s: No such file or "
			    "directory\n",
			    argv[1]);
			return 0;
		}
		fflush(stdout);
		return 0;
	}
	// signal(SIGINT, psig_handler);
	if (cmd_io_status->pipe_number > 3) {
		/*
		pipe_helper(argv, cmd_io_status,
			    cmd_io_status->init_pipe_number,
			    cmd_io_status->init_pipe_number, NULL);
		 */
		// printf("branch i want");
		iterate_pipe_helper(argv, cmd_io_status);
	} else if (cmd_io_status->pipe_number > 0) {
		// int fpp = first_pipe_position(argv);
		// argv[fpp] = NULL;
		// pipe_command(argv, argv + fpp + 1, cmd_io_status);
		pipe_command_3(argv, cmd_io_status);
		// cmd_io_status);

		// pipe_helper(argv, cmd_io_status,
		//    cmd_io_status->init_pipe_number,
		//   cmd_io_status->init_pipe_number, NULL);
		// new_pipe_cmd(argv, cmd_io_status);
	} else {
		dup_and_exc(cmd_io_status, argv);
		// break;
	}

	return 0;
}

int main()
{
	char line[1024];

	//	signal(SIGINT, SIG_IGN);

	int arg_num = 128;
	int arg_length = 1024;
	char* arg = malloc(arg_num * (arg_length + 1) * sizeof(char));
	char** argv;
	argv = malloc(arg_num * sizeof(char*));
	for (int i = 0; i < arg_num; i++) {
		argv[i] = arg + i * (arg_length + 1) * sizeof(char);
	}

	char* sh_name = "mumsh $ ";

	struct Cmd_status cmd_io_status;
	cmd_io_status.temp_in_file_name = malloc(1024 * sizeof(char));
	cmd_io_status.temp_out_file_name = malloc(1024 * sizeof(char));

	while (1) {
		signal(SIGINT, process_sig_handler);
		printf("%s", sh_name);
		fflush(stdout);
		fflush(stderr);
		if (read_line(line)) {
			continue;
		}
		parse_cmd(line, argv, &cmd_io_status);

		if (process_cmd(argv, &cmd_io_status)) {
			break;
		}
	}

	printf("exit\n");
	fflush(stdout);
	fflush(stderr);

	free(cmd_io_status.temp_in_file_name);
	free(cmd_io_status.temp_out_file_name);
	free(arg);
	free(argv);

	return 0;
}

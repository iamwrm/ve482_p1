#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif


void process_sig_handler(int sig)
{
	if (sig == SIGINT) {
		printf("\n");
		signal(SIGINT, process_sig_handler);
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

	if (cmd_io_status->pipe_number > 0) {
		// int fpp = first_pipe_position(argv);
		// argv[fpp] = NULL;
		// pipe_command(argv, argv + fpp + 1, cmd_io_status);
		pipe_command_3(argv, cmd_io_status);
		//	     cmd_io_status);
		// pipe_helper(argv, cmd_io_status,
		//    cmd_io_status->init_pipe_number,
		//   cmd_io_status->init_pipe_number, NULL);
		// new_pipe_cmd(argv, cmd_io_status);
	} else if (cmd_io_status->pipe_number > 3) {
		pipe_helper(argv, cmd_io_status,
			    cmd_io_status->init_pipe_number,
			    cmd_io_status->init_pipe_number, NULL);
	} else {
		dup_and_exc(cmd_io_status, argv);
		// break;
	}

	return 0;
}

int main()
{
	int bufsize = 1024;
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
		if (read_line(line, bufsize)) {
			continue;
		}
		parse_cmd(line, argv, &cmd_io_status);

		if (process_cmd(argv, &cmd_io_status)) {
			break;
		}
	}

	printf("exitout\n");
	fflush(stdout);
	fflush(stderr);

	free(cmd_io_status.temp_in_file_name);
	free(cmd_io_status.temp_out_file_name);
	free(arg);
	free(argv);

	return 0;
}

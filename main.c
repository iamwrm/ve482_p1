#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif


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
		int fpp = first_pipe_position(argv);
		argv[fpp] = NULL;
		pipe_command(argv, argv + fpp + 1, cmd_io_status);
		// pipe_command_3(argv, cmd_io_status);
		//	     cmd_io_status);
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

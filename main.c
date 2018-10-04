#ifndef wr_head
#define wr_head
#include "mainlib.h"
#endif

int read_line(char* line_input, int line_length)
{
	int position = 0;
	int c;
	while (1) {
		c = getchar();

		if ((c == EOF) && (position == 0)) {
			return 0;
		}

		if (c == EOF || c == '\n') {
			line_input[position] = '\0';
			return 1;
		} else {
			line_input[position] = c;
		}
		position++;
	}
}

// return if_esc
int process_cmd(char** argv)
{
	fflush(stdout);
	fflush(stderr);
	int i = 0;
	if (argv[0] == NULL) {
		return 0;
	}
	if (strcmp(argv[0], "exit") == 0) {
		printf("exit\n");
		fflush(stdout);
		return 1;
	}

	struct Cmd_io_status cmd_io_status;

	cmd_io_status.i_redirected = 0;
	cmd_io_status.o_redirected = 0;

	char* temp_in_file_name = malloc(1024 * sizeof(char));
	char* temp_out_file_name = malloc(1024 * sizeof(char));

	i = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], ">") == 0) {
			cmd_io_status.o_redirected = 1;
			strcpy(temp_out_file_name, argv[i + 1]);
			*(argv[i]) = ' ';
			*(argv[i + 1]) = ' ';
			i++;
			continue;
		}
		if (strcmp(argv[i], ">>") == 0) {
			cmd_io_status.o_redirected = 2;
			strcpy(temp_out_file_name, argv[i + 1]);
			*(argv[i]) = ' ';
			*(argv[i + 1]) = ' ';
			i++;
			continue;
		}
		if (strcmp(argv[i], "<") == 0) {
			cmd_io_status.i_redirected = 1;
			strcpy(temp_in_file_name, argv[i + 1]);
			*(argv[i]) = ' ';
			*(argv[i + 1]) = ' ';
			i++;
			continue;
		}
		i++;
	}

	// begin part remove _ _ =================
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

	// end part remove _ _ =================

	free(temp_argv);

	pid_t pid;
	int status;

	pid = fork();

	if (pid == 0) {
		// child
		if (cmd_io_status.o_redirected == 1) {
			int outfile =
			    open(temp_out_file_name, FLAGS_WRITE, MODE_WR);

			dup2(outfile, STDOUT_FILENO);
		}
		if (cmd_io_status.o_redirected == 2) {
			int outfile =
			    open(temp_out_file_name, FLAG_APPEND, MODE_WR);

			dup2(outfile, STDOUT_FILENO);
		}
		if (cmd_io_status.i_redirected == 1) {
			int in_file =
			    open(temp_in_file_name, FLAG_READ, MODE_WR);

			dup2(in_file, STDIN_FILENO);
		}

		int exe_return_value = execvp(*argv, argv);

		if (exe_return_value < 0) {
			fprintf(stderr, "Error: no such file or directory\n");
			exit(0);
		}

	} else {  // parent
		while (wait(&status) != pid) {
		}
	}
	free(temp_in_file_name);
	free(temp_out_file_name);
	return 0;
}

int main()
{
	int bufsize = 1024;
	char* line = malloc(sizeof(char) * bufsize);
	// size_t len = 0;
	// char argv[64][1024];

	int arg_num = 128;
	int arg_length = 1024;

	char* arg = malloc(arg_num * (arg_length + 1) * sizeof(char));

	char** argv;
	argv = malloc(arg_num * sizeof(char*));

	for (int i = 0; i < arg_num; i++) {
		argv[i] = arg + i * (arg_length + 1) * sizeof(char);
	}

	int if_esc = 0;

	char* sh_name = "mumsh $ ";
	printf("%s", sh_name);
	fflush(stdout);
	fflush(stderr);

	// size_t capacity = 1024;
	// while (getline(&line, &capacity, stdin)) {
	while (read_line(line, bufsize)) {
		parse_cmd(line, argv);

		if_esc = process_cmd(argv);
		if (if_esc) {
			break;
		}

		printf("%s", sh_name);

		fflush(stdout);
		fflush(stderr);
	}

	free(arg);

	free(argv);

	free(line);

	return 0;
}

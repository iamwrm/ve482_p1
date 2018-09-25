#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MODE_WR 0666
#define FLAG_READ O_RDONLY | O_CREAT
#define FLAG_APPEND O_WRONLY | O_CREAT
#define FLAGS_WRITE O_WRONLY | O_CREAT | O_TRUNC

void insert_blank(char* line, int pos)
{
	char temp[1024];
	strcpy(temp, line + pos);
	line[pos] = ' ';
	strcpy(line + pos + 1, temp);
}

void arraw_sep(char* line)
{
	int i = 0;
	while (line[i] != '\0') {
		if (line[i] == '<') {
			insert_blank(line, i);
			insert_blank(line, i + 2);
			i++;
			// insert_blank(line, i);
		}
		if (line[i] == '>') {
			if ((line + i + 1 != NULL) && (line[i + 1] != '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 2);
				i++;
			}
			if ((line + i + 1 != NULL) && (line[i + 1] == '>')) {
				insert_blank(line, i);
				insert_blank(line, i + 3);
				i += 2;
			}
		}

		i++;
	}
}

void clear_buffer(char* line, char** argv)
{
	*line = '\0';
	int position = 0;
	while (argv[position] != NULL) {
		argv[position] = NULL;
		position++;
	}
}

int parse_cmd(char* line, char** argv)
{
	int position = 0;

	arraw_sep(line);
	char* arg;
	if (!argv) {
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	char* sep_er = " \t\n";
	// char* sep_er = " \t\r\n\a";
	arg = strtok(line, sep_er);

	while (arg != NULL) {
		argv[position] = arg;
		position++;
		arg = strtok(NULL, sep_er);
	}
	argv[position] = NULL;
	return 0;
}

// return if_esc
int process_cmd(char** argv, char* line)
{
	// size_t size = strlen(line_input);
	fflush(stdout);
	fflush(stderr);
	int i = 0;
	if (argv[0] == NULL) {
		return 0;
	}
	if (strcmp(argv[0], "exit") == 0) {
		free(line);
		printf("exit\n");
		fflush(stdout);
		fflush(stderr);
		exit(0);
	}

	// if redirect
	int file_redirect_flag = 0;
	int re_l_pos = 0;
	int re_r_pos = 0;

	i = 0;
	while (argv[i] != NULL) {
		if (strcmp(argv[i], ">") == 0) {
			if (file_redirect_flag == 2) {
				file_redirect_flag = 4;
			} else {
				file_redirect_flag = 1;
			}
			re_r_pos = i;
			argv[i] = NULL;
			i++;
			continue;
		}
		if (strcmp(argv[i], ">>") == 0) {
			if (file_redirect_flag == 2) {
				file_redirect_flag = 5;
			} else {
				file_redirect_flag = 3;
			}
			re_r_pos = i;
			argv[i] = NULL;
			i++;
			continue;
		}
		if (strcmp(argv[i], "<") == 0) {
			if (file_redirect_flag == 1) {
				file_redirect_flag = 4;

			} else if (file_redirect_flag == 3) {
				file_redirect_flag = 5;
			} else {
				file_redirect_flag = 2;
			}
			re_l_pos = i;
			argv[i] = NULL;
			i++;
			continue;
		}
		i++;
	}
	// =================

	pid_t pid;
	int status;

	pid = fork();

	if (pid == 0) {
		// child
		fflush(stdout);
		fflush(stderr);
		if (file_redirect_flag > 0) {
			// file redirect happens
			// '_>'
			if (file_redirect_flag == 1) {
				char* filename = argv[re_r_pos + 1];
				int outfile =
				    open(filename, FLAGS_WRITE, MODE_WR);

				dup2(outfile, STDOUT_FILENO);
			}
			// '_+'
			if (file_redirect_flag == 3) {
				char* filename = argv[re_r_pos + 1];
				int outfile = open(
				    filename, O_WRONLY | O_APPEND | O_CREAT,
				    MODE_WR);

				dup2(outfile, STDOUT_FILENO);
			}
			// '<_'
			if (file_redirect_flag == 2) {
				char* filename = argv[re_l_pos + 1];
				int in_file =
				    open(filename, FLAG_READ, MODE_WR);

				dup2(in_file, STDIN_FILENO);
			}
			if (file_redirect_flag == 4) {
				char* out_file_name = argv[re_r_pos + 1];
				char* in_file_name = argv[re_l_pos + 1];
				int out_file =
				    open(out_file_name, FLAGS_WRITE, MODE_WR);
				int in_file =
				    open(in_file_name, FLAG_READ, MODE_WR);

				dup2(out_file, STDOUT_FILENO);
				dup2(in_file, STDIN_FILENO);
			}
			if (file_redirect_flag == 5) {
				char* out_file_name = argv[re_r_pos + 1];
				char* in_file_name = argv[re_l_pos + 1];
				// int out_file =
				//   open(out_file_name, FLAG_APPEND, MODE_WR);
				int out_file = open(
				    out_file_name,
				    O_WRONLY | O_APPEND | O_CREAT, MODE_WR);
				int in_file =
				    open(in_file_name, FLAG_READ, MODE_WR);

				dup2(out_file, STDOUT_FILENO);
				dup2(in_file, STDIN_FILENO);
			}
		}

		fflush(stdout);
		fflush(stderr);
		int exe_return_value = execvp(*argv, argv);
		fflush(stderr);
		fflush(stdout);

		if (exe_return_value < 0) {
			fprintf(stderr, "Error: no such file or directory\n");
			fflush(stderr);
			fflush(stdout);
			exit(0);
		}

	} else {  // parent
		while (wait(&status) != pid) {
		}
	}
	// =================
	return 0;
}

int main()
{
	int bufsize = 1024;
	char* line = malloc(sizeof(char) * bufsize);
	// size_t len = 0;
	size_t capacity = 1024;
	// char argv[64][1024];

	char** argv;

	argv = malloc(64 * sizeof(char*));
	for (int i = 0; i < 64; i++)
		argv[i] = malloc((1024 + 1) *
				 sizeof(char));  // yeah, I know sizeof(char) is
						 // 1, but to make it clear...

	int if_esc = 0;

	char* sh_name = "mumsh $ ";
	printf("%s", sh_name);
	fflush(stdout);
	fflush(stderr);
	//	while (1) {
	while (getline(&line, &capacity, stdin)) {
		// while (getline(&line, &capacity, stdin)) {
		// fflush(stdout);
		// fflush(stderr);
		// getline(&line, &capacity, stdin);
		// if (fgets(line, capacity, stdin) == NULL) continue;

		parse_cmd(line, argv);
		// parse_cmd1(line, argv);
		if_esc = process_cmd(argv, line);

		clear_buffer(line, argv);

		printf("%s", sh_name);
		fflush(stdout);
		if (if_esc) {
			break;
		}
	}

	free(line);
	return 0;
}

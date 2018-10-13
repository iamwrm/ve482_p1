#define _XOPEN_SOURCE 700
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MODE_WR 0666
#define FLAG_READ O_RDONLY | O_CREAT
#define FLAG_APPEND O_WRONLY | O_CREAT | O_APPEND
#define FLAGS_WRITE O_WRONLY | O_CREAT | O_TRUNC
#define BUFF_SIZE 1024
#define DEBUG_MODE 0

struct Cmd_status {
	int i_redirected;  // 0 -> stdin ; 1 -> file_in
	int o_redirected;  // 0 -> stdout; 1 -> file_out; 2 -> file_out_append
	int pipe_number;
	int init_pipe_number;
	char* temp_in_file_name;
	char* temp_out_file_name;
};

void process_sig_handler(int);

int my_execvp(char* cmdhead, char** cmd);

void cmd_mid(struct Cmd_status* cmd_io_status, char** cmd2, int* fds_1,
	     int* fds);

void cmd_head(struct Cmd_status* cmd_io_status, char** cmd1, int* fds_1);

void cmd_tail(struct Cmd_status* cmd_io_status, char** cmd3, int* fds);

int find_the_nth_pipe(char** argv, int n);

void pipe_helper(char** argv, struct Cmd_status* cmd_io_status, int init_depth,
		 int depth, int* input_p);

void pipe_command_3(char** argv, struct Cmd_status* cmd_io_status);

void pipe_command(char** cmd1, char** cmd2, struct Cmd_status* cmd_io_status);

void dup_and_exc(struct Cmd_status* cmd_io_status, char** argv);

void set_redirect_status(struct Cmd_status* cmd_io_status, char** argv);

void find_redirect_symbols(char** argv, struct Cmd_status* cmd_io_status);

int read_line(char* line_input );

int first_pipe_position(char** argv);

void remove_blank_in_argv(char** argv);

void count_real_pipe(const char* line, struct Cmd_status* cmd_status);

void insert_blank(char* line, int pos);

void arrow_sep(char* line);

// make line into * argv[]
int parse_cmd(char* line, char** argv, struct Cmd_status*);


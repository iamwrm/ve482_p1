#define _XOPEN_SOURCE 700
#include <assert.h>
#include <fcntl.h>
//#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MODE_WR 0666
#define FLAG_READ O_RDONLY | O_CREAT
#define FLAG_APPEND O_WRONLY | O_CREAT | O_APPEND
#define FLAGS_WRITE O_WRONLY | O_CREAT | O_TRUNC

struct Cmd_io_status {
	int i_redirected;  // 0 -> stdin ; 1 -> file_in
	int o_redirected;  // 0 -> stdout; 1 -> file_out; 2 -> file_out_append
};


void insert_blank(char* line, int pos);

void arrow_sep(char* line);

// make line into * argv[]
int parse_cmd(char* line, char** argv);

void clear_buffer(char* line, char** argv);

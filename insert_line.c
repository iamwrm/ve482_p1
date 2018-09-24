#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void insert_blank(char* line, int pos)
{
	int i = pos;

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
				i+=2;
			}
		}

		i++;
	}
}
int main()
{
	char aline[40] = "Geeks<for>Ge>>eks";
	printf("%s\n", aline);
	arraw_sep(aline);
	printf("%s\n", aline);
}
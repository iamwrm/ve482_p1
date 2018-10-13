CC = clang
CFLAGS = -std=c11 -O2 -Wall -Wextra -pedantic  -g
#CFLAGS = -std=c11 -O2 -Wall -Wextra -Werror -pedantic -Wno-unused-result
MUMSH_SRC = main.c mainlib.c parse_cmd.c
MUMSH = mumsh
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
MUMSHMC = mumsh_memory_check
.PHONY: clean

all: $(MUMSH) $(MUMSHMC)
	@echo mumsh successfully constructed

old: mumsh.c
	$(CC) $(CFLAGS) -o mumshold mumsh.c

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) $(MUMSHMC_FLAGS) -o $(MUMSHMC) $(MUMSH_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC)

tar:all
	tar -cvzf p1.tar Makefile main.c mainlib.c mainlib.h
tar_cmake:all
	tar -cvzf p1cmake.tar CMakeLists.txt main.c
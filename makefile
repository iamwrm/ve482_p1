CC = clang
CCg = gcc
CFLAGS = -std=c11 -O2 -Wall -Wextra -pedantic  -g
CgFLAGS = -std=c11 -O2 -Wall -Wextra -pedantic  -g
#CFLAGS = -std=c11 -O2 -Wall -Wextra -Werror -pedantic -Wno-unused-result
MUMSH_SRC = mumsh.c
MUMSH = mumsh
MUMSHMC_FLAGS = -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer
MUMSHMC = mumsh_memory_check
.PHONY: clean

#all: $(MUMSH) $(MUMSHMC)
	@echo mumsh successfully constructed

all: $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSH): $(MUMSH_SRC)
	$(CC) $(CFLAGS) -o $(MUMSH) $(MUMSH_SRC)

$(MUMSHMC) : $(MUMSH_SRC)
	$(CC) $(CFLAGS) $(MUMSHMC_FLAGS) -o $(MUMSHMC) $(MUMSHMC_SRC)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.a *~ $(MUMSH) $(MUMSHMC)

gcc: $(MUMSH_SRC)
	$(CCg) $(CgFLAGS) -o $(MUMSH) $(MUMSH_SRC)

tar:all
	tar -cvzf p1.tar makefile mumsh.c
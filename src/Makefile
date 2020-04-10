CC=gcc

# Compiler
CFLAGS=-Wall -Wextra -Werror -pedantic

# Linker flags (after sources)
LFLAGS=-lm

# Source files
SRCS = $(wildcard *.c)

# Target
PROG = simpledu

simpledu:
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LFLAGS)

clean:
	rm $(PROG)
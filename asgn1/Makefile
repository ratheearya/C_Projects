CC = clang
CFLAGS = -Wall -Wextra -Werror -pedantic

all: memory

memory: memory.o
	$(CC) $(CFLAGS) -o memory memory.o

memory.o: memory.c
	$(CC) $(CFLAGS) -c memory.c

clean:
	rm -f memory memory.o

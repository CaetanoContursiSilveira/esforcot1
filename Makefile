# Makefile
CC      = gcc
CFLAGS  = -g -O0 -Wall -fsanitize=address -fno-omit-frame-pointer
LDFLAGS = -fsanitize=address
OBJS    = agent.o cidades.o main.o

all: simulator

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

simulator: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -lm -o simulator

clean:
	rm -f $(OBJS) simulator

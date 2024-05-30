CC = gcc
CFLAGS = -Wall -Wextra -Werror -Werror=format=0 -lm

all: TermCalc ListPool

TermCalc: main.c  
	$(CC) -o TermCalc main.c $(CFLAGS)

ListPool: list_pool.c
	$(CC) -o ListPool list_pool.c $(CFLAGS)
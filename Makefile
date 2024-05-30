CC = gcc
CFLAGS = -Wall -Wextra -Werror -Werror=format=0 -lm

TermCalc: main.c  
	$(CC) -o TermCalc main.c $(CFLAGS)
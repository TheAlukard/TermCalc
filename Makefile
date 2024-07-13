CC = gcc
CFLAGS = -Wall -Wextra -Werror -Werror=format=0 -lm

TermCalc.exe: main.c  
	$(CC) -o TermCalc.exe main.c $(CFLAGS)

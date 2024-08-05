CC=gcc
SRC=$(shell ls ./src/*.c)
CFLAGS=-Wall -Wextra -Werror -Werror=format=0 -lm

TermCalc.exe: $(SRC)  
	$(CC) -o TermCalc.exe $(SRC) $(CFLAGS)

CC = gcc -Wall -g -lm

ls: ls.o utils.o
	$(CC) -o ls ls.o utils.o

ls.o : ls.c
	$(CC) -c ls.c
	
utils.o : utils.c
	$(CC) -c utils.c

clean:
	rm -f ls ls.o utils.o *~



#
# makefile for thread pool
#

CC=gcc
CFLAGS=-Wall

all: client.o threadpool.o
	$(CC) $(CFLAGS) -o client client.o threadpool.o 

client.o: client.c
	$(CC) $(CFLAGS) -c client.c 

threadpool.o: threadpool.c threadpool.h
	$(CC) $(CFLAGS) -c threadpool.c 

clean:
	rm -rf *.o
	rm -rf client

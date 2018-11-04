CC=gcc
CFLAGS=-O3 -funroll-loops

seondchessmake: secondchess.c
	$(CC) -o secondchess *.c $(CFLAGS)

CC = gcc
CC_OPTS = -Wall -std=gnu99 -I./src

VPATH = src:tests

all : grackle test

grackle : grackle.o fundamentals.o environment.o parse.o evaluate.o grackle_io.o
	$(CC) $(CC_OPTS) $^ -o $@

test : test.o fundamentals.o environment.o parse.o evaluate.o test_functions.c test_functions.h
	$(CC) $(CC_OPTS) $^ -o $@

grackle.o : grackle.c
	$(CC) $(CC_OPTS) $^ -c -o $@

test.o : test.c
	$(CC) $(CC_OPTS) $^ -c -o $@

test_functions.o : test_functions.c
	$(CC) $(CC_OPTS) $^ -c -o $@

fundamentals.o : fundamentals.c
	$(CC) $(CC_OPTS) $^ -c -o $@

environment.o : environment.c
	$(CC) $(CC_OPTS) $^ -c -o $@

parse.o : parse.c
	$(CC) $(CC_OPTS) $^ -c -o $@

evaluate.o : evaluate.c
	$(CC) $(CC_OPTS) $^ -c -o $@

grackle_io.o : grackle_io.c
	$(CC) $(CC_OPTS) $^ -c -o $@

CC = gcc
CC_OPTS = -Wall -std=gnu99

grackle : grackle.o interp.o interp.h
	$(CC) $(CC_OPTS) $^ -o $@

test : test.o interp.o test_functions.c test_functions.h interp.h
	$(CC) $(CC_OPTS) $^ -o $@

interp.o : interp.c
	$(CC) $(CC_OPTS) $^ -c -o $@

grackle.o : grackle.c
	$(CC) $(CC_OPTS) $^ -c -o $@

test.o : test.c
	$(CC) $(CC_OPTS) $^ -c -o $@

test_functions.o : test_functions.c
	$(CC) $(CC_OPTS) $^ -c -o $@
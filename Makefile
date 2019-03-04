CC = gcc
CC_OPTS = -Wall -std=gnu99

grackle : grackle.o
	$(CC) $(CC_OPTS) $^ -o $@

grackle.o : interp.c
	$(CC) $(CC_OPTS) $^ -c -o $@

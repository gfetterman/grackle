CC = gcc
CC_OPTS = -Wall -std=gnu99 -I./src -o0 -g

VPATH = src:tests

all : grackle test

grackle : grackle.o fundamentals.o environment.o parse.o evaluate.o grackle_io.o
	$(CC) $(CC_OPTS) $^ -o $@

test : test.o fundamentals.o environment.o parse.o evaluate.o end_to_end_tests.o unit_tests_fundamentals.o unit_tests_environment.o unit_tests_parse.o unit_tests_evaluate.o test_utils.o
	$(CC) $(CC_OPTS) $^ -o $@

grackle.o : grackle.c
	$(CC) $(CC_OPTS) $^ -c -o $@

test.o : test.c
	$(CC) $(CC_OPTS) $^ -c -o $@

test_utils.o : test_utils.c
	$(CC) $(CC_OPTS) $^ -c -o $@

unit_tests_fundamentals.o : unit_tests_fundamentals.c
	$(CC) $(CC_OPTS) $^ -c -o $@

unit_tests_environment.o : unit_tests_environment.c
	$(CC) $(CC_OPTS) $^ -c -o $@

unit_tests_parse.o : unit_tests_parse.c
	$(CC) $(CC_OPTS) $^ -c -o $@

unit_tests_evaluate.o : unit_tests_evaluate.c
	$(CC) $(CC_OPTS) $^ -c -o $@

end_to_end_tests.o : end_to_end_tests.c
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

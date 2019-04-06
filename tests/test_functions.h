#ifndef TEST_FUNCTIONS_H
#define TEST_FUNCTIONS_H

#include<stdlib.h>
#include<assert.h>
#include<stdio.h>

#include "fundamentals.h"
#include "environment.h"
#include "parse.h"
#include "evaluate.h"

typedef struct TEST_ENV {
    Environment* env;
    unsigned int passed;
    unsigned int run;
} test_env;

typed_ptr* parse_and_evaluate(char command[], Environment* env);

bool check_typed_ptr(typed_ptr* tp, type t, tp_value ptr);
bool check_pair(typed_ptr* tp, \
                typed_ptr** tp_list, \
                unsigned int tp_list_len, \
                Environment* env);
bool check_sexpr(typed_ptr* tp, \
                 typed_ptr** tp_list, \
                 unsigned int tp_list_len, \
                 Environment* env);

void e2e_autofail_test(char cmd[], test_env* te);
void e2e_atom_test(char cmd[], type t, long val, test_env* te);
void e2e_multiline_atom_test(char* cmds[], \
                             unsigned int cmd_num, \
                             type t, \
                             long val, \
                             test_env* te);
void e2e_pair_test(char cmd[], \
                   typed_ptr** tp_list, \
                   unsigned int tp_list_len, \
                   test_env* te);
void e2e_sexpr_test(char cmd[], \
                    typed_ptr** tp_list, \
                    unsigned int tp_list_len, \
                    test_env* te);

#endif

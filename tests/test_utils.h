#ifndef TEST_UTILS_H
#define TEST_UTILS_H

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

bool match_typed_ptrs(typed_ptr* first, typed_ptr* second);

#endif

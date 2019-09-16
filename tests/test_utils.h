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

bool check_error(const typed_ptr* tp, interpreter_error err);

bool match_typed_ptrs(typed_ptr* first, typed_ptr* second);
bool match_s_exprs(const s_expr* first, const s_expr* second);
bool deep_match_typed_ptrs(typed_ptr* first, typed_ptr* second);

void print_test_announce(char function[]);
void print_test_result(bool pass);

typed_ptr* builtin_tp_from_name(Environment* env, const char name[]);
typed_ptr* symbol_tp_from_name(Environment* env, const char name[]);

#endif

#ifndef TEST_FUNCTIONS_H
#define TEST_FUNCTIONS_H

#include<stdlib.h>
#include<assert.h>
#include<stdio.h>

#include "interp.h"

typedef struct TEST_ENV {
    environment* env;
    unsigned int passed;
    unsigned int run;
} test_env;

typed_ptr* parse_and_eval(char command[], environment* env);

bool check_tp(typed_ptr* tp, type t, union_idx_se ptr);
bool check_pair(typed_ptr* tp, typed_ptr** tplist, unsigned int len, environment* env);
bool check_sexpr(typed_ptr* tp, typed_ptr** tplist, unsigned int len, environment* env);

void e2e_autofail_test(char cmd[], test_env* te);
void e2e_atom_test(char cmd[], type t, unsigned int val, test_env* te);
void e2e_multiline_atom_test(char* cmds[], unsigned int cmd_num, type t, unsigned int val, test_env* te);
void e2e_pair_test(char cmd[], typed_ptr** tplist, unsigned int len, test_env* te);
void e2e_sexpr_test(char cmd[], typed_ptr** tplist, unsigned int len, test_env* te);

#endif

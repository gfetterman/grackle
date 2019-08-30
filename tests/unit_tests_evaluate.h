#ifndef UNIT_TESTS_EVALUATE_H
#define UNIT_TESTS_EVALUATE_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "fundamentals.h"
#include "evaluate.h"
#include "test_utils.h"

void unit_tests_evaluate(test_env* te);

void test_collect_parameters(test_env* te);
void test_bind_args(test_env* te);
void test_make_eval_env(test_env* te);
void test_collect_arguments(test_env* te);

void test_eval_arithmetic(test_env* te);
void test_eval_comparison(test_env* te);
void test_eval_define(test_env* te);
void test_eval_setvar(test_env* te);
void test_eval_exit(test_env* te);
void test_eval_cons(test_env* te);
void test_eval_car_cdr(test_env* te);
void test_eval_list_construction(test_env* te);
void test_eval_and_or(test_env* te);
void test_eval_not(test_env* te);
void test_eval_cond(test_env* te);
void test_eval_list_pred(test_env* te);
void test_eval_atom_pred(test_env* te);
void test_eval_null_pred(test_env* te);
void test_eval_lambda(test_env* te);
void test_eval_quote(test_env* te);
void test_eval_string_length(test_env* te);
void test_eval_string_equals(test_env* te);

void test_eval_builtin(test_env* te);
void test_eval_s_expr(test_env* te);
void test_eval_function(test_env* te);

void test_evaluate(test_env* te);

#endif

#ifndef UNIT_TESTS_EVALUATE_H
#define UNIT_TESTS_EVALUATE_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "evaluate.h"
#include "test_utils.h"

void unit_tests_evaluate(test_env* te);

void test_collect_parameters(test_env* te);
void test_bind_args(test_env* te);
void test_make_eval_env(test_env* te);
void test_collect_arguments(test_env* te);

#endif

#ifndef UNIT_TESTS_TEST_UTILS_H
#define UNIT_TESTS_TEST_UTILS_H

#include<stdlib.h>
#include<assert.h>
#include<stdio.h>

#include "unit_tests.h"

void unit_tests_test_utils(test_env* te);

void test_check_error(test_env* te);

void test_match_typed_ptrs(test_env* te);
void test_match_s_exprs(test_env* te);
void test_deep_match_typed_ptrs(test_env* te);

void test_unit_list(test_env* te);
void test_s_expr_append(test_env* te);
void test_create_number_tp(test_env* te);

void test_builtin_tp_from_name(test_env* te);
void test_symbol_tp_from_name(test_env* te);

#endif

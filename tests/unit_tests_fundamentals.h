#ifndef UNIT_TESTS_FUNDAMENTALS_H
#define UNIT_TESTS_FUNDAMENTALS_H

#include "fundamentals.h"
#include "test_utils.h"

void unit_tests_fundamentals(test_env* t_env);

void test_create_typed_ptr(test_env* te);
void test_create_atom_tp(test_env* te);
void test_create_error_tp(test_env* te);
void test_create_void_tp(test_env* te);
void test_create_s_expr_tp(test_env* te);
void test_create_string_tp(test_env* te);
void test_copy_typed_ptr(test_env* te);
void test_create_s_expr(test_env* te);
void test_create_empty_s_expr(test_env* te);
void test_copy_s_expr(test_env* te);
void test_delete_s_expr_recursive(test_env* te);
void test_create_string(test_env* te);
void test_s_expr_next(test_env* te);
void test_is_empty_list(test_env* te);
void test_is_false_literal(test_env* te);
void test_is_pair(test_env* te);

#endif

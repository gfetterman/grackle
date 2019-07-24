#ifndef UNIT_TESTS_H
#define UNIT_TESTS_H

#include "fundamentals.h"
#include "test_utils.h"

// fundamentals.c

void unit_tests_fundamentals(test_env* t_env);

void test_create_typed_ptr(test_env* te);
void test_create_atom_tp(test_env* te);
void test_create_s_expr_tp(test_env* te);
void test_create_error_tp(test_env* te);
void test_copy_typed_ptr(test_env* te);
void test_create_s_expr(test_env* te);
void test_create_empty_s_expr(test_env* te);
void test_copy_s_expr(test_env* te);
void test_delete_s_expr_recursive(test_env* te);
void test_s_expr_next(test_env* te);
void test_is_empty_list(test_env* te);
void test_is_false_literal(test_env* te);
void test_is_pair(test_env* te);

// environment.c

void unit_tests_environment(test_env* t_env);

void test_create_symbol_node(test_env* te);
void test_create_error_symbol_node(test_env* te);
void test_create_symbol_table(test_env* te);
void test_merge_symbol_tables(test_env* te);
void test_delete_symbol_node_list(test_env* te);
void test_create_function_node(test_env* te);
void test_create_function_table(test_env* te);
void test_create_environment(test_env* te);
void test_copy_environment(test_env* te);
void test_delete_environment_shared_full(test_env* te);
void test_install_symbol_regular_and_blind(test_env* te);
void test_install_function(test_env* te);
void test_symbol_lookup_string(test_env* te);
void test_symbol_lookup_index(test_env* te);
void test_builtin_lookup_index(test_env* te);
void test_value_lookup_index(test_env* te);
void test_function_lookup_index(test_env* te);

#endif

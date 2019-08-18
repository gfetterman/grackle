#ifndef UNIT_TESTS_ENVIRONMENT_H
#define UNIT_TESTS_ENVIRONMENT_H

#include "environment.h"
#include "test_utils.h"

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
void test_symbol_lookup_name(test_env* te);
void test_symbol_lookup_index(test_env* te);
void test_builtin_lookup_index(test_env* te);
void test_value_lookup_index(test_env* te);
void test_function_lookup_index(test_env* te);

#endif

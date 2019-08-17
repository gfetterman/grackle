#ifndef UNIT_TESTS_PARSE_H
#define UNIT_TESTS_PARSE_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "parse.h"
#include "test_utils.h"

void unit_tests_parse(test_env* t_env);

void test_create_s_expr_stack(test_env* te);
void test_s_expr_stack_push_pop(test_env* te);
void test_init_new_s_expr(test_env* te);
void test_extend_s_expr(test_env* te);
void test_terminate_s_expr(test_env* te);
void test_register_symbol(test_env* te);
void test_substring(test_env* te);
void test_string_is_number(test_env* te);
void test_parse(test_env* te);

#endif

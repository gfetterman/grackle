#ifndef END_TO_END_TESTS_H
#define END_TO_END_TESTS_H

#include "test_utils.h"

void e2e_autofail_test(char cmd[], test_env* te);
void e2e_atom_test(char cmd[], type t, long val, test_env* te);
void e2e_multiline_atom_test(char* cmds[], \
                             unsigned int cmd_num, \
                             type t, \
                             long val, \
                             test_env* te);
void e2e_pair_test(char cmd[], \
                   typed_ptr** tp_list, \
                   unsigned int tp_list_len, \
                   test_env* te);
void e2e_sexpr_test(char cmd[], \
                    typed_ptr** tp_list, \
                    unsigned int tp_list_len, \
                    test_env* te);

void end_to_end_parse_tests(test_env* t_env);
void end_to_end_arithmetic_tests(test_env* t_env);
void end_to_end_numerical_comparison_tests(test_env* t_env);
void end_to_end_exit_tests(test_env* t_env);
void end_to_end_predicate_tests(test_env* t_env);
void end_to_end_boolean_operation_tests(test_env* t_env);
void end_to_end_car_cdr_tests(test_env* t_env);
void end_to_end_cond_tests(test_env* t_env);
void end_to_end_cons_tests(test_env* t_env);
void end_to_end_list_tests(test_env* t_env);
void end_to_end_lambda_tests(test_env* t_env);
void end_to_end_setvar_tests(test_env* t_env);
void end_to_end_define_tests(test_env* t_env);

#endif

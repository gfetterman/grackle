#include "end_to_end_tests.h"

void e2e_autofail_test(char cmd[], test_env* te) {
    // for tests that segfault, so I know to come back to them later
    printf("test command: %-40s", cmd);
    printf("FAILED <= (auto)\n");
    te->run++;
    return;
}

void e2e_atom_test(char cmd[], type t, long val, test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    typed_ptr* expected = create_atom_tp(t, val);
    bool pass = match_typed_ptrs(output, expected);
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    } else if (output->type == TYPE_STRING) {
        delete_string(output->ptr.string);
    }
    free(output);
    free(expected);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void e2e_pair_test(char cmd[], typed_ptr* car, typed_ptr* cdr, test_env* te) {
    printf("test command: %-40s", cmd);
    bool pass = false;
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    typed_ptr* expected = create_s_expr_tp(create_s_expr(car, cdr));
    pass = deep_match_typed_ptrs(output, expected);
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    } else if (output->type == TYPE_STRING) {
        delete_string(output->ptr.string);
    }
    free(output);
    free(expected->ptr.se_ptr);
    free(expected);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void e2e_s_expr_test(char cmd[], \
                     typed_ptr** tp_list, \
                     unsigned int tp_list_len, \
                     test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    typed_ptr* expected = create_s_expr_tp(create_empty_s_expr());
    for (unsigned int i = 0; i < tp_list_len; i++) {
        s_expr_append(expected->ptr.se_ptr, copy_typed_ptr(tp_list[i]));
    }
    bool pass = deep_match_typed_ptrs(output, expected);
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    } else if (output->type == TYPE_STRING) {
        delete_string(output->ptr.string);
    }
    free(output);
    delete_s_expr_recursive(expected->ptr.se_ptr, false);
    free(expected);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void e2e_multiline_atom_test(char* cmds[], \
                             unsigned int num_cmds, \
                             type t, \
                             long val, \
                             test_env* te) {
    printf("test command: %-40s", cmds[0]);
    typed_ptr* output = parse_and_evaluate(cmds[0], te->env);
    for (unsigned int i = 1; i < num_cmds; i++) {
        printf("\n");
        free(output);
        printf("              %-40s", cmds[i]);
        output = parse_and_evaluate(cmds[i], te->env);
    }
    typed_ptr* expected = create_atom_tp(t, val);
    bool pass = match_typed_ptrs(output, expected);
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    } else if (output->type == TYPE_STRING) {
        delete_string(output->ptr.string);
    }
    free(output);
    free(expected);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void e2e_string_test(char cmd[], char expected_str[], test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    typed_ptr* expected = create_string_tp(create_string(expected_str));
    bool pass = match_typed_ptrs(output, expected);
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    } else if (output->type == TYPE_STRING) {
        delete_string(output->ptr.string);
    }
    free(output);
    delete_string(expected->ptr.string);
    free(expected);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void end_to_end_parse_tests(test_env* t_env) {
    printf("# parsing #\n");
    e2e_atom_test("", TYPE_ERROR, EVAL_ERROR_MISSING_PROCEDURE, t_env);
    e2e_atom_test("(", TYPE_ERROR, PARSE_ERROR_UNBAL_PAREN, t_env);
    e2e_atom_test(")", TYPE_ERROR, PARSE_ERROR_UNBAL_PAREN, t_env);
    e2e_atom_test("a", TYPE_ERROR, PARSE_ERROR_BARE_SYM, t_env);
    e2e_atom_test("()", TYPE_ERROR, EVAL_ERROR_MISSING_PROCEDURE, t_env);
    e2e_atom_test("(+ 1 1) (+ 1 1)", TYPE_ERROR, PARSE_ERROR_TOO_MANY, t_env);
    e2e_atom_test("(\"test", TYPE_ERROR, PARSE_ERROR_UNBAL_DOUBLE_QUOTE, t_env);
    return;
}

void end_to_end_arithmetic_tests(test_env* t_env) {
    printf("# arithmetic #\n");
    printf("## + ##\n");
    e2e_atom_test("(+ 1 2 3 4)", TYPE_FIXNUM, 10, t_env);
    e2e_atom_test("(+ 1 1)", TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(+ 1)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(+)", TYPE_FIXNUM, 0, t_env);
    e2e_atom_test("(+ 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(+ 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## - ##\n");
    e2e_atom_test("(- 10 1 2 3)", TYPE_FIXNUM, 4, t_env);
    e2e_atom_test("(- 10 1)", TYPE_FIXNUM, 9, t_env);
    e2e_atom_test("(- 10)", TYPE_FIXNUM, -10, t_env);
    e2e_atom_test("(-)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(- 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(- 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## * ##\n");
    e2e_atom_test("(* 2 3 4)", TYPE_FIXNUM, 24, t_env);
    e2e_atom_test("(* 2 3)", TYPE_FIXNUM, 6, t_env);
    e2e_atom_test("(* 2)", TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(*)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(* 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(* 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## / ##\n");
    e2e_atom_test("(/ 100 4 5)", TYPE_FIXNUM, 5, t_env);
    e2e_atom_test("(/ 100 4)", TYPE_FIXNUM, 25, t_env);
    e2e_atom_test("(/ 1)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(/)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(/ 100 0)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(/ 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(/ 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_numerical_comparison_tests(test_env* t_env) {
    printf("# comparisons #\n");
    printf("## = ##\n");
    e2e_atom_test("(= 1 1 (- 2 1))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(= 1 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(= 1 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(= 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(=)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(= 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(= 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## < ##\n");
    e2e_atom_test("(< 1 2)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(< 1 2 3)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(< 2 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(< 1 3 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(< 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(<)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(< 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(< 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## > ##\n");
    e2e_atom_test("(> 2 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(> 3 2 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(> 1 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(> 3 1 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(> 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(>)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(> 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(> 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## <= ##\n");
    e2e_atom_test("(<= 1 2)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(<= 1 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(<= 1 2 3)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(<= 1 2 2)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(<= 1 2 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(<= 2 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(<= 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(<=)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(<= 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(<= 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## >= ##\n");
    e2e_atom_test("(>= 2 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(>= 2 2)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(>= 3 2 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(>= 3 2 2)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(>= 3 2 3)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(>= 1 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(>= 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(>=)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(>= 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(>= 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_exit_tests(test_env* t_env) {
    printf("# exit #\n");
    e2e_atom_test("(exit)", TYPE_ERROR, EVAL_ERROR_EXIT, t_env);
    e2e_atom_test("(exit 1)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    return;
}

void end_to_end_predicate_tests(test_env* t_env) {
    printf("# predicates #\n");
    printf("## pair? ##\n");
    e2e_atom_test("(pair? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(pair? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(pair? (list 1 2 3))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(pair? (cons 1 2))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(pair? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(pair? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(pair? +)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(pair? (lambda () 1))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(pair? \"hello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(pair?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(pair? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(pair? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## list? ##\n");
    e2e_atom_test("(list? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? (list 1 2 3))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(list? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? null)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(list? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? +)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? (lambda () 1))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? \"hello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(list? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(list? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## number? ##\n");
    e2e_atom_test("(number? 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(number? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? +)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? (lambda () 1))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? \"hello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(number? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(number? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## boolean? ##\n");
    e2e_atom_test("(boolean? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(boolean? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? +)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? (lambda () 1))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? \"hello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(boolean? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(boolean? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## void? ##\n");
    e2e_atom_test("(void? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? (cond))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(void? +)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? (lambda () 1))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? \"hello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(void? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(void? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## procedure? ##\n");
    e2e_atom_test("(procedure? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(procedure? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(procedure? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(procedure? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(procedure? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(procedure? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(procedure? +)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(procedure? (lambda () 1))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(procedure? \"hello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(procedure?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(procedure? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(procedure? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## null? ##\n");
    e2e_atom_test("(null? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null? null)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(null? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null? +)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null? (lambda () 1))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null? \"hello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(null?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(null? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(null? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## string? ##\n");
    e2e_atom_test("(string? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? +)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? (lambda () 1))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string? \"hello\")", TYPE_BOOL, true, t_env);
    e2e_atom_test("(string?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(string? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(string? (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_boolean_operation_tests(test_env* t_env) {
    printf("# boolean operators #\n");
    printf("## and ##\n");
    e2e_atom_test("(and)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(and 1)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(and #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(and #f)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(and #t #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(and #t #f)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(and #f #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(and #f #f)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(and #t #t #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(and #t #t #f)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(and #t #f #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(and #f #t #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(and (/ 0) #t #t)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(and #t (/ 0) #t)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## or ##\n");
    e2e_atom_test("(or)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(or 1)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(or #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #f)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(or #t #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #t #f)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #f #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #f #f)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(or #t #t #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #t #t #f)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #t #f #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #f #t #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(or #f #f #f)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(or (/ 0) #t #t)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(or #f (/ 0) #t)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    printf("## not ##\n");
    e2e_atom_test("(not 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(not 0)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(not #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(not #f)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(not (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(not (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(not null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(not)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(not #t #t)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(not (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_car_cdr_tests(test_env* t_env) {
    printf("# car & cdr #\n");
    e2e_atom_test("(car (cons 1 2))", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(cdr (cons 1 2))", TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(car (list 1 2 3))", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(car (cdr (list 1 2 3)))", TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(car (cdr (cdr (list 1 2 3))))", TYPE_FIXNUM, 3, t_env);
    e2e_atom_test("(car 1)", TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(car)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    char* car_many = "(car (cons 1 2) (cons 3 4))";
    e2e_atom_test(car_many, TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(car null)", TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(cdr 1)", TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(cdr)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    char* cdr_many = "(cdr (cons 1 2) (cons 3 4))";
    e2e_atom_test(cdr_many, TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(cdr null)", TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(car (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(cdr (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_cond_tests(test_env* t_env) {
    printf("# cond #\n");
    e2e_atom_test("(cond)", TYPE_VOID, 0, t_env);
    e2e_atom_test("(cond #t)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(cond (#t))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(cond (#t 1))", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(cond (#f 1))", TYPE_VOID, 0, t_env);
    e2e_atom_test("(cond (#t 1) (else 2))", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(cond (#f 1) (else 2))", TYPE_FIXNUM, 2, t_env);
    char* empty_else = "(cond (#f 1) (else))";
    e2e_atom_test(empty_else, TYPE_ERROR, EVAL_ERROR_EMPTY_ELSE, t_env);
    char* nonterm_else = "(cond (#f 1) (else 2) (#t 3))";
    e2e_atom_test(nonterm_else, TYPE_ERROR, EVAL_ERROR_NONTERMINAL_ELSE, t_env);
    e2e_atom_test("(cond (#t (+ 1 1) (+ 2 2)))", TYPE_FIXNUM, 4, t_env);
    e2e_atom_test("(cond ((/ 0) 1))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(cond (#t (/ 0)))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_cons_tests(test_env* t_env) {
    typed_ptr* num_1 = create_atom_tp(TYPE_FIXNUM, 1);
    typed_ptr* num_2 = create_atom_tp(TYPE_FIXNUM, 2);
    typed_ptr* cons_test_num_pair[] = {num_1, num_2};
    printf("# cons #\n");
    e2e_atom_test("(cons)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cons 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_pair_test("(cons 1 2)", num_1, num_2, t_env);
    e2e_s_expr_test("(cons 1 null)", cons_test_num_pair, 1, t_env);
    e2e_s_expr_test("(cons 1 (cons 2 null))", cons_test_num_pair, 2, t_env);
    e2e_atom_test("(cons 1 2 3)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(cons (/ 0) 2)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(cons 1 (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    free(num_1);
    free(num_2);
    return;
}

void end_to_end_list_tests(test_env* t_env) {
    typed_ptr* num_1 = create_atom_tp(TYPE_FIXNUM, 1);
    typed_ptr* num_2 = create_atom_tp(TYPE_FIXNUM, 2);
    typed_ptr* list_test_num_pair[] = {num_1, num_2};
    printf("# list #\n");
    e2e_s_expr_test("(list)", list_test_num_pair, 0, t_env);
    e2e_s_expr_test("(list 1)", list_test_num_pair, 1, t_env);
    e2e_s_expr_test("(list (- 3 2) (+ 1 1))", list_test_num_pair, 2, t_env);
    e2e_atom_test("(list (/ 0) 2 3)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(list 1 (/ 0) 3)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    free(num_1);
    free(num_2);
    return;
}

void end_to_end_lambda_tests(test_env* t_env) {
    printf("# lambda #\n");
    e2e_atom_test("(lambda)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(lambda 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(lambda 1 2)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(lambda ())", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("((lambda () 1))", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("((lambda () 1) 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("((lambda () (+ 1 1)))", TYPE_FIXNUM, 2, t_env);
    char* lam_0_few = "((lambda (x) (* x 10)))";
    e2e_atom_test(lam_0_few, TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("((lambda (x) (* x 10)) 3)", TYPE_FIXNUM, 30, t_env);
    e2e_atom_test("((lambda (x) (* x 10)) (+ 1 2))", TYPE_FIXNUM, 30, t_env);
    char* lam_body_error = "((lambda (x) (* x 10)) #t)";
    e2e_atom_test(lam_body_error, TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    char* lam_prop_error = "((lambda (x) (* x 10)) (/ 0))";
    e2e_atom_test(lam_prop_error, TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    char* lam_0_many = "((lambda (x) (* x 10)) 3 4)";
    e2e_atom_test(lam_0_many, TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    char* lam_not_id = "((lambda (x 2) (* x 10)) 3)";
    e2e_atom_test(lam_not_id, TYPE_ERROR, EVAL_ERROR_NOT_SYMBOL, t_env);
    e2e_atom_test("((lambda (x y) (* x y)) 3 4)", TYPE_FIXNUM, 12, t_env);
    char* lam_1_few = "((lambda (x y) (* x y)) 3)";
    e2e_atom_test(lam_1_few, TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    char* lam_1_many = "((lambda (x y) (* x y)) 3 4 5)";
    e2e_atom_test(lam_1_many, TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    return;
}

void end_to_end_setvar_tests(test_env* t_env) {
    printf("# set! #\n");
    e2e_atom_test("(set!)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(set! x)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(set! x 1)", TYPE_ERROR, EVAL_ERROR_UNDEF_SYM, t_env);
    e2e_atom_test("(set! 1 2)", TYPE_ERROR, EVAL_ERROR_NOT_SYMBOL, t_env);
    char* set_line1 = "(define x 1)";
    char* set_line2 = "(set! x (+ 1 1))";
    char* set_line3 = "(+ x 1)";
    char* set_line4 = "(set! x (/ 0))";
    char* set_lines[] = {set_line1, set_line2, set_line3};
    char* set_lines2[] = {set_line1, set_line4};
    e2e_multiline_atom_test(set_lines2, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_DIV_ZERO, t_env);
    e2e_multiline_atom_test(set_lines, 2, TYPE_VOID, 0, t_env);
    e2e_multiline_atom_test(set_lines, 3, TYPE_FIXNUM, 3, t_env);
    return;
}

void end_to_end_define_tests(test_env* t_env) {
    printf("# define #\n");
    e2e_atom_test("(define)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define x)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define 1 2)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(define x (/ 0))", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    char* def_line1 = "(define x (+ 1 2))";
    char* def_line2 = "(+ x 10)";
    char* def_lines[] = {def_line1, def_line2};
    e2e_multiline_atom_test(def_lines, 1, TYPE_VOID, 0, t_env);
    e2e_multiline_atom_test(def_lines, 2, TYPE_FIXNUM, 13, t_env);
    e2e_atom_test("(define ())", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define () 1)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(define (x))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define (x 1) 1)", TYPE_ERROR, EVAL_ERROR_NOT_SYMBOL, t_env);
    e2e_atom_test("(define (x) (/ 0))", TYPE_VOID, 0, t_env);
    char* def_fnx_zero_param = "(define (x) 10)";
    char* def_fnx_zero_param_se = "(define (x) (+ 1 2))";
    char* def_fnx_one_param = "(define (x y) (* y 10))";
    char* def_fnx_two_param = "(define (x y z) (+ (* y 10) z))";
    char* def_fnx_body_error = "(define (x) (/ 0))";
    char* fnx_call_zero_arg = "(x)";
    char* fnx_call_one_arg = "(x 2)";
    char* fnx_call_two_arg = "(x 2 5)";
    char* fnx_call_three_arg = "(x 2 5 9)";
    char* fnx_call_arg_error = "(x (+ 2 null))";
    char* def_call_fnx_zero_param_zero_arg[] = {def_fnx_zero_param, \
                                                fnx_call_zero_arg};
    char* def_call_fnx_zero_param_s_expr_zero_arg[] = {def_fnx_zero_param_se, \
                                                       fnx_call_zero_arg};
    char* def_call_fnx_zero_param_one_arg[] = {def_fnx_zero_param_se, \
                                               fnx_call_one_arg};
    char* def_call_fnx_one_param_zero_arg[] = {def_fnx_one_param, \
                                               fnx_call_zero_arg};
    char* def_call_fnx_one_param_one_arg[] = {def_fnx_one_param, \
                                              fnx_call_one_arg};
    char* def_call_fnx_one_param_two_arg[] = {def_fnx_one_param, \
                                              fnx_call_two_arg};
    char* def_call_fnx_two_param_zero_arg[] = {def_fnx_two_param, \
                                               fnx_call_zero_arg};
    char* def_call_fnx_two_param_one_arg[] = {def_fnx_two_param, \
                                              fnx_call_one_arg};
    char* def_call_fnx_two_param_two_arg[] = {def_fnx_two_param, \
                                              fnx_call_two_arg};
    char* def_call_fnx_two_param_three_arg[] = {def_fnx_two_param, \
                                                fnx_call_three_arg};
    char* def_call_fnx_body_error_zero_arg[] = {def_fnx_body_error, \
                                                fnx_call_zero_arg};
    char* def_call_fnx_one_param_arg_error[] = {def_fnx_one_param, \
                                                fnx_call_arg_error};
    e2e_multiline_atom_test(def_call_fnx_zero_param_zero_arg, \
                            1, \
                            TYPE_VOID, \
                            0, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_zero_param_zero_arg, \
                            2, \
                            TYPE_FIXNUM, \
                            10, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_zero_param_s_expr_zero_arg, \
                            2, \
                            TYPE_FIXNUM, \
                            3, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_zero_param_one_arg, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_MANY_ARGS, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_one_param_zero_arg, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_FEW_ARGS, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_one_param_one_arg, \
                            2, \
                            TYPE_FIXNUM, \
                            20, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_one_param_two_arg, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_MANY_ARGS, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_two_param_zero_arg, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_FEW_ARGS, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_two_param_one_arg, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_FEW_ARGS, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_two_param_two_arg, \
                            2, \
                            TYPE_FIXNUM, \
                            25, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_two_param_three_arg, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_MANY_ARGS, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_body_error_zero_arg, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_DIV_ZERO, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_one_param_arg_error, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_NEED_NUM, \
                            t_env);
    return;
}

void end_to_end_quote_tests(test_env* t_env) {
    printf("# quote #\n");
    type err_t = TYPE_ERROR;
    typed_ptr undef = {.type=TYPE_UNDEF, .ptr={.idx=0}};
    typed_ptr* x_sym = install_symbol(t_env->env, "x", &undef);
    e2e_atom_test("(quote)", err_t, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(quote 1 2)", err_t, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(quote 1)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(quote x)", TYPE_SYMBOL, x_sym->ptr.idx, t_env);
    e2e_string_test("(quote \"test\")", "test", t_env);
    e2e_atom_test("(quote #f)", TYPE_BOOL, false, t_env);
    typed_ptr* add_sym = symbol_tp_from_name(t_env->env, "+");
    e2e_atom_test("(quote +)", TYPE_SYMBOL, add_sym->ptr.idx, t_env);
    typed_ptr* one = create_number_tp(1);
    typed_ptr* two = create_number_tp(2);
    typed_ptr* one_two_list[] = {one, two};
    e2e_s_expr_test("(quote (1 2))", one_two_list, 2, t_env);
    typed_ptr* divide_sym = symbol_tp_from_name(t_env->env, "/");
    typed_ptr* zero = create_number_tp(0);
    typed_ptr* div_zero_list[] = {divide_sym, zero};
    e2e_s_expr_test("(quote (/ 0))", div_zero_list, 2, t_env);
    free(one);
    free(two);
    free(x_sym);
    free(add_sym);
    free(divide_sym);
    free(zero);
    return;
}

void end_to_end_string_length_tests(test_env* t_env) {
    printf("# string-length #\n");
    type err_t = TYPE_ERROR;
    e2e_atom_test("(string-length)", err_t, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(string-length 1 2)", err_t, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(string-length 1)", err_t, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(string-length \"\")", TYPE_FIXNUM, 0, t_env);
    e2e_atom_test("(string-length \"hello\")", TYPE_FIXNUM, 5, t_env);
    e2e_atom_test("(string-length (/ 0))", err_t, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_string_equals_tests(test_env* t_env) {
    printf("# string=? #\n");
    type err_t = TYPE_ERROR;
    e2e_atom_test("(string=?)", err_t, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(string=? 1)", err_t, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(string=? \"\" 1)", err_t, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(string=? \"\" \"\")", TYPE_BOOL, true, t_env);
    e2e_atom_test("(string=? \"hello\" \"hello\")", TYPE_BOOL, true, t_env);
    e2e_atom_test("(string=? \"hello\" \"jello\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string=? \"he\" \"he\" \"he\")", TYPE_BOOL, true, t_env);
    e2e_atom_test("(string=? \"he\" \"he\" \"we\")", TYPE_BOOL, false, t_env);
    e2e_atom_test("(string=? (/ 0))", err_t, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_string_append_tests(test_env* t_env) {
    printf("# string-append #\n");
    type err_t = TYPE_ERROR;
    e2e_string_test("(string-append)", "", t_env);
    e2e_string_test("(string-append \"hello\")", "hello", t_env);
    e2e_string_test("(string-append \"he\" \"\" \"llo\")", "hello", t_env);
    e2e_atom_test("(string-append 1)", err_t, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(string-append (/ 0))", err_t, EVAL_ERROR_DIV_ZERO, t_env);
    return;
}

void end_to_end_scoping_tests(test_env* t_env) {
    printf("# scoping #\n");
    char define_a_one[] = "(define a 1)";
    char scope_define_no_param_a[] = "(define (f) (cond (#t (define a 2) a)))";
    char call_f[] = "(f)";
    char scope_mutate_no_param_a[] = "(define (g) (cond (#t (set! a 2) a)))";
    char call_g[] = "(g)";
    char scope_define_param_a[] = "(define (h a) (cond (#t (define a 2) a)))";
    char call_h[] = "(h 3)";
    char scope_mutate_param_a[] = "(define (j a) (cond (#t (set! a 2) a)))";
    char call_j[] = "(j 3)";
    // defining a without a shadowing variable in-scope
    char* define_a_no_shadow[] = {define_a_one, scope_define_no_param_a, call_f};
    e2e_multiline_atom_test(define_a_no_shadow, 3, TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(cond (#t a))", TYPE_FIXNUM, 1, t_env);
    // mutating a without a shadowing variable in-scope
    char* mutate_a_no_shadow[] = {define_a_one, scope_mutate_no_param_a, call_g};
    e2e_multiline_atom_test(mutate_a_no_shadow, 3, TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(cond (#t a))", TYPE_FIXNUM, 2, t_env);
    // defining a with a shadowing variable in-scope
    char* define_a_shadow[] = {define_a_one, scope_define_param_a, call_h};
    e2e_multiline_atom_test(define_a_shadow, 3, TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(cond (#t a))", TYPE_FIXNUM, 1, t_env);
    // mutating a with a shadowing variable in-scope
    char* mutate_a_shadow[] = {define_a_one, scope_mutate_param_a, call_j};
    e2e_multiline_atom_test(mutate_a_shadow, 3, TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(cond (#t a))", TYPE_FIXNUM, 1, t_env);
    return;
}

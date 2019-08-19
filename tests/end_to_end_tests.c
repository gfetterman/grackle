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
    bool pass = check_typed_ptr(output, t, (tp_value){.idx=val});
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    }
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void e2e_pair_test(char cmd[], \
                   typed_ptr** tp_list, \
                   unsigned int tp_list_len, \
                   test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    bool pass = check_pair(output, tp_list, tp_list_len, te->env);
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    }
    free(output);
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
    bool pass = check_s_expr(output, tp_list, tp_list_len, te->env);
    if (output->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    }
    free(output);
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
    bool pass = check_typed_ptr(output, t, (tp_value){.idx=val});
    free(output);
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
    e2e_atom_test("(+ 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## - ##\n");
    e2e_atom_test("(- 10 1 2 3)", TYPE_FIXNUM, 4, t_env);
    e2e_atom_test("(- 10 1)", TYPE_FIXNUM, 9, t_env);
    e2e_atom_test("(- 10)", TYPE_FIXNUM, -10, t_env);
    e2e_atom_test("(-)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(- 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(- 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## * ##\n");
    e2e_atom_test("(* 2 3 4)", TYPE_FIXNUM, 24, t_env);
    e2e_atom_test("(* 2 3)", TYPE_FIXNUM, 6, t_env);
    e2e_atom_test("(* 2)", TYPE_FIXNUM, 2, t_env);
    e2e_atom_test("(*)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(* 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(* 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## / ##\n");
    e2e_atom_test("(/ 100 4 5)", TYPE_FIXNUM, 5, t_env);
    e2e_atom_test("(/ 100 4)", TYPE_FIXNUM, 25, t_env);
    e2e_atom_test("(/ 1)", TYPE_FIXNUM, 1, t_env);
    e2e_atom_test("(/)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(/ 100 0)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(/ 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(/ 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(= 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## < ##\n");
    e2e_atom_test("(< 1 2)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(< 1 2 3)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(< 2 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(< 1 3 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(< 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(<)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(< 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(< 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## > ##\n");
    e2e_atom_test("(> 2 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(> 3 2 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(> 1 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(> 3 1 2)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(> 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(>)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(> 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(> 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(<= 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(>= 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(pair?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(pair? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(pair? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## list? ##\n");
    e2e_atom_test("(list? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? (list 1 2 3))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(list? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list? null)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(list? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(list?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(list? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(list? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## number? ##\n");
    e2e_atom_test("(number? 1)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(number? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(number?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(number? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(number? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## boolean? ##\n");
    e2e_atom_test("(boolean? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? #t)", TYPE_BOOL, true, t_env);
    e2e_atom_test("(boolean? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean? (cond))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(boolean?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(boolean? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(boolean? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    printf("## void? ##\n");
    e2e_atom_test("(void? 1)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? #t)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? (list 1 2 3))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? (cons 1 2))", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? null)", TYPE_BOOL, false, t_env);
    e2e_atom_test("(void? (cond))", TYPE_BOOL, true, t_env);
    e2e_atom_test("(void?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(void? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(void? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(and (-) #t #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(and #t (-) #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(or (-) #t #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(or #f (-) #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(not (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(car (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cdr (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(cond ((-) 1))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cond (#t (-)))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    return;
}

void end_to_end_cons_tests(test_env* t_env) {
    typed_ptr* num_1 = create_atom_tp(TYPE_FIXNUM, 1);
    typed_ptr* num_2 = create_atom_tp(TYPE_FIXNUM, 2);
    typed_ptr* cons_test_num_pair[] = {num_1, num_2};
    printf("# cons #\n");
    e2e_atom_test("(cons)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cons 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_pair_test("(cons 1 2)", cons_test_num_pair, 2, t_env);
    e2e_s_expr_test("(cons 1 null)", cons_test_num_pair, 1, t_env);
    e2e_s_expr_test("(cons 1 (cons 2 null))", cons_test_num_pair, 2, t_env);
    e2e_atom_test("(cons 1 2 3)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(cons (-) 2)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cons 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    e2e_atom_test("(list (-) 2 3)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(list 1 (-) 3)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    char* lam_prop_error = "((lambda (x) (* x 10)) (-))";
    e2e_atom_test(lam_prop_error, TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
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
    char* set_line4 = "(set! x (-))";
    char* set_lines[] = {set_line1, set_line2, set_line3};
    char* set_lines2[] = {set_line1, set_line4};
    e2e_multiline_atom_test(set_lines2, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_FEW_ARGS, t_env);
    e2e_multiline_atom_test(set_lines, 2, TYPE_VOID, 0, t_env);
    e2e_multiline_atom_test(set_lines, 3, TYPE_FIXNUM, 3, t_env);
    return;
}

void end_to_end_define_tests(test_env* t_env) {
    printf("# define #\n");
    e2e_atom_test("(define)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define x)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define 1 2)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(define x (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    char* def_line1 = "(define x (+ 1 2))";
    char* def_line2 = "(+ x 10)";
    char* def_lines[] = {def_line1, def_line2};
    e2e_multiline_atom_test(def_lines, 1, TYPE_VOID, 0, t_env);
    e2e_multiline_atom_test(def_lines, 2, TYPE_FIXNUM, 13, t_env);
    e2e_atom_test("(define ())", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define () 1)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(define (x))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define (x 1) 1)", TYPE_ERROR, EVAL_ERROR_NOT_SYMBOL, t_env);
    e2e_atom_test("(define (x) (-))", TYPE_VOID, 0, t_env);
    char* def_fnx_zero_param = "(define (x) 10)";
    char* def_fnx_zero_param_se = "(define (x) (+ 1 2))";
    char* def_fnx_one_param = "(define (x y) (* y 10))";
    char* def_fnx_two_param = "(define (x y z) (+ (* y 10) z))";
    char* def_fnx_body_error = "(define (x) (+ 1 null))";
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
                            EVAL_ERROR_NEED_NUM, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_one_param_arg_error, \
                            2, \
                            TYPE_ERROR, \
                            EVAL_ERROR_NEED_NUM, \
                            t_env);
    return;
}

#include "fundamentals.h"
#include "environment.h"
#include "test_functions.h"

void end_to_end_tests() {
    printf("end-to-end tests\n");
    printf("----------------\n");
    // setup
    environment* env = create_environment(0, 0);
    setup_environment(env);
    test_env* t_env = malloc(sizeof(test_env));
    if (t_env == NULL) {
        printf("malloc failed - aborting\n");
    }
    t_env->env = env;
    t_env->passed = 0;
    t_env->run = 0;
    // parsing
    printf("# parsing #\n");
    e2e_atom_test("", TYPE_ERROR, EVAL_ERROR_MISSING_PROCEDURE, t_env);
    e2e_atom_test("(", TYPE_ERROR, PARSE_ERROR_UNBAL_PAREN, t_env);
    e2e_atom_test(")", TYPE_ERROR, PARSE_ERROR_UNBAL_PAREN, t_env);
    e2e_atom_test("a", TYPE_ERROR, PARSE_ERROR_BARE_SYM, t_env);
    e2e_atom_test("()", TYPE_ERROR, EVAL_ERROR_MISSING_PROCEDURE, t_env);
    e2e_atom_test("(+ 1 1) (+ 1 1)", TYPE_ERROR, PARSE_ERROR_TOO_MANY, t_env);
    // arithmetic
    printf("# arithmetic #\n");
    //     addition
    printf("## + ##\n");
    e2e_atom_test("(+ 1 2 3 4)", TYPE_NUM, 10, t_env);
    e2e_atom_test("(+ 1 1)", TYPE_NUM, 2, t_env);
    e2e_atom_test("(+ 1)", TYPE_NUM, 1, t_env);
    e2e_atom_test("(+)", TYPE_NUM, 0, t_env);
    e2e_atom_test("(+ 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(+ 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     subtraction
    printf("## - ##\n");
    e2e_atom_test("(- 10 1 2 3)", TYPE_NUM, 4, t_env);
    e2e_atom_test("(- 10 1)", TYPE_NUM, 9, t_env);
    e2e_atom_test("(- 10)", TYPE_NUM, -10, t_env);
    e2e_atom_test("(-)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(- 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(- 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     multiplication
    printf("## * ##\n");
    e2e_atom_test("(* 2 3 4)", TYPE_NUM, 24, t_env);
    e2e_atom_test("(* 2 3)", TYPE_NUM, 6, t_env);
    e2e_atom_test("(* 2)", TYPE_NUM, 2, t_env);
    e2e_atom_test("(*)", TYPE_NUM, 1, t_env);
    e2e_atom_test("(* 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(* 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     division
    printf("## / ##\n");
    e2e_atom_test("(/ 100 4 5)", TYPE_NUM, 5, t_env);
    e2e_atom_test("(/ 100 4)", TYPE_NUM, 25, t_env);
    e2e_atom_test("(/ 1)", TYPE_NUM, 1, t_env);
    e2e_atom_test("(/)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(/ 100 0)", TYPE_ERROR, EVAL_ERROR_DIV_ZERO, t_env);
    e2e_atom_test("(/ 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(/ 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    // numerical comparisons
    printf("# comparisons #\n");
    //     =
    printf("## = ##\n");
    e2e_atom_test("(= 1 1 (- 2 1))", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(= 1 1)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(= 1 2)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(= 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(=)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(= 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(= 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     <
    printf("## < ##\n");
    e2e_atom_test("(< 1 2)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(< 1 2 3)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(< 2 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(< 1 3 2)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(< 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(<)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(< 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(< 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     >
    printf("## > ##\n");
    e2e_atom_test("(> 2 1)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(> 3 2 1)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(> 1 2)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(> 3 1 2)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(> 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(>)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(> 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(> 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     <=
    printf("## <= ##\n");
    e2e_atom_test("(<= 1 2)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(<= 1 1)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(<= 1 2 3)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(<= 1 2 2)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(<= 1 2 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(<= 2 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(<= 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(<=)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(<= 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(<= 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     >=
    printf("## >= ##\n");
    e2e_atom_test("(>= 2 1)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(>= 2 2)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(>= 3 2 1)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(>= 3 2 2)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(>= 3 2 3)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(>= 1 2)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(>= 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(>=)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(>= 1 #t)", TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    e2e_atom_test("(>= 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    // exit
    printf("# exit #\n");
    e2e_atom_test("(exit)", TYPE_ERROR, EVAL_ERROR_EXIT, t_env);
    e2e_atom_test("(exit 1)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    // predicates
    printf("# predicates #\n");
    //     pair?
    printf("## pair? ##\n");
    e2e_atom_test("(pair? 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(pair? #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(pair? (list 1 2 3))", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(pair? (cons 1 2))", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(pair? null)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(pair? (cond))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(pair?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(pair? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(pair? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     list?
    printf("## list? ##\n");
    e2e_atom_test("(list? 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(list? #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(list? (list 1 2 3))", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(list? (cons 1 2))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(list? null)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(list? (cond))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(list?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(list? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(list? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     number?
    printf("## number? ##\n");
    e2e_atom_test("(number? 1)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(number? #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(number? (list 1 2 3))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(number? (cons 1 2))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(number? null)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(number? (cond))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(number?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(number? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(number? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     boolean?
    printf("## boolean? ##\n");
    e2e_atom_test("(boolean? 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(boolean? #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(boolean? (list 1 2 3))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(boolean? (cons 1 2))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(boolean? null)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(boolean? (cond))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(boolean?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(boolean? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(boolean? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     void?
    printf("## void? ##\n");
    e2e_atom_test("(void? 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(void? #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(void? (list 1 2 3))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(void? (cons 1 2))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(void? null)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(void? (cond))", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(void?)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(void? 1 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(void? (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    // boolean operators
    printf("# boolean operators #\n");
    //     and
    printf("## and ##\n");
    e2e_atom_test("(and)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(and 1)", TYPE_NUM, 1, t_env);
    e2e_atom_test("(and #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(and #f)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(and #t #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(and #t #f)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(and #f #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(and #f #f)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(and #t #t #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(and #t #t #f)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(and #t #f #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(and #f #t #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(and (-) #t #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(and #t (-) #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     or
    printf("## or ##\n");
    e2e_atom_test("(or)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(or 1)", TYPE_NUM, 1, t_env);
    e2e_atom_test("(or #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #f)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(or #t #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #t #f)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #f #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #f #f)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(or #t #t #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #t #t #f)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #t #f #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #f #t #t)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(or #f #f #f)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(or (-) #t #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(or #f (-) #t)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    //     not
    printf("## not ##\n");
    e2e_atom_test("(not 1)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(not 0)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(not #t)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(not #f)", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(not (list 1 2 3))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(not (cons 1 2))", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(not null)", TYPE_BOOL, 0, t_env);
    e2e_atom_test("(not)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(not #t #t)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(not (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    // car/cdr
    printf("# car & cdr #\n");
    e2e_atom_test("(car (cons 1 2))", TYPE_NUM, 1, t_env);
    e2e_atom_test("(cdr (cons 1 2))", TYPE_NUM, 2, t_env);
    e2e_atom_test("(car (list 1 2 3))", TYPE_NUM, 1, t_env);
    e2e_atom_test("(car (cdr (list 1 2 3)))", TYPE_NUM, 2, t_env);
    e2e_atom_test("(car (cdr (cdr (list 1 2 3))))", TYPE_NUM, 3, t_env);
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
    // cond
    printf("# cond #\n");
    e2e_atom_test("(cond)", TYPE_VOID, 0, t_env);
    e2e_atom_test("(cond #t)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(cond (#t))", TYPE_BOOL, 1, t_env);
    e2e_atom_test("(cond (#t 1))", TYPE_NUM, 1, t_env);
    e2e_atom_test("(cond (#f 1))", TYPE_VOID, 0, t_env);
    e2e_atom_test("(cond (#t 1) (else 2))", TYPE_NUM, 1, t_env);
    e2e_atom_test("(cond (#f 1) (else 2))", TYPE_NUM, 2, t_env);
    char* empty_else = "(cond (#f 1) (else))";
    e2e_atom_test(empty_else, TYPE_ERROR, EVAL_ERROR_EMPTY_ELSE, t_env);
    char* nonterm_else = "(cond (#f 1) (else 2) (#t 3))";
    e2e_atom_test(nonterm_else, TYPE_ERROR, EVAL_ERROR_NONTERMINAL_ELSE, t_env);
    e2e_atom_test("(cond (#t (+ 1 1) (+ 2 2)))", TYPE_NUM, 4, t_env);
    e2e_atom_test("(cond ((-) 1))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cond (#t (-)))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    // cons
    printf("# cons #\n");
    e2e_atom_test("(cons)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cons 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    typed_ptr* num_1 = create_atom_tp(TYPE_NUM, 1);
    typed_ptr* num_2 = create_atom_tp(TYPE_NUM, 2);
    typed_ptr* cons_test_num_pair[] = {num_1, num_2};
    e2e_pair_test("(cons 1 2)", cons_test_num_pair, 2, t_env);
    e2e_sexpr_test("(cons 1 null)", cons_test_num_pair, 1, t_env);
    e2e_sexpr_test("(cons 1 (cons 2 null))", cons_test_num_pair, 2, t_env);
    e2e_atom_test("(cons 1 2 3)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("(cons (-) 2)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(cons 1 (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    // list
    printf("# list #\n");
    e2e_sexpr_test("(list)", cons_test_num_pair, 0, t_env);
    e2e_sexpr_test("(list 1)", cons_test_num_pair, 1, t_env);
    e2e_sexpr_test("(list (- 3 2) (+ 1 1))", cons_test_num_pair, 2, t_env);
    e2e_atom_test("(list (-) 2 3)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(list 1 (-) 3)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    free(num_1);
    free(num_2);
    // lambda
    printf("# lambda #\n");
    e2e_atom_test("(lambda)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(lambda 1)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(lambda 1 2)", TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE, t_env);
    e2e_atom_test("(lambda ())", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("((lambda () 1))", TYPE_NUM, 1, t_env);
    e2e_atom_test("((lambda () 1) 2)", TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    e2e_atom_test("((lambda () (+ 1 1)))", TYPE_NUM, 2, t_env);
    char* lam_0_few = "((lambda (x) (* x 10)))";
    e2e_atom_test(lam_0_few, TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("((lambda (x) (* x 10)) 3)", TYPE_NUM, 30, t_env);
    e2e_atom_test("((lambda (x) (* x 10)) (+ 1 2))", TYPE_NUM, 30, t_env);
    char* lam_body_error = "((lambda (x) (* x 10)) #t)";
    e2e_atom_test(lam_body_error, TYPE_ERROR, EVAL_ERROR_NEED_NUM, t_env);
    char* lam_prop_error = "((lambda (x) (* x 10)) (-))";
    e2e_atom_test(lam_prop_error, TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    char* lam_0_many = "((lambda (x) (* x 10)) 3 4)";
    e2e_atom_test(lam_0_many, TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    char* lam_not_id = "((lambda (x 2) (* x 10)) 3)";
    e2e_atom_test(lam_not_id, TYPE_ERROR, EVAL_ERROR_NOT_ID, t_env);
    e2e_atom_test("((lambda (x y) (* x y)) 3 4)", TYPE_NUM, 12, t_env);
    char* lam_1_few = "((lambda (x y) (* x y)) 3)";
    e2e_atom_test(lam_1_few, TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    char* lam_1_many = "((lambda (x y) (* x y)) 3 4 5)";
    e2e_atom_test(lam_1_many, TYPE_ERROR, EVAL_ERROR_MANY_ARGS, t_env);
    // set!
    printf("# set! #\n");
    e2e_atom_test("(set!)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(set! x)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(set! x 1)", TYPE_ERROR, EVAL_ERROR_UNDEF_SYM, t_env);
    e2e_atom_test("(set! 1 2)", TYPE_ERROR, EVAL_ERROR_NOT_ID, t_env);
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
    e2e_multiline_atom_test(set_lines, 3, TYPE_NUM, 3, t_env);
    // define
    printf("# define #\n");
    e2e_atom_test("(define)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define x)", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define 1 2)", TYPE_ERROR, EVAL_ERROR_NOT_ID, t_env);
    e2e_atom_test("(define x (-))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    char* def_line1 = "(define x (+ 1 2))";
    char* def_line2 = "(+ x 10)";
    char* def_lines[] = {def_line1, def_line2};
    e2e_multiline_atom_test(def_lines, 1, TYPE_VOID, 0, t_env);
    e2e_multiline_atom_test(def_lines, 2, TYPE_NUM, 13, t_env);
    e2e_atom_test("(define ())", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define () 1)", TYPE_ERROR, EVAL_ERROR_BAD_SYNTAX, t_env);
    e2e_atom_test("(define (x))", TYPE_ERROR, EVAL_ERROR_FEW_ARGS, t_env);
    e2e_atom_test("(define (x 1) 1)", TYPE_ERROR, EVAL_ERROR_NOT_ID, t_env);
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
    char* def_call_fnx_zero_param_sexpr_zero_arg[] = {def_fnx_zero_param_se, \
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
                            TYPE_NUM, \
                            10, \
                            t_env);
    e2e_multiline_atom_test(def_call_fnx_zero_param_sexpr_zero_arg, \
                            2, \
                            TYPE_NUM, \
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
                            TYPE_NUM, \
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
                            TYPE_NUM, \
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
    // reporting
    printf("-----\n");
    printf("tests passed/run: %u/%u\n", t_env->passed, t_env->run);
    delete_env_full(t_env->env);
    free(t_env);
    return;
}

int main() {
    end_to_end_tests();
    printf("\n----------------\n");
    printf("testing complete\n");
    return 64;
}

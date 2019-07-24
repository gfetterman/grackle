#include "unit_tests_fundamentals.h"

#define NUM_BUILTIN_TYPES 9
static type type_list[NUM_BUILTIN_TYPES] = {TYPE_UNDEF, \
                                            TYPE_ERROR, \
                                            TYPE_VOID, \
                                            TYPE_NUM, \
                                            TYPE_BOOL, \
                                            TYPE_BUILTIN, \
                                            TYPE_SEXPR, \
                                            TYPE_SYMBOL, \
                                            TYPE_FUNCTION};
#define TEST_NUM 42
#define TEST_SEXPR (s_expr*) 0xDEADBEEF
static tp_value TEST_NUM_TP_VAL = {.idx=TEST_NUM};
static tp_value TEST_SEXPR_TP_VAL = {.se_ptr=TEST_SEXPR};

void unit_tests_fundamentals(test_env* t_env) {
    printf("# fundamentals.c #\n");
    test_create_typed_ptr(t_env);
    test_create_atom_tp(t_env);
    test_create_s_expr_tp(t_env);
    test_create_error_tp(t_env);
    test_copy_typed_ptr(t_env);
    test_create_s_expr(t_env);
    test_create_empty_s_expr(t_env);
    test_copy_s_expr(t_env);
    test_delete_s_expr_recursive(t_env);
    test_s_expr_next(t_env);
    test_is_empty_list(t_env);
    test_is_false_literal(t_env);
    test_is_pair(t_env);
    return;
}

void test_create_typed_ptr(test_env* te) {
    print_test_announce("create_typed_ptr()");
    typed_ptr* out = NULL;
    bool pass = 1;
    for (unsigned int idx = 0; pass && (idx < NUM_BUILTIN_TYPES); idx++) {
        if (type_list[idx] == TYPE_SEXPR) {
            out = create_typed_ptr(type_list[idx], TEST_SEXPR_TP_VAL);
            if (!check_typed_ptr(out, TYPE_SEXPR, TEST_SEXPR_TP_VAL)) {
                pass = 0;
            }
        } else {
            out = create_typed_ptr(type_list[idx], TEST_NUM_TP_VAL);
            if (!check_typed_ptr(out, type_list[idx], TEST_NUM_TP_VAL)) {
                pass = 0;
            }
        }
        free(out);
    }
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_atom_tp(test_env* te) {
    print_test_announce("create_atom_tp()");
    typed_ptr* out = NULL;
    bool pass = 1;
    for (unsigned int idx = 0; pass && (idx < NUM_BUILTIN_TYPES); idx++) {
        if (type_list[idx] == TYPE_SEXPR) {
            continue;
        } else {
            out = create_atom_tp(type_list[idx], TEST_NUM);
            if (!check_typed_ptr(out, type_list[idx], TEST_NUM_TP_VAL)) {
                pass = 0;
            }
        }
        free(out);
    }
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_s_expr_tp(test_env* te) {
    print_test_announce("create_s_expr_tp()");
    bool pass = 1;
    typed_ptr* out = create_s_expr_tp(TEST_SEXPR);
    if (!check_typed_ptr(out, TYPE_SEXPR, TEST_SEXPR_TP_VAL)) {
        pass = 0;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_error_tp(test_env* te) {
    print_test_announce("create_error_tp()");
    bool pass = 1;
    typed_ptr* out = create_error_tp(EVAL_ERROR_EXIT);
    if (!check_typed_ptr(out, TYPE_ERROR, (tp_value){.idx=EVAL_ERROR_EXIT})) {
        pass = 0;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_copy_typed_ptr(test_env* te) {
    print_test_announce("copy_typed_ptr()");
    typed_ptr* original = create_atom_tp(TYPE_NUM, TEST_NUM);
    typed_ptr* copied = copy_typed_ptr(original);
    bool pass = 1;
    if (!match_typed_ptrs(original, copied)) {
        pass = 0;
    }
    original->ptr.idx = 100;
    if (match_typed_ptrs(original, copied)) {
        pass = 0;
    }
    free(original);
    free(copied);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_s_expr(test_env* te) {
    print_test_announce("create_s_expr()");
    typed_ptr* input_car = create_atom_tp(TYPE_NUM, TEST_NUM);
    typed_ptr* input_cdr = create_atom_tp(TYPE_NUM, 100);
    s_expr* out = create_s_expr(input_car, input_cdr);
    bool pass = 1;
    if (out == NULL || \
        out->car != input_car || \
        out->cdr != input_cdr) {
        pass = 0;
    }
    free(input_car);
    free(input_cdr);
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_empty_s_expr(test_env* te) {
    print_test_announce("create_empty_s_expr()");
    s_expr* out = create_empty_s_expr();
    bool pass = 1;
    if (out == NULL || \
        out->car != NULL || \
        out->cdr != NULL) {
        pass = 0;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_copy_s_expr(test_env* te) {
    print_test_announce("copy_s_expr()");
    // copy(NULL) -> NULL
    s_expr* original = NULL;
    s_expr* copied = copy_s_expr(original);
    bool pass = 1;
    if (copied != NULL) {
        pass = 0;
    }
    // copy(empty s-expression) -> new empty s-expression
    original = create_empty_s_expr();
    copied = copy_s_expr(original);
    if (copied == NULL || \
        !is_empty_list(copied) || \
        copied == original) {
        pass = 0;
    }
    free(original);
    free(copied);
    // copy(pair of atomic typed pointers) -> new pair of atomic typed pointers
    int first_value = 64;
    int second_value = 128;
    int third_value = 256;
    int fourth_value = 512;
    typed_ptr* first_atom = create_atom_tp(TYPE_NUM, first_value);
    typed_ptr* second_atom = create_atom_tp(TYPE_NUM, second_value);
    typed_ptr* third_atom = create_atom_tp(TYPE_NUM, third_value);
    typed_ptr* fourth_atom = create_atom_tp(TYPE_NUM, fourth_value);
    typed_ptr* atom_list[] = {first_atom, second_atom, third_atom, fourth_atom};
    original = create_s_expr(first_atom, second_atom);
    copied = copy_s_expr(original);
    typed_ptr* copied_tp = create_s_expr_tp(copied);
    if (!check_pair(copied_tp, atom_list, 2, NULL)) {
        pass = 0;
    }
    delete_s_expr_recursive(original, true);
    delete_s_expr_recursive(copied, true);
    free(copied_tp);
    // copy(one-atomic-element list) -> new one-atomic-element list
    original = create_s_expr(first_atom, \
                             create_s_expr_tp(create_empty_s_expr()));
    copied = copy_s_expr(original);
    copied_tp = create_s_expr_tp(copied);
    if (!check_sexpr(copied_tp, atom_list, 1, NULL)) {
        pass = 0;
    }
    delete_s_expr_recursive(original, true);
    delete_s_expr_recursive(copied, true);
    free(copied_tp);
    // copy(multi-atomic-element list) -> new multi-atomic-element list
    original = create_empty_s_expr();
    original = create_s_expr(fourth_atom, create_s_expr_tp(original));
    original = create_s_expr(third_atom, create_s_expr_tp(original));
    original = create_s_expr(second_atom, create_s_expr_tp(original));
    original = create_s_expr(first_atom, create_s_expr_tp(original));
    copied = copy_s_expr(original);
    copied_tp = create_s_expr_tp(copied);
    if (!check_sexpr(copied_tp, atom_list, 4, NULL)) {
        pass = 0;
    }
    delete_s_expr_recursive(original, true);
    delete_s_expr_recursive(copied, true);
    free(copied_tp);
    // copy(list with list elements) -> deep copy of list elements
    original = create_empty_s_expr();
    original = create_s_expr(fourth_atom, create_s_expr_tp(original));
    original = create_s_expr(third_atom, create_s_expr_tp(original));
    s_expr* branch = create_empty_s_expr();
    branch = create_s_expr(second_atom, create_s_expr_tp(branch));
    branch = create_s_expr(first_atom, create_s_expr_tp(branch));
    original = create_s_expr(create_s_expr_tp(branch), \
                             create_s_expr_tp(original));
    copied = copy_s_expr(original);
    if (copied == NULL || \
        !check_sexpr(copied->car, atom_list, 2, NULL) || \
        !check_sexpr(copied->cdr, atom_list + 2, 2, NULL)) {
        pass = 0;
    }
    delete_s_expr_recursive(original, true);
    delete_s_expr_recursive(copied, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

// these will have to remain smoke tests (plus valgrind checks)
void test_delete_s_expr_recursive(test_env* te) {
    print_test_announce("delete_s_expr_recursive()");
    // deleting a NULL se has no effect
    s_expr* se = NULL;
    delete_s_expr_recursive(se, true);
    delete_s_expr_recursive(se, false);
    // deleting a pair
    se = create_s_expr(create_atom_tp(TYPE_NUM, 64), \
                       create_atom_tp(TYPE_NUM, 128));
    delete_s_expr_recursive(se, true);
    // deleting a one-atomic-element list
    se = create_s_expr(create_atom_tp(TYPE_NUM, 64), \
                       create_s_expr_tp(create_empty_s_expr()));
    delete_s_expr_recursive(se, true);
    // deleting a multi-atomic-element list
    se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_NUM, 256), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_NUM, 128), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_NUM, 64), create_s_expr_tp(se));
    delete_s_expr_recursive(se, true);
    // deleting a one-list-element list
    s_expr* branch = create_empty_s_expr();
    branch = create_s_expr(create_atom_tp(TYPE_NUM, 256), \
                           create_s_expr_tp(branch));
    branch = create_s_expr(create_atom_tp(TYPE_NUM, 128), \
                           create_s_expr_tp(branch));
    branch = create_s_expr(create_atom_tp(TYPE_NUM, 64), \
                           create_s_expr_tp(branch));
    se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_NUM, 1024), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_NUM, 512), create_s_expr_tp(se));
    se = create_s_expr(create_s_expr_tp(branch), create_s_expr_tp(se));
    delete_s_expr_recursive(se, true);
    // deleting a multi-list-element list
    s_expr* branch_1 = create_empty_s_expr();
    branch_1 = create_s_expr(create_atom_tp(TYPE_NUM, 256), \
                             create_s_expr_tp(branch_1));
    branch_1 = create_s_expr(create_atom_tp(TYPE_NUM, 128), \
                             create_s_expr_tp(branch_1));
    branch_1 = create_s_expr(create_atom_tp(TYPE_NUM, 64), \
                             create_s_expr_tp(branch_1));
    s_expr* branch_2 = create_empty_s_expr();
    branch_2 = create_s_expr(create_atom_tp(TYPE_NUM, 2048), \
                             create_s_expr_tp(branch_2));
    branch_2 = create_s_expr(create_atom_tp(TYPE_NUM, 1024), \
                             create_s_expr_tp(branch_2));
    branch_2 = create_s_expr(create_atom_tp(TYPE_NUM, 512), \
                             create_s_expr_tp(branch_2));
    se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_NUM, 8192), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_NUM, 4096), create_s_expr_tp(se));
    se = create_s_expr(create_s_expr_tp(branch_2), create_s_expr_tp(se));
    se = create_s_expr(create_s_expr_tp(branch_1), create_s_expr_tp(se));
    delete_s_expr_recursive(se, true);
    print_test_result(true);
    te->passed++; // if it runs (and produces no valgrind errors) it passes
    te->run++;
    return;
}

void test_s_expr_next(test_env* te) {
    print_test_announce("s_expr_next()");
    int first_value = 64;
    int second_value = 128;
    int third_value = 256;
    int fourth_value = 512;
    typed_ptr* first_atom = create_atom_tp(TYPE_NUM, first_value);
    typed_ptr* second_atom = create_atom_tp(TYPE_NUM, second_value);
    typed_ptr* third_atom = create_atom_tp(TYPE_NUM, third_value);
    typed_ptr* fourth_atom = create_atom_tp(TYPE_NUM, fourth_value);
    typed_ptr* atom_list[] = {first_atom, second_atom, third_atom, fourth_atom};
    s_expr* se = create_empty_s_expr();
    se = create_s_expr(fourth_atom, create_s_expr_tp(se));
    se = create_s_expr(third_atom, create_s_expr_tp(se));
    se = create_s_expr(second_atom, create_s_expr_tp(se));
    se = create_s_expr(first_atom, create_s_expr_tp(se));
    bool pass = 1;
    s_expr* curr_se = se;
    for (unsigned int idx = 0; idx < 4; idx++) {
        if (!match_typed_ptrs(curr_se->car, atom_list[idx])) {
            pass = 0;
        }
        curr_se = s_expr_next(curr_se);
    }
    if (!is_empty_list(curr_se)) {
        pass = 0;
    }
    delete_s_expr_recursive(se, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_is_empty_list(test_env* te) {
    print_test_announce("is_empty_list()");
    s_expr* se = create_empty_s_expr();
    bool pass = 1;
    if (!is_empty_list(se)) {
        pass = 0;
    }
    se->car = create_atom_tp(TYPE_NUM, 64);
    se->cdr = create_atom_tp(TYPE_NUM, 128);
    if (is_empty_list(se)) {
        pass = 0;
    }
    free(se->cdr);
    se->cdr = create_s_expr_tp(create_empty_s_expr());
    if (is_empty_list(se)) {
        pass = 0;
    }
    delete_s_expr_recursive(se, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_is_false_literal(test_env* te) {
    print_test_announce("is_false_literal()");
    typed_ptr* tp = create_atom_tp(TYPE_BOOL, 0);
    bool pass = 1;
    if (!is_false_literal(tp)) {
        pass = 0;
    }
    tp->ptr.idx = 1;
    if (is_false_literal(tp)) {
        pass = 0;
    }
    tp->type = TYPE_NUM;
    if (is_false_literal(tp)) {
        pass = 0;
    }
    tp->type = TYPE_SEXPR;
    tp->ptr.se_ptr = NULL;
    if (is_false_literal(tp)) {
        pass = 0;
    }
    free(tp);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_is_pair(test_env* te) {
    print_test_announce("is_pair()");
    s_expr* se = create_empty_s_expr();
    bool pass = 1;
    if (is_pair(se)) {
        pass = 0;
    }
    free(se);
    se = create_s_expr(create_atom_tp(TYPE_NUM, 64), \
                       create_atom_tp(TYPE_NUM, 128));
    if (!is_pair(se)) {
        pass = 0;
    }
    free(se->cdr);
    se->cdr = create_s_expr_tp(create_empty_s_expr());
    if (is_pair(se)) {
        pass = 0;
    }
    delete_s_expr_recursive(se, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

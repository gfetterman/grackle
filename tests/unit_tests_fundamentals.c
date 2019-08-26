#include "unit_tests_fundamentals.h"

#define NUM_BUILTIN_TYPES 9
static type type_list[NUM_BUILTIN_TYPES] = {TYPE_UNDEF, \
                                            TYPE_ERROR, \
                                            TYPE_VOID, \
                                            TYPE_FIXNUM, \
                                            TYPE_BOOL, \
                                            TYPE_BUILTIN, \
                                            TYPE_S_EXPR, \
                                            TYPE_SYMBOL, \
                                            TYPE_FUNCTION};
#define TEST_NUM 42
#define TEST_S_EXPR (s_expr*) 0xDEADBEEF
static tp_value TEST_NUM_TP_VAL = {.idx=TEST_NUM};
static tp_value TEST_S_EXPR_TP_VAL = {.se_ptr=TEST_S_EXPR};

void unit_tests_fundamentals(test_env* t_env) {
    printf("# fundamentals.c #\n");
    test_create_typed_ptr(t_env);
    test_create_atom_tp(t_env);
    test_create_error_tp(t_env);
    test_create_void_tp(t_env);
    test_create_s_expr_tp(t_env);
    test_create_string_tp(t_env);
    test_copy_typed_ptr(t_env);
    test_create_s_expr(t_env);
    test_create_empty_s_expr(t_env);
    test_copy_s_expr(t_env);
    test_delete_s_expr_recursive(t_env);
    test_create_string(t_env);
    test_delete_string(t_env);
    test_s_expr_next(t_env);
    test_is_empty_list(t_env);
    test_is_false_literal(t_env);
    test_is_pair(t_env);
    return;
}

void test_create_typed_ptr(test_env* te) {
    print_test_announce("create_typed_ptr()");
    typed_ptr* out = NULL;
    bool pass = true;
    for (unsigned int idx = 0; pass && (idx < NUM_BUILTIN_TYPES); idx++) {
        if (type_list[idx] == TYPE_S_EXPR) {
            out = create_typed_ptr(type_list[idx], TEST_S_EXPR_TP_VAL);
            if (out == NULL || \
                out->type != TYPE_S_EXPR || \
                out->ptr.se_ptr != TEST_S_EXPR_TP_VAL.se_ptr) {
                pass = false;
            }
        } else {
            out = create_typed_ptr(type_list[idx], TEST_NUM_TP_VAL);
            if (out == NULL || \
                out->type != type_list[idx] || \
                out->ptr.idx != TEST_NUM_TP_VAL.idx) {
                pass = false;
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
    bool pass = true;
    for (unsigned int idx = 0; pass && (idx < NUM_BUILTIN_TYPES); idx++) {
        if (type_list[idx] == TYPE_S_EXPR) {
            continue;
        } else {
            out = create_atom_tp(type_list[idx], TEST_NUM);
            if (out == NULL || \
                out->type != type_list[idx] || \
                out->ptr.idx != TEST_NUM_TP_VAL.idx) {
                pass = false;
            }
        }
        free(out);
    }
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_error_tp(test_env* te) {
    print_test_announce("create_error_tp()");
    bool pass = true;
    typed_ptr* out = create_error_tp(TEST_ERROR_DUMMY);
    if (!check_error(out, TEST_ERROR_DUMMY)) {
        pass = false;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_void_tp(test_env* te) {
    print_test_announce("create_void_tp()");
    bool pass = true;
    typed_ptr* out = create_void_tp();
    if (out == NULL || \
        out->type != TYPE_VOID || \
        out->ptr.idx != 0) {
        pass = false;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_s_expr_tp(test_env* te) {
    print_test_announce("create_s_expr_tp()");
    bool pass = true;
    typed_ptr* out = create_s_expr_tp(TEST_S_EXPR);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        out->ptr.se_ptr != TEST_S_EXPR_TP_VAL.se_ptr) {
        pass = false;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_string_tp(test_env* te) {
    print_test_announce("create_string_tp()");
    bool pass = true;
    char test_str[] = "test string";
    String* str_obj = create_string(test_str);
    typed_ptr* out = create_string_tp(str_obj);
    if (out == NULL || \
        out->type != TYPE_STRING || \
        out->ptr.string != str_obj) {
        pass = false;
    }
    free(str_obj->contents);
    free(str_obj);
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_copy_typed_ptr(test_env* te) {
    print_test_announce("copy_typed_ptr()");
    typed_ptr* original = create_atom_tp(TYPE_FIXNUM, TEST_NUM);
    typed_ptr* copied = copy_typed_ptr(original);
    bool pass = true;
    if (!match_typed_ptrs(original, copied)) {
        pass = false;
    }
    original->ptr.idx = 100;
    if (match_typed_ptrs(original, copied)) {
        pass = false;
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
    typed_ptr* input_car = create_atom_tp(TYPE_FIXNUM, TEST_NUM);
    typed_ptr* input_cdr = create_atom_tp(TYPE_FIXNUM, 100);
    s_expr* out = create_s_expr(input_car, input_cdr);
    bool pass = true;
    if (out == NULL || \
        out->car != input_car || \
        out->cdr != input_cdr) {
        pass = false;
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
    bool pass = true;
    if (out == NULL || \
        out->car != NULL || \
        out->cdr != NULL) {
        pass = false;
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
    bool pass = true;
    if (copied != NULL) {
        pass = false;
    }
    // copy(empty s-expression) -> new empty s-expression
    original = create_empty_s_expr();
    copied = copy_s_expr(original);
    if (copied == NULL || \
        !is_empty_list(copied) || \
        copied == original) {
        pass = false;
    }
    free(original);
    free(copied);
    // copy(pair of atomic typed pointers) -> new pair of atomic typed pointers
    int first_value = 64;
    int second_value = 128;
    int third_value = 256;
    int fourth_value = 512;
    typed_ptr* first_atom = create_atom_tp(TYPE_FIXNUM, first_value);
    typed_ptr* second_atom = create_atom_tp(TYPE_FIXNUM, second_value);
    typed_ptr* third_atom = create_atom_tp(TYPE_FIXNUM, third_value);
    typed_ptr* fourth_atom = create_atom_tp(TYPE_FIXNUM, fourth_value);
    original = create_s_expr(first_atom, second_atom);
    copied = copy_s_expr(original);
    typed_ptr* copied_tp = create_s_expr_tp(copied);
    if (!match_s_exprs(original, copied)) {
        pass = false;
    }
    delete_s_expr_recursive(original, true);
    delete_s_expr_recursive(copied, true);
    free(copied_tp);
    // copy(one-atomic-element list) -> new one-atomic-element list
    first_atom = create_atom_tp(TYPE_FIXNUM, first_value);
    second_atom = create_atom_tp(TYPE_FIXNUM, second_value);
    original = create_s_expr(first_atom, \
                             create_s_expr_tp(create_empty_s_expr()));
    copied = copy_s_expr(original);
    copied_tp = create_s_expr_tp(copied);
    if (!match_s_exprs(original, copied)) {
        pass = false;
    }
    delete_s_expr_recursive(original, true);
    delete_s_expr_recursive(copied, true);
    free(copied_tp);
    // copy(multi-atomic-element list) -> new multi-atomic-element list
    first_atom = create_atom_tp(TYPE_FIXNUM, first_value);
    original = create_empty_s_expr();
    original = create_s_expr(fourth_atom, create_s_expr_tp(original));
    original = create_s_expr(third_atom, create_s_expr_tp(original));
    original = create_s_expr(second_atom, create_s_expr_tp(original));
    original = create_s_expr(first_atom, create_s_expr_tp(original));
    copied = copy_s_expr(original);
    copied_tp = create_s_expr_tp(copied);
    if (!match_s_exprs(original, copied)) {
        pass = false;
    }
    delete_s_expr_recursive(original, true);
    delete_s_expr_recursive(copied, true);
    free(copied_tp);
    // copy(list with list elements) -> deep copy of list elements
    first_atom = create_atom_tp(TYPE_FIXNUM, first_value);
    second_atom = create_atom_tp(TYPE_FIXNUM, second_value);
    third_atom = create_atom_tp(TYPE_FIXNUM, third_value);
    fourth_atom = create_atom_tp(TYPE_FIXNUM, fourth_value);
    original = create_empty_s_expr();
    original = create_s_expr(fourth_atom, create_s_expr_tp(original));
    original = create_s_expr(third_atom, create_s_expr_tp(original));
    s_expr* branch = create_empty_s_expr();
    branch = create_s_expr(second_atom, create_s_expr_tp(branch));
    branch = create_s_expr(first_atom, create_s_expr_tp(branch));
    original = create_s_expr(create_s_expr_tp(branch), \
                             create_s_expr_tp(original));
    copied = copy_s_expr(original);
    if (!match_s_exprs(original, copied)) {
        pass = false;
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
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 64), \
                       create_atom_tp(TYPE_FIXNUM, 128));
    delete_s_expr_recursive(se, true);
    // deleting a one-atomic-element list
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 64), \
                       create_s_expr_tp(create_empty_s_expr()));
    delete_s_expr_recursive(se, true);
    // deleting a multi-atomic-element list
    se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 256), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 128), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 64), create_s_expr_tp(se));
    delete_s_expr_recursive(se, true);
    // deleting a one-string-element list
    se = unit_list(create_string_tp(create_string("test string 1")));
    delete_s_expr_recursive(se, true);
    // deleting a multi-string-element list
    se = unit_list(create_string_tp(create_string("test string 1")));
    s_expr_append(se, create_string_tp(create_string("test string 2")));
    delete_s_expr_recursive(se, true);
    // deleting a string-containing pair
    se = create_s_expr(create_string_tp(create_string("test string 1")), \
                       create_string_tp(create_string("test string 2")));
    delete_s_expr_recursive(se, true);
    // deleting a one-list-element list
    s_expr* branch = create_empty_s_expr();
    branch = create_s_expr(create_atom_tp(TYPE_FIXNUM, 256), \
                           create_s_expr_tp(branch));
    branch = create_s_expr(create_atom_tp(TYPE_FIXNUM, 128), \
                           create_s_expr_tp(branch));
    branch = create_s_expr(create_atom_tp(TYPE_FIXNUM, 64), \
                           create_s_expr_tp(branch));
    se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 1024), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 512), create_s_expr_tp(se));
    se = create_s_expr(create_s_expr_tp(branch), create_s_expr_tp(se));
    delete_s_expr_recursive(se, true);
    // deleting a multi-list-element list
    s_expr* branch_1 = create_empty_s_expr();
    branch_1 = create_s_expr(create_atom_tp(TYPE_FIXNUM, 256), \
                             create_s_expr_tp(branch_1));
    branch_1 = create_s_expr(create_atom_tp(TYPE_FIXNUM, 128), \
                             create_s_expr_tp(branch_1));
    branch_1 = create_s_expr(create_atom_tp(TYPE_FIXNUM, 64), \
                             create_s_expr_tp(branch_1));
    s_expr* branch_2 = create_empty_s_expr();
    branch_2 = create_s_expr(create_atom_tp(TYPE_FIXNUM, 2048), \
                             create_s_expr_tp(branch_2));
    branch_2 = create_s_expr(create_atom_tp(TYPE_FIXNUM, 1024), \
                             create_s_expr_tp(branch_2));
    branch_2 = create_s_expr(create_atom_tp(TYPE_FIXNUM, 512), \
                             create_s_expr_tp(branch_2));
    se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 8192), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 4096), create_s_expr_tp(se));
    se = create_s_expr(create_s_expr_tp(branch_2), create_s_expr_tp(se));
    se = create_s_expr(create_s_expr_tp(branch_1), create_s_expr_tp(se));
    delete_s_expr_recursive(se, true);
    print_test_result(true);
    te->passed++; // if it runs (and produces no valgrind errors) it passes
    te->run++;
    return;
}

void test_create_string(test_env* te) {
    print_test_announce("create_string()");
    bool pass = true;
    char test_str[] = "test string";
    String* string_obj = create_string(test_str);
    if (string_obj == NULL || \
        string_obj->len != strlen(test_str) || \
        strcmp(string_obj->contents, test_str)) {
        pass = false;
    }
    free(string_obj->contents);
    free(string_obj);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

// smoke & Valgrind test
void test_delete_string(test_env* te) {
    print_test_announce("delete_string()");
    char test_str[] = "test string";
    String* string_obj = create_string(test_str);
    delete_string(string_obj);
    print_test_result(true);
    te->passed++;
    te->run++;
    return;
}

void test_s_expr_next(test_env* te) {
    print_test_announce("s_expr_next()");
    int first_value = 64;
    int second_value = 128;
    int third_value = 256;
    int fourth_value = 512;
    typed_ptr* first_atom = create_atom_tp(TYPE_FIXNUM, first_value);
    typed_ptr* second_atom = create_atom_tp(TYPE_FIXNUM, second_value);
    typed_ptr* third_atom = create_atom_tp(TYPE_FIXNUM, third_value);
    typed_ptr* fourth_atom = create_atom_tp(TYPE_FIXNUM, fourth_value);
    typed_ptr* atom_list[] = {first_atom, second_atom, third_atom, fourth_atom};
    s_expr* se = create_empty_s_expr();
    se = create_s_expr(fourth_atom, create_s_expr_tp(se));
    se = create_s_expr(third_atom, create_s_expr_tp(se));
    se = create_s_expr(second_atom, create_s_expr_tp(se));
    se = create_s_expr(first_atom, create_s_expr_tp(se));
    bool pass = true;
    s_expr* curr_se = se;
    for (unsigned int idx = 0; idx < 4; idx++) {
        if (!match_typed_ptrs(curr_se->car, atom_list[idx])) {
            pass = false;
        }
        curr_se = s_expr_next(curr_se);
    }
    if (!is_empty_list(curr_se)) {
        pass = false;
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
    bool pass = true;
    if (!is_empty_list(se)) {
        pass = false;
    }
    se->car = create_atom_tp(TYPE_FIXNUM, 64);
    se->cdr = create_atom_tp(TYPE_FIXNUM, 128);
    if (is_empty_list(se)) {
        pass = false;
    }
    free(se->cdr);
    se->cdr = create_s_expr_tp(create_empty_s_expr());
    if (is_empty_list(se)) {
        pass = false;
    }
    delete_s_expr_recursive(se, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_is_false_literal(test_env* te) {
    print_test_announce("is_false_literal()");
    typed_ptr* tp = create_atom_tp(TYPE_BOOL, false);
    bool pass = true;
    if (!is_false_literal(tp)) {
        pass = false;
    }
    tp->ptr.idx = true;
    if (is_false_literal(tp)) {
        pass = false;
    }
    tp->type = TYPE_FIXNUM;
    if (is_false_literal(tp)) {
        pass = false;
    }
    tp->type = TYPE_S_EXPR;
    tp->ptr.se_ptr = NULL;
    if (is_false_literal(tp)) {
        pass = false;
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
    bool pass = true;
    if (is_pair(se)) {
        pass = false;
    }
    free(se);
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 64), \
                       create_atom_tp(TYPE_FIXNUM, 128));
    if (!is_pair(se)) {
        pass = false;
    }
    free(se->cdr);
    se->cdr = create_s_expr_tp(create_empty_s_expr());
    if (is_pair(se)) {
        pass = false;
    }
    delete_s_expr_recursive(se, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

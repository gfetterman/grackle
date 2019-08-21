#include "unit_tests_test_utils.h"

void unit_tests_test_utils(test_env* te) {
    printf("# test_utils.c #\n");
    test_check_error(te);
    test_match_typed_ptrs(te);
    test_match_s_exprs(te);
    test_deep_match_typed_ptrs(te);
    test_unit_list(te);
    test_s_expr_append(te);
    test_create_number_tp(te);
    test_builtin_tp_from_name(te);
    test_symbol_tp_from_name(te);
    return;
}

void test_check_error(test_env* te) {
    print_test_announce("check_error()");
    bool pass = true;
    // check(NULL, <any error>) -> false
    typed_ptr* tp = NULL;
    if (check_error(tp, TEST_ERROR_DUMMY)) {
        pass = false;
    }
    // check(<non-error>, <any error>) -> false
    tp = create_number_tp((long) TEST_ERROR_DUMMY);
    if (check_error(tp, TEST_ERROR_DUMMY)) {
        pass = false;
    }
    free(tp);
    // check(<error>, <different error>) -> false
    tp = create_error_tp(TEST_ERROR_DUMMY);
    if (check_error(tp, PARSE_ERROR_NONE)) {
        pass = false;
    }
    free(tp);
    // check(<error>, <same error>) -> false
    tp = create_error_tp(TEST_ERROR_DUMMY);
    if (!check_error(tp, TEST_ERROR_DUMMY)) {
        pass = false;
    }
    free(tp);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_match_typed_ptrs(test_env* te) {
    print_test_announce("match_typed_ptrs()");
    bool pass = true;
    // match(NULL, NULL) -> true
    typed_ptr* first = NULL;
    typed_ptr* second = NULL;
    pass = match_typed_ptrs(first, second) && pass;
    // match(<something>, NULL) -> false
    first = create_number_tp(1);
    second = NULL;
    pass = !match_typed_ptrs(first, second) && pass;
    free(first);
    // match(NULL, <something>) -> false
    first = NULL;
    second = create_number_tp(1);
    pass = !match_typed_ptrs(first, second) && pass;
    free(second);
    // match(<number>, <bool>) -> false
    first = create_number_tp(1);
    second = create_atom_tp(TYPE_BOOL, 1);
    pass = !match_typed_ptrs(first, second) && pass;
    free(first);
    free(second);
    // match(<s-expr>, <same s-expr>) -> true
    first = create_s_expr_tp(unit_list(create_number_tp(1)));
    second = create_s_expr_tp(first->ptr.se_ptr);
    pass = match_typed_ptrs(first, second) && pass;
    free(first);
    free(second);
    // match(<s-expr>, <copied s-expr>) -> false
    first = create_s_expr_tp(unit_list(create_number_tp(1)));
    second = create_s_expr_tp(unit_list(create_number_tp(1)));
    pass = !match_typed_ptrs(first, second) && pass;
    delete_s_expr_recursive(first->ptr.se_ptr, true);
    free(first);
    delete_s_expr_recursive(second->ptr.se_ptr, true);
    free(second);
    // match(<number>, <copied number>) -> true
    first = create_number_tp(1);
    second = create_number_tp(1);
    pass = match_typed_ptrs(first, second) && pass;
    free(first);
    free(second);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_match_s_exprs(test_env* te) {
    print_test_announce("match_s_exprs()");
    bool pass = true;
    // match(NULL, NULL) -> true
    s_expr* first = NULL;
    s_expr* second = NULL;
    pass = match_s_exprs(first, second) && pass;
    // match(<something>, NULL) -> false
    first = unit_list(create_number_tp(1));
    second = NULL;
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    // match(NULL, <something>) -> false
    first = NULL;
    second = unit_list(create_number_tp(1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(second, true);
    // match('(), '()) -> true
    first = create_empty_s_expr();
    second = create_empty_s_expr();
    pass = match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1), '()) -> false
    first = unit_list(create_number_tp(1));
    second = create_empty_s_expr();
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(), '(1)) -> false
    first = create_empty_s_expr();
    second = unit_list(create_number_tp(1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1), <NULL car>) -> false
    first = unit_list(create_number_tp(1));
    second = create_s_expr(NULL, create_number_tp(1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match(<NULL car>, '(1)) -> false
    first = create_s_expr(NULL, create_number_tp(1));
    second = unit_list(create_number_tp(1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1), <NULL cdr>) -> false
    first = unit_list(create_number_tp(1));
    second = create_s_expr(create_number_tp(1), NULL);
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match(<NULL cdr>, '(1)) -> false
    first = create_s_expr(create_number_tp(1), NULL);
    second = unit_list(create_number_tp(1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1), '(#t)) -> false
    first = unit_list(create_number_tp(1));
    second = unit_list(create_atom_tp(TYPE_BOOL, 1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1 . 1), '(1 . #t)) -> false
    first = create_s_expr(create_number_tp(1), create_number_tp(1));
    second = create_s_expr(create_number_tp(1), create_atom_tp(TYPE_BOOL, 1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1 . 2), '(1 . 2)) -> true
    first = create_s_expr(create_number_tp(1), create_number_tp(2));
    second = create_s_expr(create_number_tp(1), create_number_tp(2));
    pass = match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1 2 3), '(1 2 3)) -> true
    first = unit_list(create_number_tp(1));
    s_expr_append(first, create_number_tp(2));
    s_expr_append(first, create_number_tp(3));
    second = unit_list(create_number_tp(1));
    s_expr_append(second, create_number_tp(2));
    s_expr_append(second, create_number_tp(3));
    pass = match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1 2 3), '(1)) -> false
    first = unit_list(create_number_tp(1));
    s_expr_append(first, create_number_tp(2));
    s_expr_append(first, create_number_tp(3));
    second = unit_list(create_number_tp(1));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1), '(1 2 3)) -> false
    first = unit_list(create_number_tp(1));
    second = unit_list(create_number_tp(1));
    s_expr_append(second, create_number_tp(2));
    s_expr_append(second, create_number_tp(3));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1 2 3), '(1 2 . 3)) -> false
    first = unit_list(create_number_tp(1));
    s_expr_append(first, create_number_tp(2));
    s_expr_append(first, create_number_tp(3));
    second = unit_list(create_number_tp(1));
    s_expr_append(second, create_s_expr_tp(create_s_expr(create_number_tp(2), \
                                                         create_number_tp(3))));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1 2 . 3), '(1 2 3)) -> false
    first = unit_list(create_number_tp(1));
    s_expr_append(first, create_s_expr_tp(create_s_expr(create_number_tp(2), \
                                                        create_number_tp(3))));
    second = unit_list(create_number_tp(1));
    s_expr_append(second, create_number_tp(2));
    s_expr_append(second, create_number_tp(3));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('(1 2 . 3), '(1 2 . 3)) -> true
    first = unit_list(create_number_tp(1));
    s_expr_append(first, create_s_expr_tp(create_s_expr(create_number_tp(2), \
                                                        create_number_tp(3))));
    second = unit_list(create_number_tp(1));
    s_expr_append(second, create_s_expr_tp(create_s_expr(create_number_tp(2), \
                                                         create_number_tp(3))));
    pass = match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('((1)), '((1))) -> true
    first = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    second = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    pass = match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('((1) 2), '((1) 2)) -> true
    first = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(first, create_number_tp(2));
    second = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(second, create_number_tp(2));
    pass = match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('((1 2 3) (4 5 6)), '((1 2 3) (4 5 6))) -> true
    first = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(2));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(first, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(6));
    second = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(2));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(second, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(6));
    pass = match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    // match('((1 2 3) (4 5 6)), '((1 10 3) (4 5 6))) -> false
    first = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(2));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(first, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(6));
    second = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(10));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(second, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(6));
    pass = !match_s_exprs(first, second) && pass;
    delete_s_expr_recursive(first, true);
    delete_s_expr_recursive(second, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_deep_match_typed_ptrs(test_env* te) {
    print_test_announce("deep_match_typed_ptrs()");
    bool pass = true;
    // match(NULL, NULL) -> true
    typed_ptr* first_tp = NULL;
    typed_ptr* second_tp = NULL;
    pass = deep_match_typed_ptrs(first_tp, second_tp) && pass;
    // match(<something>, NULL) -> false
    first_tp = create_number_tp(1);
    second_tp = NULL;
    pass = !deep_match_typed_ptrs(first_tp, second_tp) && pass;
    free(first_tp);
    // match(NULL, <something>) -> false
    first_tp = NULL;
    second_tp = create_number_tp(1);
    pass = !deep_match_typed_ptrs(first_tp, second_tp) && pass;
    free(second_tp);
    // match(1, #t) -> false
    first_tp = create_number_tp(1);
    second_tp = create_atom_tp(TYPE_BOOL, 1);
    pass = !deep_match_typed_ptrs(first_tp, second_tp) && pass;
    free(first_tp);
    free(second_tp);
    // match(1, 2) -> false
    first_tp = create_number_tp(1);
    second_tp = create_number_tp(2);
    pass = !deep_match_typed_ptrs(first_tp, second_tp) && pass;
    free(first_tp);
    free(second_tp);
    // match(1, 1) -> true
    first_tp = create_number_tp(1);
    second_tp = create_number_tp(1);
    pass = deep_match_typed_ptrs(first_tp, second_tp) && pass;
    free(first_tp);
    free(second_tp);
    // match('(), '()) -> true
    s_expr* first = create_empty_s_expr();
    s_expr* second = create_empty_s_expr();
    first_tp = create_s_expr_tp(first);
    second_tp = create_s_expr_tp(second);
    pass = deep_match_typed_ptrs(first_tp, second_tp) && pass;
    delete_s_expr_recursive(first, true);
    free(first_tp);
    delete_s_expr_recursive(second, true);
    free(second_tp);
    // match('(1 2 3), '(1 2 3)) -> true
    first = unit_list(create_number_tp(1));
    s_expr_append(first, create_number_tp(2));
    s_expr_append(first, create_number_tp(3));
    second = unit_list(create_number_tp(1));
    s_expr_append(second, create_number_tp(2));
    s_expr_append(second, create_number_tp(3));
    first_tp = create_s_expr_tp(first);
    second_tp = create_s_expr_tp(second);
    pass = deep_match_typed_ptrs(first_tp, second_tp) && pass;
    delete_s_expr_recursive(first, true);
    free(first_tp);
    delete_s_expr_recursive(second, true);
    free(second_tp);
    // match('((1 2 3) (4 5 6)), '((1 2 3) (4 5 6))) -> true
    first = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(2));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(first, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(6));
    first_tp = create_s_expr_tp(first);
    second = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(2));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(second, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(6));
    second_tp = create_s_expr_tp(second);
    pass = deep_match_typed_ptrs(first_tp, second_tp) && pass;
    delete_s_expr_recursive(first, true);
    free(first_tp);
    delete_s_expr_recursive(second, true);
    free(second_tp);
    // match('((1 2 3) (4 5 6)), '((1 10 3) (4 5 6))) -> false
    first = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(2));
    s_expr_append(first->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(first, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(first)->car->ptr.se_ptr, create_number_tp(6));
    first_tp = create_s_expr_tp(first);
    second = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(10));
    s_expr_append(second->car->ptr.se_ptr, create_number_tp(3));
    s_expr_append(second, create_s_expr_tp(unit_list(create_number_tp(4))));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(5));
    s_expr_append(s_expr_next(second)->car->ptr.se_ptr, create_number_tp(6));
    second_tp = create_s_expr_tp(second);
    pass = !deep_match_typed_ptrs(first_tp, second_tp) && pass;
    delete_s_expr_recursive(first, true);
    free(first_tp);
    delete_s_expr_recursive(second, true);
    free(second_tp);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_unit_list(test_env* te) {
    print_test_announce("unit_list()");
    bool pass = true;
    // unit_list(create_number_tp(1)) -> '(1)
    s_expr* out = unit_list(create_number_tp(1));
    s_expr* expected = create_s_expr(create_number_tp(1), \
                                     create_s_expr_tp(create_empty_s_expr()));
    pass = match_s_exprs(out, expected) && pass;
    delete_s_expr_recursive(out, true);
    delete_s_expr_recursive(expected, true);
    // unit_list(create_s_expr_tp(unit_list(1))) -> '((1))
    out = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    expected = create_s_expr(create_s_expr_tp(unit_list(create_number_tp(1))), \
                             create_s_expr_tp(create_empty_s_expr()));
    pass = match_s_exprs(out, expected) && pass;
    delete_s_expr_recursive(out, true);
    delete_s_expr_recursive(expected, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_s_expr_append(test_env* te) {
    print_test_announce("s_expr_append()");
    bool pass = true;
    // append(empty, 1) -> '(1)
    s_expr* out = create_empty_s_expr();
    s_expr_append(out, create_number_tp(1));
    s_expr* expected = unit_list(create_number_tp(1));
    pass = match_s_exprs(out, expected) && pass;
    delete_s_expr_recursive(out, true);
    delete_s_expr_recursive(expected, true);
    // append(empty, empty) -> '(())
    out = create_empty_s_expr();
    s_expr_append(out, create_s_expr_tp(create_empty_s_expr()));
    expected = unit_list(create_s_expr_tp(create_empty_s_expr()));
    pass = match_s_exprs(out, expected) && pass;
    delete_s_expr_recursive(out, true);
    delete_s_expr_recursive(expected, true);
    // append('(1), 2) -> '(1 2)
    out = unit_list(create_number_tp(1));
    s_expr_append(out, create_number_tp(2));
    expected = create_s_expr(create_number_tp(2), \
                             create_s_expr_tp(create_empty_s_expr()));
    expected = create_s_expr(create_number_tp(1), create_s_expr_tp(expected));
    pass = match_s_exprs(out, expected) && pass;
    delete_s_expr_recursive(out, true);
    delete_s_expr_recursive(expected, true);
    // append('(1 2 3), 4) -> '(1 2 3 4)
    out = unit_list(create_number_tp(1));
    s_expr_append(out, create_number_tp(2));
    s_expr_append(out, create_number_tp(3));
    s_expr_append(out, create_number_tp(4));
    expected = create_s_expr(create_number_tp(4), \
                             create_s_expr_tp(create_empty_s_expr()));
    expected = create_s_expr(create_number_tp(3), create_s_expr_tp(expected));
    expected = create_s_expr(create_number_tp(2), create_s_expr_tp(expected));
    expected = create_s_expr(create_number_tp(1), create_s_expr_tp(expected));
    pass = match_s_exprs(out, expected) && pass;
    delete_s_expr_recursive(out, true);
    delete_s_expr_recursive(expected, true);
    // append('(1 2), '(3 4)) -> '(1 2 (3 4))
    out = unit_list(create_number_tp(1));
    s_expr_append(out, create_number_tp(2));
    s_expr* sublist = unit_list(create_number_tp(3));
    s_expr_append(sublist, create_number_tp(4));
    s_expr_append(out, create_s_expr_tp(sublist));
    expected = create_s_expr(create_s_expr_tp(copy_s_expr(sublist)), \
                             create_s_expr_tp(create_empty_s_expr()));
    expected = create_s_expr(create_number_tp(2), create_s_expr_tp(expected));
    expected = create_s_expr(create_number_tp(1), create_s_expr_tp(expected));
    pass = match_s_exprs(out, expected) && pass;
    delete_s_expr_recursive(out, true);
    delete_s_expr_recursive(expected, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_number_tp(test_env* te) {
    print_test_announce("create_number_tp()");
    bool pass = true;
    // -10
    typed_ptr* out = create_number_tp(-10);
    typed_ptr* expected = create_typed_ptr(TYPE_FIXNUM, (tp_value){.idx=-10});
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    free(expected);
    // 0
    out = create_number_tp(0);
    expected = create_typed_ptr(TYPE_FIXNUM, (tp_value){.idx=0});
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    free(expected);
    // 10
    out = create_number_tp(10);
    expected = create_typed_ptr(TYPE_FIXNUM, (tp_value){.idx=10});
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    free(expected);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_builtin_tp_from_name(test_env* te) {
    print_test_announce("builtin_tp_from_name()");
    Environment* env = create_environment(0, 0);
    blind_install_symbol(env, \
                         "test_builtin", \
                         &(typed_ptr){.type=TYPE_BUILTIN, \
                                      .ptr={.idx=1000}});
    blind_install_symbol(env, \
                         "test_number", \
                         &(typed_ptr){.type=TYPE_FIXNUM, \
                                      .ptr={.idx=1000}});
    bool pass = true;
    // get("test_builtin") -> (TYPE_BUILTIN, 1000)
    typed_ptr* out = builtin_tp_from_name(env, "test_builtin");
    typed_ptr* expected = create_atom_tp(TYPE_BUILTIN, 1000);
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    free(expected);
    // get("test_number") -> NULL
    out = builtin_tp_from_name(env, "test_number");
    expected = NULL;
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    // get("test_absent") -> NULL
    out = builtin_tp_from_name(env, "test_absent");
    expected = NULL;
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_symbol_tp_from_name(test_env* te) {
    print_test_announce("symbol_tp_from_name()");
    Environment* env = create_environment(0, 0);
    blind_install_symbol(env, \
                         "test_builtin", \
                         &(typed_ptr){.type=TYPE_BUILTIN, \
                                      .ptr={.idx=1000}});
    blind_install_symbol(env, \
                         "test_number", \
                         &(typed_ptr){.type=TYPE_FIXNUM, \
                                      .ptr={.idx=1000}});
    bool pass = true;
    // get("test_builtin") -> (TYPE_SYMBOL, 0)
    typed_ptr* out = symbol_tp_from_name(env, "test_builtin");
    typed_ptr* expected = create_atom_tp(TYPE_SYMBOL, 0);
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    free(expected);
    // get("test_number") -> (TYPE_SYMBOL, 1)
    out = symbol_tp_from_name(env, "test_number");
    expected = create_atom_tp(TYPE_SYMBOL, 1);
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    free(expected);
    // get("test_absent") -> NULL
    out = symbol_tp_from_name(env, "test_absent");
    expected = NULL;
    pass = match_typed_ptrs(out, expected) && pass;
    free(out);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

#include "unit_tests_evaluate.h"

void unit_tests_evaluate(test_env* te) {
    printf("# evaluate.c #\n");
    test_collect_parameters(te);
    test_bind_args(te);
    test_make_eval_env(te);
    test_collect_arguments(te);
    test_eval_arithmetic(te);
    test_eval_comparison(te);
    test_eval_exit(te);
    test_eval_cons(te);
    test_eval_car_cdr(te);
    test_eval_list_construction(te);
    test_eval_and_or(te);
    test_eval_not(te);
    test_eval_list_pred(te);
    test_eval_atom_pred(te);
    test_eval_null_pred(te);
    test_eval_lambda(te);
    test_eval_cond(te);
    test_eval_define(te);
    test_eval_setvar(te);
    test_eval_builtin(te);
    test_eval_s_expr(te);
    test_eval_function(te);
    test_evaluate(te);
    return;
}

// test helpers

// NOTE: frees cmd AND expected
bool run_test_expect(typed_ptr* (*function)(const s_expr*, Environment*), \
                     s_expr* cmd, \
                     Environment* env, \
                     typed_ptr* expected) {
    typed_ptr* out = (*function)(cmd, env);
    bool passed = deep_match_typed_ptrs(out, expected);
    delete_s_expr_recursive(cmd, true);
    if (out != NULL && out->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    if (expected != NULL && expected->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(expected->ptr.se_ptr, true);
    }
    free(expected);
    return passed;
}

bool compare_expect_bool(Environment* env, \
                         builtin_code compare_op, \
                         unsigned int num_args, \
                         long args[], \
                         bool expected) {
    s_expr* cmd = unit_list(create_atom_tp(TYPE_BUILTIN, compare_op));
    for (unsigned int i = 0; i < num_args; i++) {
        s_expr_append(cmd, create_number_tp(args[i]));
    }
    typed_ptr* expected_tp = create_atom_tp(TYPE_BOOL, expected);
    return run_test_expect(eval_comparison, cmd, env, expected_tp);
}

// set up some useful "constants"

#define ADD builtin_tp_from_name(env, "+")
#define SUBTRACT builtin_tp_from_name(env, "-")
#define MULTIPLY builtin_tp_from_name(env, "*")
#define DIVIDE builtin_tp_from_name(env, "/")

#define CONS_SYM symbol_tp_from_name(env, "cons")
#define NULL_SYM symbol_tp_from_name(env, "null")
#define LIST_SYM symbol_tp_from_name(env, "list")
#define LAMBDA_SYM symbol_tp_from_name(env, "lambda")

typed_ptr undef = {.type=TYPE_UNDEF, .ptr={.idx=0}};

static inline s_expr* divide_zero_s_expr(Environment* env) {
    s_expr* se = unit_list(DIVIDE);
    s_expr_append(se, create_number_tp(0));
    return se;
}

static inline s_expr* add_one_one_s_expr(Environment* env) {
    s_expr* se = unit_list(ADD);
    s_expr_append(se, create_number_tp(1));
    s_expr_append(se, create_number_tp(1));
    return se;
}

static inline s_expr* list_one_two_s_expr(Environment* env) {
    s_expr* se = unit_list(LIST_SYM);
    s_expr_append(se, create_number_tp(1));
    s_expr_append(se, create_number_tp(2));
    return se;
}

// actual test functions

void test_collect_parameters(test_env* te) {
    print_test_announce("collect_parameters()");
    bool pass = true;
    // pass an empty list
    Environment* env = create_environment(0, 0);
    typed_ptr* se_tp = create_s_expr_tp(create_empty_s_expr());
    Symbol_Node* params = collect_parameters(se_tp, env);
    if (params != NULL) {
        pass = false;
    }
    // pass a list whose first car is not a symbol
    s_expr_append(se_tp->ptr.se_ptr, create_atom_tp(TYPE_FIXNUM, 1000));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_NOT_SYMBOL || \
        params->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a list whose middle car is not a symbol
    typed_ptr thousand = {.type=TYPE_FIXNUM, .ptr={.idx=1000}};
    typed_ptr *sym_1, *sym_2, *sym_3;
    sym_1 = install_symbol(env, "x", &thousand);
    sym_2 = install_symbol(env, "y", &thousand);
    sym_3 = install_symbol(env, "z", &thousand);
    se_tp = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_1));
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(&thousand));
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_2));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_NOT_SYMBOL || \
        params->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a list whose last car is not a symbol
    se_tp = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_1));
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_2));
    s_expr_append(se_tp->ptr.se_ptr, create_atom_tp(TYPE_FIXNUM, 1000));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_NOT_SYMBOL || \
        params->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a pair
    se_tp = create_s_expr_tp(create_s_expr(copy_typed_ptr(sym_1), \
                                           copy_typed_ptr(sym_2)));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_BAD_ARG_TYPE || \
        params->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a valid lone symbol
    se_tp = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_1));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        strcmp(params->name, "x") || \
        params->type != TYPE_UNDEF || \
        params->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a lone invalid symbol
    se_tp = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(se_tp->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 1000));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_BAD_SYMBOL || \
        params->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a list of valid symbols
    se_tp = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_1));
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_2));
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_3));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        strcmp(params->name, "x") || \
        params->type != TYPE_UNDEF || \
        params->next == NULL || \
        strcmp(params->next->name, "y") || \
        params->next->type != TYPE_UNDEF || \
        params->next->next == NULL || \
        strcmp(params->next->next->name, "z") || \
        params->next->next->type != TYPE_UNDEF || \
        params->next->next->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    delete_environment_full(env);
    free(sym_1);
    free(sym_2);
    free(sym_3);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_bind_args(test_env* te) {
    print_test_announce("bind_args()");
    bool pass = true;
    // no params, no args
    Function_Node* fn_no_params = create_function_node(0, "", NULL, NULL, NULL);
    typed_ptr* empty_args = create_s_expr_tp(create_empty_s_expr());
    Symbol_Node* bound_args = bind_args(NULL, fn_no_params, empty_args);
    if (bound_args != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    // no params, one arg
    typed_ptr* one_arg = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(one_arg->ptr.se_ptr, create_atom_tp(TYPE_FIXNUM, 1000));
    bound_args = bind_args(NULL, fn_no_params, one_arg);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_MANY_ARGS || \
        bound_args->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    // one param, no args
    Symbol_Node* one_param = create_symbol_node(0, \
                                                "x", \
                                                TYPE_UNDEF, \
                                                (tp_value){.idx=0});
    Function_Node* fn_1_param = create_function_node(0, "", one_param, NULL, NULL);
    bound_args = bind_args(NULL, fn_1_param, empty_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_FEW_ARGS || \
        bound_args->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    // one param, one arg
    bound_args = bind_args(NULL, fn_1_param, one_arg);
    if (bound_args == NULL || \
        strcmp(bound_args->name, "x") || \
        bound_args->type != TYPE_FIXNUM || \
        bound_args->value.idx != 1000 || \
        bound_args->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    // one param, two args
    typed_ptr* two_args = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(two_args->ptr.se_ptr, create_atom_tp(TYPE_FIXNUM, 1000));
    s_expr* se = create_empty_s_expr();
    s_expr_append(two_args->ptr.se_ptr, create_s_expr_tp(se));
    bound_args = bind_args(NULL, fn_1_param, two_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_MANY_ARGS || \
        bound_args->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    // two params, no args
    Symbol_Node* two_params = create_symbol_node(0, \
                                                 "x", \
                                                 TYPE_UNDEF, \
                                                 (tp_value){.idx=0});
    two_params->next = create_symbol_node(0, \
                                          "y", \
                                          TYPE_UNDEF, \
                                          (tp_value){.idx=0});
    Function_Node* fn_2_params = create_function_node(0, \
                                                      "", \
                                                      two_params, \
                                                      NULL, \
                                                      NULL);
    bound_args = bind_args(NULL, fn_2_params, empty_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_FEW_ARGS || \
        bound_args->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    // two params, one arg
    bound_args = bind_args(NULL, fn_2_params, one_arg);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_FEW_ARGS || \
        bound_args->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    // two params, two args
    bound_args = bind_args(NULL, fn_2_params, two_args);
    if (bound_args == NULL || \
        strcmp(bound_args->name, "y") || \
        bound_args->type != TYPE_S_EXPR || \
        bound_args->value.se_ptr == se || \
        !is_empty_list(bound_args->value.se_ptr) || \
        bound_args->next == NULL || \
        strcmp(bound_args->next->name, "x") || \
        bound_args->next->type != TYPE_FIXNUM || \
        bound_args->next->value.idx != 1000 || \
        bound_args->next->next != NULL) {
        pass = false;
    }
    for (Symbol_Node* arg = bound_args; arg != NULL; arg = arg->next) {
        if (arg->type == TYPE_S_EXPR) {
            delete_s_expr_recursive(arg->value.se_ptr, true);
        }
    }
    delete_symbol_node_list(bound_args);
    // two params, three args
    typed_ptr* three_args = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(three_args->ptr.se_ptr, create_atom_tp(TYPE_FIXNUM, 1000));
    s_expr_append(three_args->ptr.se_ptr, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(three_args->ptr.se_ptr, create_atom_tp(TYPE_FIXNUM, 2000));
    bound_args = bind_args(NULL, fn_2_params, three_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_MANY_ARGS || \
        bound_args->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(bound_args);
    free(fn_no_params->name);
    free(fn_no_params);
    delete_s_expr_recursive(empty_args->ptr.se_ptr, true);
    free(empty_args);
    delete_s_expr_recursive(one_arg->ptr.se_ptr, true);
    free(one_arg);
    delete_symbol_node_list(one_param);
    free(fn_1_param->name);
    free(fn_1_param);
    delete_s_expr_recursive(two_args->ptr.se_ptr, true);
    free(two_args);
    delete_symbol_node_list(two_params);
    free(fn_2_params->name);
    free(fn_2_params);
    delete_s_expr_recursive(three_args->ptr.se_ptr, true);
    free(three_args);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_make_eval_env(test_env* te) {
    print_test_announce("make_eval_env()");
    bool pass = true;
    // empty list of bound args
    Environment* env = create_environment(0, 0);
    blind_install_symbol(env, "x", &undef);
    blind_install_symbol(env, "y", &undef);
    blind_install_symbol(env, "z", &undef);
    Symbol_Node* args = NULL;
    Environment* out = make_eval_env(env, args);
    if (out == env || \
        symbol_lookup_name(env, "x") == symbol_lookup_name(out, "x") || \
        symbol_lookup_name(env, "y") == symbol_lookup_name(out, "y") || \
        symbol_lookup_name(env, "z") == symbol_lookup_name(out, "z") || \
        symbol_lookup_name(env, "x") == NULL || \
        symbol_lookup_name(env, "x")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "y") == NULL || \
        symbol_lookup_name(env, "y")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "z") == NULL || \
        symbol_lookup_name(env, "z")->type != TYPE_UNDEF || \
        symbol_lookup_name(out, "x") == NULL || \
        symbol_lookup_name(out, "x")->type != TYPE_UNDEF || \
        symbol_lookup_name(out, "y") == NULL || \
        symbol_lookup_name(out, "y")->type != TYPE_UNDEF || \
        symbol_lookup_name(out, "z") == NULL || \
        symbol_lookup_name(out, "z")->type != TYPE_UNDEF) {
        pass = false;
    }
    delete_environment_shared(out);
    // one bound arg
    args = create_symbol_node(0, "x", TYPE_FIXNUM, (tp_value){.idx=1000});
    out = make_eval_env(env, args);
    if (out == env || \
        symbol_lookup_name(env, "x") == symbol_lookup_name(out, "x") || \
        symbol_lookup_name(env, "y") == symbol_lookup_name(out, "y") || \
        symbol_lookup_name(env, "z") == symbol_lookup_name(out, "z") || \
        symbol_lookup_name(env, "x") == NULL || \
        symbol_lookup_name(env, "x")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "y") == NULL || \
        symbol_lookup_name(env, "y")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "z") == NULL || \
        symbol_lookup_name(env, "z")->type != TYPE_UNDEF || \
        symbol_lookup_name(out, "x") == NULL || \
        symbol_lookup_name(out, "x")->type != TYPE_FIXNUM || \
        symbol_lookup_name(out, "x")->value.idx != 1000 || \
        symbol_lookup_name(out, "y") == NULL || \
        symbol_lookup_name(out, "y")->type != TYPE_UNDEF || \
        symbol_lookup_name(out, "z") == NULL || \
        symbol_lookup_name(out, "z")->type != TYPE_UNDEF) {
        pass = false;
    }
    delete_environment_shared(out);
    // two bound args
    args->value.idx = 2000;
    s_expr* se = create_empty_s_expr();
    args->next = create_symbol_node(0, \
                                    "y", \
                                    TYPE_S_EXPR, \
                                    (tp_value){.se_ptr=se});
    out = make_eval_env(env, args);
    if (out == env || \
        symbol_lookup_name(env, "x") == symbol_lookup_name(out, "x") || \
        symbol_lookup_name(env, "y") == symbol_lookup_name(out, "y") || \
        symbol_lookup_name(env, "z") == symbol_lookup_name(out, "z") || \
        symbol_lookup_name(env, "x") == NULL || \
        symbol_lookup_name(env, "x")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "y") == NULL || \
        symbol_lookup_name(env, "y")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "z") == NULL || \
        symbol_lookup_name(env, "z")->type != TYPE_UNDEF || \
        symbol_lookup_name(out, "x") == NULL || \
        symbol_lookup_name(out, "x")->type != TYPE_FIXNUM || \
        symbol_lookup_name(out, "x")->value.idx != 2000 || \
        symbol_lookup_name(out, "y") == NULL || \
        symbol_lookup_name(out, "y")->type != TYPE_S_EXPR || \
        symbol_lookup_name(out, "y")->value.se_ptr != se || \
        !is_empty_list(symbol_lookup_name(out, "y")->value.se_ptr) || \
        symbol_lookup_name(out, "z") == NULL || \
        symbol_lookup_name(out, "z")->type != TYPE_UNDEF) {
        pass = false;
    }
    delete_environment_shared(out);
    delete_symbol_node_list(args);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_collect_arguments(test_env* te) {
    print_test_announce("collect_arguments()");
    bool pass = true;
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    s_expr empty_s_expr = {NULL, NULL};
    typed_ptr empty_se_tp = {.type=TYPE_S_EXPR, .ptr={.se_ptr=&empty_s_expr}};
    // arg s-expr is a pair -> error
    s_expr* call_pair = create_s_expr(create_atom_tp(TYPE_BUILTIN, 0), \
                                      create_atom_tp(TYPE_FIXNUM, 1000));
    typed_ptr* out = collect_arguments(call_pair, env, 0, -1, true);
    if (!check_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = false;
    }
    delete_s_expr_recursive(call_pair, true);
    free(out);
    // arg s-expr ends in a pair
    s_expr* call_bad_list = create_empty_s_expr();
    s_expr_append(call_bad_list, create_atom_tp(TYPE_BUILTIN, 0));
    s_expr_next(call_bad_list)->car = create_atom_tp(TYPE_FIXNUM, 1000);
    s_expr_next(call_bad_list)->cdr = create_atom_tp(TYPE_FIXNUM, 2000);
    out = collect_arguments(call_bad_list, env, 0, -1, true);
    if (!check_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = false;
    }
    delete_s_expr_recursive(call_bad_list, true);
    free(out);
    // empty arg s-expr, with min_args == 0 & max_args == 0
    s_expr* call_no_args = create_empty_s_expr();
    s_expr_append(call_no_args, create_atom_tp(TYPE_BUILTIN, 0));
    out = collect_arguments(call_no_args, env, 0, 0, true);
    if (!deep_match_typed_ptrs(out, &empty_se_tp) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    //    -> and without evaluate_all_args
    out = collect_arguments(call_no_args, env, 0, 0, false);
    if (!deep_match_typed_ptrs(out, &empty_se_tp) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // empty arg s-expr, with min_args == 0 & max_args > 0
    out = collect_arguments(call_no_args, env, 0, 1, true);
    if (!deep_match_typed_ptrs(out, &empty_se_tp) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // empty arg s-expr, with min_args == 0 & max_args < 0
    out = collect_arguments(call_no_args, env, 0, -1, true);
    if (!deep_match_typed_ptrs(out, &empty_se_tp) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // empty arg s-expr, with min_args > 0
    out = collect_arguments(call_no_args, env, 1, 1, true);
    if (!check_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    free(out);
    delete_s_expr_recursive(call_no_args, true);
    // one-elt arg s-expr, with min_args == 0 & max_args == 0
    s_expr* call_one_arg = create_empty_s_expr();
    s_expr_append(call_one_arg, create_atom_tp(TYPE_BUILTIN, 0));
    typed_ptr* value_1 = create_atom_tp(TYPE_FIXNUM, 1000);
    s_expr_append(call_one_arg, value_1);
    out = collect_arguments(call_one_arg, env, 0, 0, true);
    if (!check_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = false;
    }
    free(out);
    // one-elt arg s-expr, with min_args == 0 & max_args == 1
    out = collect_arguments(call_one_arg, env, 0, 1, true);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    //    -> and without evaluate_all_args
    out = collect_arguments(call_one_arg, env, 0, 1, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 0 & max_args == 2
    out = collect_arguments(call_one_arg, env, 0, 2, true);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 0 & max_args == -1
    out = collect_arguments(call_one_arg, env, 0, -1, true);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 1 & max_args == 1
    out = collect_arguments(call_one_arg, env, 1, 1, true);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 1 & max_args == 2
    out = collect_arguments(call_one_arg, env, 1, 2, true);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 1 & max_args == -1
    out = collect_arguments(call_one_arg, env, 1, -1, true);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 2 & max_args == 2
    out = collect_arguments(call_one_arg, env, 2, 2, true);
    if (!check_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    free(out);
    // one-elt arg s-expr, with min_args == 2 & max_args == -1
    out = collect_arguments(call_one_arg, env, 2, -1, true);
    if (!check_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    free(out);
    delete_s_expr_recursive(call_one_arg, true);
    // two-elt arg s-expr, with min_args == 0 & max_args == 0
    s_expr* call_two_args = create_empty_s_expr();
    s_expr_append(call_two_args, create_atom_tp(TYPE_BUILTIN, 0));
    value_1 = create_atom_tp(TYPE_FIXNUM, 1000);
    s_expr_append(call_two_args, value_1);
    typed_ptr* value_2 = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(value_2->ptr.se_ptr, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(call_two_args, value_2);
    out = collect_arguments(call_two_args, env, 0, 0, false);
    if (!check_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = false;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == 1
    out = collect_arguments(call_two_args, env, 0, 1, false);
    if (!check_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = false;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == 2
    out = collect_arguments(call_two_args, env, 0, 2, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == 3
    out = collect_arguments(call_two_args, env, 0, 3, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == -1
    out = collect_arguments(call_two_args, env, 0, -1, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == 1
    out = collect_arguments(call_two_args, env, 1, 1, false);
    if (!check_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = false;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == 2
    out = collect_arguments(call_two_args, env, 1, 2, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == 3
    out = collect_arguments(call_two_args, env, 1, 3, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == -1
    out = collect_arguments(call_two_args, env, 1, -1, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 2 & max_args == 2
    out = collect_arguments(call_two_args, env, 2, 2, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 2 & max_args == 3
    out = collect_arguments(call_two_args, env, 2, 3, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 2 & max_args == -1
    out = collect_arguments(call_two_args, env, 2, -1, false);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 3 & max_args == 3
    out = collect_arguments(call_two_args, env, 3, 3, false);
    if (!check_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 3 & max_args == -1
    out = collect_arguments(call_two_args, env, 3, -1, false);
    if (!check_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    free(out);
    delete_s_expr_recursive(call_two_args, true);
    // and with evaluating
    s_expr* addition_arg = create_empty_s_expr();
    s_expr_append(addition_arg, symbol_tp_from_name(env, "+"));
    s_expr_append(addition_arg, create_atom_tp(TYPE_FIXNUM, 1));
    s_expr_append(addition_arg, create_atom_tp(TYPE_FIXNUM, 2));
    s_expr* list_arg = create_empty_s_expr();
    s_expr_append(list_arg, symbol_tp_from_name(env, "list"));
    s_expr_append(list_arg, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(list_arg, create_atom_tp(TYPE_BOOL, true));
    s_expr* call_with_eval = create_empty_s_expr();
    s_expr_append(call_with_eval, create_atom_tp(TYPE_BUILTIN, 0));
    s_expr_append(call_with_eval, create_s_expr_tp(addition_arg));
    s_expr_append(call_with_eval, create_s_expr_tp(list_arg));
    s_expr* expected = create_empty_s_expr();
    s_expr_append(expected, create_atom_tp(TYPE_FIXNUM, 3));
    s_expr* list_result = create_empty_s_expr();
    s_expr_append(list_result, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(list_result, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(expected, create_s_expr_tp(list_result));
    out = collect_arguments(call_with_eval, env, 2, 2, true);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = false;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    delete_s_expr_recursive(expected, true);
    // with evaluating but an error
    s_expr* plus_error = create_empty_s_expr();
    s_expr_append(plus_error, symbol_tp_from_name(env, "+"));
    s_expr_append(plus_error, create_atom_tp(TYPE_FIXNUM, 1));
    s_expr_append(plus_error, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(call_with_eval, create_s_expr_tp(plus_error));
    out = collect_arguments(call_with_eval, env, 3, 3, true);
    if (!check_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = false;
    }
    free(out);
    delete_s_expr_recursive(call_with_eval, true);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_arithmetic(test_env* te) {
    print_test_announce("eval_arithmetic()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    bool pass = true;
    #define NUM_OPS 4
    typed_ptr* arith_ops[NUM_OPS] = {ADD, SUBTRACT, MULTIPLY, DIVIDE};
    // (+ . 1) -> EVAL_ERROR_ILLEGAL_PAIR
    s_expr* cmd = create_s_expr(ADD, create_number_tp(1));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 1 . 2) -> EVAL_ERROR_ILLEGAL_PAIR
    cmd = create_s_expr(create_number_tp(1), create_number_tp(2));
    cmd = create_s_expr(ADD, create_s_expr_tp(cmd));
    expected = create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 1 2 . 3) -> EVAL_ERROR_ILLEGAL_PAIR
    cmd = create_s_expr(create_number_tp(2), create_number_tp(3));
    cmd = create_s_expr(create_number_tp(1), create_s_expr_tp(cmd));
    cmd = create_s_expr(ADD, create_s_expr_tp(cmd));
    expected = create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+) -> 0
    cmd = unit_list(ADD);
    expected = create_number_tp(0);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (-) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(SUBTRACT);
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (*) -> 1
    cmd = unit_list(MULTIPLY);
    expected = create_number_tp(1);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(DIVIDE);
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 0) -> 0
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(0));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- 0) -> 0
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(0));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* 0) -> 0
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(0));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 0) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(0));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 2) -> 2
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- 2) -> -2
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(-2);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* 2) -> 2
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 2) -> 0 (since floats don't work yet - integer division w/flooring)
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 2 3) -> 5
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    expected = create_number_tp(5);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- 2 3) -> -1
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    expected = create_number_tp(-1);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* 2 3) -> 6
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    expected = create_number_tp(6);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* 2 0) -> 0
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 6 2) -> 3
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(6));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(3);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 5 2) -> 2 (again - integer division)
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(5));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 6 0) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(6));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 0 6) -> 0
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_number_tp(6));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 2 (+ 2 2)) -> 6
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(2));
    s_expr* add_two_two = unit_list(ADD);
    s_expr_append(add_two_two, create_number_tp(2));
    s_expr_append(add_two_two, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(add_two_two)));
    expected = create_number_tp(6);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- 2 (+ 2 2)) -> -2
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(add_two_two)));
    expected = create_number_tp(-2);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* 2 (+ 2 2)) -> 8
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(add_two_two)));
    expected = create_number_tp(8);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 20 (+ 2 2)) -> 5
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(20));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(add_two_two)));
    expected = create_number_tp(5);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 1 1 1) -> 3
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(3);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- 10 1 1) -> 8
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(10));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(8);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* 2 3 4) -> 24
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    s_expr_append(cmd, create_number_tp(4));
    expected = create_number_tp(24);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 72 3 4) -> 6
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(72));
    s_expr_append(cmd, create_number_tp(3));
    s_expr_append(cmd, create_number_tp(4));
    expected = create_number_tp(6);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 72 0 2) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(72));
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ 72 2 0) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(72));
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 0 <LONG_MIN>) -> <LONG_MIN>
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_number_tp(LONG_MIN));
    expected = create_number_tp(LONG_MIN);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 0 <LONG_MAX>) -> <LONG_MAX>
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_number_tp(LONG_MAX));
    expected = create_number_tp(LONG_MAX);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ -1 <LONG_MIN>) -> EVAL_ERROR_FIXNUM_UNDER
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(-1));
    s_expr_append(cmd, create_number_tp(LONG_MIN));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_UNDER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 1 <LONG_MAX>) -> EVAL_ERROR_FIXNUM_OVER
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(LONG_MAX));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- <LONG_MIN> 0) -> <LONG_MIN>
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(LONG_MIN));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_number_tp(LONG_MIN);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- <LONG_MAX> 0) -> <LONG_MAX>
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(LONG_MAX));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_number_tp(LONG_MAX);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- <LONG_MIN> 1) -> EVAL_ERROR_FIXNUM_UNDER
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(LONG_MIN));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_UNDER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (- <LONG_MAX> -1) -> EVAL_ERROR_FIXNUM_OVER
    cmd = unit_list(SUBTRACT);
    s_expr_append(cmd, create_number_tp(LONG_MAX));
    s_expr_append(cmd, create_number_tp(-1));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* <LONG_MIN> 1) -> <LONG_MIN>
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(LONG_MIN));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(LONG_MIN);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* <LONG_MAX> 1) -> <LONG_MAX>
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp(LONG_MAX));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(LONG_MAX);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* ((<LONG_MIN> / 2) - 1) 2) -> EVAL_ERROR_FIXNUM_UNDER
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp((LONG_MIN / 2) - 1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_UNDER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* ((<LONG_MAX> / 2) + 1) 2) -> EVAL_ERROR_FIXNUM_OVER
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp((LONG_MAX / 2) + 1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* ((<LONG_MIN> / 2) - 1) -2) -> EVAL_ERROR_FIXNUM_OVER
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp((LONG_MIN / 2) - 1));
    s_expr_append(cmd, create_number_tp(-2));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (* ((<LONG_MAX> / 2) + 1) -2) -> EVAL_ERROR_FIXNUM_UNDER
    cmd = unit_list(MULTIPLY);
    s_expr_append(cmd, create_number_tp((LONG_MAX / 2) + 2));
    s_expr_append(cmd, create_number_tp(-2));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_UNDER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (/ <LONG_MIN> -1) -> EVAL_ERROR_FIXNUM_OVER
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(LONG_MIN));
    s_expr_append(cmd, create_number_tp(-1));
    expected = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (<arith op> 1 #t) -> EVAL_ERROR_NEED_NUM
    // (<arith op> #t 1) -> EVAL_ERROR_NEED_NUM
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        cmd = unit_list(copy_typed_ptr(arith_ops[i]));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
        expected = create_error_tp(EVAL_ERROR_NEED_NUM);
        pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
        cmd = unit_list(copy_typed_ptr(arith_ops[i]));
        s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_error_tp(EVAL_ERROR_NEED_NUM);
        pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    }
    // (<arith op> 1 TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    // (<arith op> TEST_ERROR_DUMMY 1) -> TEST_ERROR_DUMMY
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        cmd = unit_list(copy_typed_ptr(arith_ops[i]));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
        expected = create_error_tp(TEST_ERROR_DUMMY);
        pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
        cmd = unit_list(copy_typed_ptr(arith_ops[i]));
        s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_error_tp(TEST_ERROR_DUMMY);
        pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    }
    // (<arith op> 1 (/ 0)) -> EVAL_ERROR_DIV_ZERO
    // (<arith op> (/ 0) 1) -> EVAL_ERROR_DIV_ZERO
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        cmd = unit_list(copy_typed_ptr(arith_ops[i]));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
        expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
        pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
        cmd = unit_list(copy_typed_ptr(arith_ops[i]));
        s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
        pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    }
    // (+ 1 null) -> EVAL_ERROR_NEED_NUM
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, NULL_SYM);
    expected = create_error_tp(EVAL_ERROR_NEED_NUM);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    // (+ 1 (list 1 2)) -> EVAL_ERROR_NEED_NUM
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_NEED_NUM);
    pass = run_test_expect(eval_arithmetic, cmd, env, expected) && pass;
    delete_environment_full(env);
    delete_s_expr_recursive(add_two_two, true);
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        free(arith_ops[i]);
    }
    #undef NUM_OPS
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_comparison(test_env* te) {
    print_test_announce("eval_comparison()");
    bool pass = true;
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr *eq_tp, *lt_tp, *gt_tp, *le_tp, *ge_tp;
    eq_tp = builtin_tp_from_name(env, "=");
    lt_tp = builtin_tp_from_name(env, "<");
    gt_tp = builtin_tp_from_name(env, ">");
    le_tp = builtin_tp_from_name(env, "<=");
    ge_tp = builtin_tp_from_name(env, ">=");
    builtin_code eq = BUILTIN_NUMBEREQ;
    builtin_code lt = BUILTIN_NUMBERLT;
    builtin_code gt = BUILTIN_NUMBERGT;
    builtin_code le = BUILTIN_NUMBERLE;
    builtin_code ge = BUILTIN_NUMBERGE;
    #define NUM_OPS 5
    typed_ptr* compare_ops[NUM_OPS] = {eq_tp, lt_tp, gt_tp, le_tp, ge_tp};
    // (= . 1) -> EVAL_ERROR_ILLEGAL_PAIR
    s_expr* cmd = create_s_expr(copy_typed_ptr(eq_tp), create_number_tp(1));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (= 1 . 2) -> EVAL_ERROR_ILLEGAL_PAIR
    cmd = create_s_expr(create_number_tp(1), create_number_tp(2));
    cmd = create_s_expr(copy_typed_ptr(eq_tp), create_s_expr_tp(cmd));
    expected = create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (= 1 2 . 3) -> EVAL_ERROR_ILLEGAL_PAIR
    cmd = create_s_expr(create_number_tp(2), create_number_tp(3));
    cmd = create_s_expr(create_number_tp(1), create_s_expr_tp(cmd));
    cmd = create_s_expr(copy_typed_ptr(eq_tp), create_s_expr_tp(cmd));
    expected = create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (<compare op>) -> EVAL_ERROR_FEW_ARGS
    // (<compare op> 1) -> EVAL_ERROR_FEW_ARGS
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    }
    // (= 1 1) -> #t
    pass = pass && compare_expect_bool(env, eq, 2, (long[2]){1, 1}, true);
    // (< 1 1) -> #f
    pass = pass && compare_expect_bool(env, lt, 2, (long[2]){1, 1}, false);
    // (> 1 1) -> #f
    pass = pass && compare_expect_bool(env, gt, 2, (long[2]){1, 1}, false);
    // (<= 1 1) -> #t
    pass = pass && compare_expect_bool(env, le, 2, (long[2]){1, 1}, true);
    // (<= 1 1) -> #t
    pass = pass && compare_expect_bool(env, ge, 2, (long[2]){1, 1}, true);
    // (= 0 0) -> #t
    pass = pass && compare_expect_bool(env, eq, 2, (long[2]){0, 0}, true);
    // (< 0 0) -> #f
    pass = pass && compare_expect_bool(env, lt, 2, (long[2]){0, 0}, false);
    // (> 0 0) -> #f
    pass = pass && compare_expect_bool(env, gt, 2, (long[2]){0, 0}, false);
    // (<= 0 0) -> #t
    pass = pass && compare_expect_bool(env, le, 2, (long[2]){0, 0}, true);
    // (>= 0 0) -> #t
    pass = pass && compare_expect_bool(env, ge, 2, (long[2]){0, 0}, true);
    // (= 1 2) -> #f
    pass = pass && compare_expect_bool(env, eq, 2, (long[2]){1, 2}, false);
    // (< 1 2) -> #t
    pass = pass && compare_expect_bool(env, lt, 2, (long[2]){1, 2}, true);
    // (> 1 2) -> #f
    pass = pass && compare_expect_bool(env, gt, 2, (long[2]){1, 2}, false);
    // (<= 1 2) -> #t
    pass = pass && compare_expect_bool(env, le, 2, (long[2]){1, 2}, true);
    // (>= 1 2) -> #f
    pass = pass && compare_expect_bool(env, ge, 2, (long[2]){1, 2}, false);
    // (= 0 1) -> #f
    pass = pass && compare_expect_bool(env, eq, 2, (long[2]){0, 1}, false);
    // (< 0 1) -> #t
    pass = pass && compare_expect_bool(env, lt, 2, (long[2]){0, 1}, true);
    // (> 0 1) -> #f
    pass = pass && compare_expect_bool(env, gt, 2, (long[2]){0, 1}, false);
    // (<= 0 1) -> #t
    pass = pass && compare_expect_bool(env, le, 2, (long[2]){0, 1}, true);
    // (>= 0 1) -> #f
    pass = pass && compare_expect_bool(env, ge, 2, (long[2]){0, 1}, false);
    // (= 1 -1) -> #f
    pass = pass && compare_expect_bool(env, eq, 2, (long[2]){1, -1}, false);
    // (< 1 -1) -> #f
    pass = pass && compare_expect_bool(env, lt, 2, (long[2]){1, -1}, false);
    // (> 1 -1) -> #t
    pass = pass && compare_expect_bool(env, gt, 2, (long[2]){1, -1}, true);
    // (<= 1 -1) -> #f
    pass = pass && compare_expect_bool(env, le, 2, (long[2]){1, -1}, false);
    // (>= 1 -1) -> #t
    pass = pass && compare_expect_bool(env, ge, 2, (long[2]){1, -1}, true);
    // (<compare op> 1 #t) -> EVAL_ERROR_NEED_NUM
    // (<compare op> #t 1) -> EVAL_ERROR_NEED_NUM
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
        expected = create_error_tp(EVAL_ERROR_NEED_NUM);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_error_tp(EVAL_ERROR_NEED_NUM);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    }
    // (= 1 (- 3 2)) -> #t
    cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(cmd, create_number_tp(1));
    s_expr* subtract_three_two = unit_list(SUBTRACT);
    s_expr_append(subtract_three_two, create_number_tp(3));
    s_expr_append(subtract_three_two, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(subtract_three_two)));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (< 1 (- 3 2)) -> #f
    cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(subtract_three_two)));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (> 1 (- 3 2)) -> #f
    cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(subtract_three_two)));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (<= 1 (- 3 2)) -> #t
    cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(subtract_three_two)));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (>= 1 (- 3 2)) -> #t
    cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(subtract_three_two)));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    // (= 1 1 1) -> #t
    pass = pass && compare_expect_bool(env, eq, 3, (long[3]){1, 1, 1}, true);
    // (= 1 1 2) -> #f
    pass = pass && compare_expect_bool(env, eq, 3, (long[3]){1, 1, 2}, false);
    // (= 1 2 1) -> #f
    pass = pass && compare_expect_bool(env, eq, 3, (long[3]){1, 2, 1}, false);
    // (= 2 1 1) -> #f
    pass = pass && compare_expect_bool(env, eq, 3, (long[3]){2, 1, 1}, false);
    // (< 1 2 3) -> #t
    pass = pass && compare_expect_bool(env, lt, 3, (long[3]){1, 2, 3}, true);
    // (< 1 3 2) -> #f
    pass = pass && compare_expect_bool(env, lt, 3, (long[3]){1, 3, 2}, false);
    // (< 3 1 2) -> #f
    pass = pass && compare_expect_bool(env, lt, 3, (long[3]){3, 1, 2}, false);
    // (< 1 0 2) -> #f
    pass = pass && compare_expect_bool(env, lt, 3, (long[3]){1, 0, 2}, false);
    // (> 3 2 1) -> #t
    pass = pass && compare_expect_bool(env, gt, 3, (long[3]){3, 2, 1}, true);
    // (> 2 3 1) -> #f
    pass = pass && compare_expect_bool(env, gt, 3, (long[3]){2, 3, 1}, false);
    // (> 2 1 3) -> #f
    pass = pass && compare_expect_bool(env, gt, 3, (long[3]){2, 1, 3}, false);
    // (> 1 2 0) -> #f
    pass = pass && compare_expect_bool(env, gt, 3, (long[3]){1, 2, 0}, false);
    // (<= 1 2 2) -> #t
    pass = pass && compare_expect_bool(env, le, 3, (long[3]){1, 2, 2}, true);
    // (<= 1 3 2) -> #f
    pass = pass && compare_expect_bool(env, le, 3, (long[3]){1, 3, 2}, false);
    // (<= 3 1 2) -> #f
    pass = pass && compare_expect_bool(env, le, 3, (long[3]){3, 1, 2}, false);
    // (<= 1 0 2) -> #f
    pass = pass && compare_expect_bool(env, le, 3, (long[3]){1, 0, 2}, false);
    // (>= 2 2 1) -> #t
    pass = pass && compare_expect_bool(env, ge, 3, (long[3]){2, 2, 1}, true);
    // (>= 3 1 2) -> #f
    pass = pass && compare_expect_bool(env, ge, 3, (long[3]){3, 1, 2}, false);
    // (>= 1 3 2) -> #f
    pass = pass && compare_expect_bool(env, ge, 3, (long[3]){1, 3, 2}, false);
    // (>= 1 2 0) -> #f
    pass = pass && compare_expect_bool(env, ge, 3, (long[3]){1, 2, 0}, false);
    // (<compare op> 1 TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    // (<compare op> TEST_ERROR_DUMMY 1) -> TEST_ERROR_DUMMY
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
        expected = create_error_tp(TEST_ERROR_DUMMY);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_error_tp(TEST_ERROR_DUMMY);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    }
    // (<compare op> 1 (/ 0)) -> EVAL_ERROR_DIV_ZERO
    // (<compare op> (/ 0) 1) -> EVAL_ERROR_DIV_ZERO
    for (unsigned int i = 0; i < NUM_OPS; i++) {
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
        expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
        cmd = unit_list(copy_typed_ptr(compare_ops[i]));
        s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
        pass = run_test_expect(eval_comparison, cmd, env, expected) && pass;
    }
    free(eq_tp);
    free(lt_tp);
    free(gt_tp);
    free(le_tp);
    free(ge_tp);
    delete_environment_full(env);
    delete_s_expr_recursive(subtract_three_two, true);
    #undef NUM_OPS
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_exit(test_env* te) {
    print_test_announce("eval_exit()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    bool pass = true;
    // (exit) -> EVAL_ERROR_EXIT
    s_expr* cmd = unit_list(builtin_tp_from_name(env, "exit"));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_EXIT);
    pass = run_test_expect(eval_exit, cmd, env, expected) && pass;
    // (exit 1) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(builtin_tp_from_name(env, "exit"));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_exit, cmd, env, expected) && pass;
    // (exit TEST_ERROR_DUMMY) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(builtin_tp_from_name(env, "exit"));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_exit, cmd, env, expected) && pass;
    // (exit (/ 0)) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(builtin_tp_from_name(env, "exit"));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_exit, cmd, env, expected) && pass;
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_cons(test_env* te) {
    print_test_announce("eval_cons()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* cons = builtin_tp_from_name(env, "cons");
    bool pass = true;
    // (cons) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(cons));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons 1) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons 1 #t) -> '(1 . #t)
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_s_expr_tp(create_s_expr(create_number_tp(1), \
                                              create_atom_tp(TYPE_BOOL, true)));
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons 1 #t 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons (- 3 1) #t) -> '(2 . #t)
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr* subtract_three_one = unit_list(SUBTRACT);
    s_expr_append(subtract_three_one, create_number_tp(3));
    s_expr_append(subtract_three_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(subtract_three_one)));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_s_expr_tp(create_s_expr(create_number_tp(2), \
                                              create_atom_tp(TYPE_BOOL, true)));
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons #t (- 3 1)) -> '(#t . 2)
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_s_expr_tp(copy_s_expr(subtract_three_one)));
    expected = create_s_expr_tp(create_s_expr(create_atom_tp(TYPE_BOOL, true), \
                                              create_number_tp(2)));
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons TEST_ERROR_DUMMY 1) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons 1 TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons (/ 0) 1) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons 1 (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    // (cons 1 (cons 2 (cons 3 null))) -> '(1 2 3)
    cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cmd, create_number_tp(1));
    s_expr* cons_two_list = unit_list(CONS_SYM);
    s_expr_append(cons_two_list, create_number_tp(2));
    s_expr* cons_three_list = unit_list(CONS_SYM);
    s_expr_append(cons_three_list, create_number_tp(3));
    s_expr_append(cons_three_list, NULL_SYM);
    s_expr_append(cons_two_list, create_s_expr_tp(cons_three_list));
    s_expr_append(cmd, create_s_expr_tp(cons_two_list));
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(2));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(3));
    pass = run_test_expect(eval_cons, cmd, env, expected) && pass;
    free(cons);
    delete_environment_full(env);
    delete_s_expr_recursive(subtract_three_one, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_car_cdr(test_env* te) {
    print_test_announce("eval_car_cdr()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* car = builtin_tp_from_name(env, "car");
    typed_ptr* cdr = builtin_tp_from_name(env, "cdr");
    bool pass = true;
    // (car) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(car));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (cdr) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(cdr));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (car 1) -> EVAL_ERROR_BAD_ARG_TYPE
    cmd = unit_list(copy_typed_ptr(car));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (cdr 1) -> EVAL_ERROR_BAD_ARG_TYPE
    cmd = unit_list(copy_typed_ptr(cdr));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (car (list 1 2)) -> 1
    cmd = unit_list(copy_typed_ptr(car));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (cdr (list 1 2)) -> '(2)
    cmd = unit_list(copy_typed_ptr(cdr));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_s_expr_tp(unit_list(create_number_tp(2)));
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (car (list 1) (list 2)) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(car));
    s_expr* list_one = unit_list(LIST_SYM);
    s_expr_append(list_one, create_number_tp(1));
    s_expr* list_two = unit_list(LIST_SYM);
    s_expr_append(list_two, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(list_one));
    s_expr_append(cmd, create_s_expr_tp(list_two));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (cdr (list 1) (list 2)) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(cdr));
    list_one = unit_list(LIST_SYM);
    s_expr_append(list_one, create_number_tp(1));
    list_two = unit_list(LIST_SYM);
    s_expr_append(list_two, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(list_one));
    s_expr_append(cmd, create_s_expr_tp(list_two));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (car TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(car));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (cdr TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(cdr));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (car (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(car));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    // (cdr (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cdr));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_car_cdr, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(car);
    free(cdr);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_list_construction(test_env* te) {
    print_test_announce("eval_list_construction()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* list = builtin_tp_from_name(env, "list");
    bool pass = true;
    // (list) -> '()
    s_expr* cmd = unit_list(copy_typed_ptr(list));
    typed_ptr* expected = create_s_expr_tp(create_empty_s_expr());
    pass = run_test_expect(eval_list_construction, cmd, env, expected) && pass;
    // (list 1) -> '(1)
    cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    pass = run_test_expect(eval_list_construction, cmd, env, expected) && pass;
    // (list 1 #t) -> '(1 #t)
    cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_BOOL, true));
    pass = run_test_expect(eval_list_construction, cmd, env, expected) && pass;
    // (list 1 #t (- 3 1)) -> '(1 #t 2)
    cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr* subtract_three_one = unit_list(SUBTRACT);
    s_expr_append(subtract_three_one, create_number_tp(3));
    s_expr_append(subtract_three_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(subtract_three_one));
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(2));
    pass = run_test_expect(eval_list_construction, cmd, env, expected) && pass;
    // (list TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_list_construction, cmd, env, expected) && pass;
    // (list (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_list_construction, cmd, env, expected) && pass;
    // (list (list 1) 2) -> '((1) 2)
    cmd = unit_list(copy_typed_ptr(list));
    s_expr* list_one = unit_list(LIST_SYM);
    s_expr_append(list_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(list_one));
    s_expr_append(cmd, create_number_tp(2));
    s_expr* lone_one = unit_list(create_number_tp(1));
    expected = create_s_expr_tp(unit_list(create_s_expr_tp(lone_one)));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(2));
    pass = run_test_expect(eval_list_construction, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(list);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_and_or(test_env* te) {
    print_test_announce("eval_and_or()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* and_builtin = builtin_tp_from_name(env, "and");
    typed_ptr* or_builtin = builtin_tp_from_name(env, "or");
    typed_ptr* cond_sym = symbol_tp_from_name(env, "cond");
    typed_ptr* define_sym = symbol_tp_from_name(env, "define");
    typed_ptr* boolpred_sym = symbol_tp_from_name(env, "boolean?");
    typed_ptr* x_sym;
    x_sym = install_symbol(env, "x", &undef);
    bool pass = true;
    // (and) -> #t
    s_expr* cmd = unit_list(copy_typed_ptr(and_builtin));
    typed_ptr* expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and 1) -> 1
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #f) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t #f) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #f #t) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #f #f) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and null #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, NULL_SYM);
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and 0 #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and (cond) #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(cond_sym))));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t 1) -> 1
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t (+ 1 1)) -> 2
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_s_expr_tp(add_one_one_s_expr(env)));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t #t #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t #f #t) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and 1 2 3 #f #t) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and (define x 1) #t (define x 2)) -> <void> AND SIDE EFFECT x == 2
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr* define_x_one = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_one, copy_typed_ptr(x_sym));
    s_expr_append(define_x_one, create_number_tp(1));
    s_expr* define_x_two = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_two, copy_typed_ptr(x_sym));
    s_expr_append(define_x_two, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(define_x_one));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_s_expr_tp(define_x_two));
    expected = create_void_tp();
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    typed_ptr* curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_FIXNUM || \
        curr_x_val->ptr.idx != 2) {
        pass = false;
    }
    free(curr_x_val);
    // (and (define x 3) #f (define x 4)) -> #f AND SIDE EFFECT x == 3
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr* define_x_three = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_three, copy_typed_ptr(x_sym));
    s_expr_append(define_x_three, create_number_tp(3));
    s_expr* define_x_four = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_four, copy_typed_ptr(x_sym));
    s_expr_append(define_x_four, create_number_tp(4));
    s_expr_append(cmd, create_s_expr_tp(define_x_three));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_s_expr_tp(define_x_four));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_FIXNUM || \
        curr_x_val->ptr.idx != 3) {
        pass = false;
    }
    free(curr_x_val);
    // (and TEST_ERROR_DUMMY #t) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and (/ 0) #t) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (and #t (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or 1) -> 1
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #t) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #t #t) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #t #f) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f #t) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f #f) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or null #t) -> null
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, NULL_SYM);
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_s_expr_tp(create_empty_s_expr());
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or 0 #t) -> 0
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or (cond) #t) -> <void>
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(cond_sym))));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_void_tp();
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f 1) -> 1
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f (+ 1 1)) -> 2
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_s_expr_tp(add_one_one_s_expr(env)));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f #f #f) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f #t #f) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f #f #f 1 #f) -> 1
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or (boolean? (define x 5)) (define x 6)) -> <void> AND SIDE EFFECT x = 6
    //   (using (bool? ...) here so the first define returns false and doesn't
    //    short-circuit out of the (or ...))
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr* define_x_five = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_five, copy_typed_ptr(x_sym));
    s_expr_append(define_x_five, create_number_tp(5));
    s_expr* bool_pred_define_x_five = unit_list(copy_typed_ptr(boolpred_sym));
    s_expr_append(bool_pred_define_x_five, create_s_expr_tp(define_x_five));
    s_expr* define_x_six = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_six, copy_typed_ptr(x_sym));
    s_expr_append(define_x_six, create_number_tp(6));
    s_expr_append(cmd, create_s_expr_tp(bool_pred_define_x_five));
    s_expr_append(cmd, create_s_expr_tp(define_x_six));
    expected = create_void_tp();
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_FIXNUM || \
        curr_x_val->ptr.idx != 6) {
        pass = false;
    }
    free(curr_x_val);
    // (or (define x 7) (define x 8)) -> <void> AND SIDE EFFECT x == 7
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr* define_x_seven = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_seven, copy_typed_ptr(x_sym));
    s_expr_append(define_x_seven, create_number_tp(7));
    s_expr* define_x_eight = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_eight, copy_typed_ptr(x_sym));
    s_expr_append(define_x_eight, create_number_tp(8));
    s_expr_append(cmd, create_s_expr_tp(define_x_seven));
    s_expr_append(cmd, create_s_expr_tp(define_x_eight));
    expected = create_void_tp();
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_FIXNUM || \
        curr_x_val->ptr.idx != 7) {
        pass = false;
    }
    free(curr_x_val);
    // (or TEST_ERROR_DUMMY #f) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or (/ 0) #f) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    // (or #f (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_and_or, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(and_builtin);
    free(or_builtin);
    free(cond_sym);
    free(define_sym);
    free(boolpred_sym);
    free(x_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_not(test_env* te) {
    print_test_announce("eval_not()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* not_builtin = builtin_tp_from_name(env, "not");
    typed_ptr* cond_sym = symbol_tp_from_name(env, "cond");
    bool pass = true;
    // (not) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(not_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not #t #t) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not #t) -> #f
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not 1) -> #f
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not 0) -> #f
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not null) -> #f
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, NULL_SYM);
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not (cond)) -> #f
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr* lone_cond = unit_list(copy_typed_ptr(cond_sym));
    s_expr_append(cmd, create_s_expr_tp(lone_cond));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not (list 1 2)) -> #f
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not #f) -> #t
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    delete_environment_full(env);
    free(not_builtin);
    free(cond_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_list_pred(test_env* te) {
    print_test_announce("eval_list_pred()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* listpred_builtin = builtin_tp_from_name(env, "list?");
    typed_ptr* cons_sym = symbol_tp_from_name(env, "cons");
    bool pass = true;
    // (list?) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(listpred_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? 1) -> #f
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? #t) -> #f
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? (cons 1 2)) -> #f
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr* cons_subexpr = unit_list(CONS_SYM);
    s_expr_append(cons_subexpr, create_number_tp(1));
    s_expr_append(cons_subexpr, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(cons_subexpr));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? (list 1 2)) -> #t
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? null) -> #t
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, NULL_SYM);
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    // (list? (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_list_pred, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(listpred_builtin);
    free(cons_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_atom_pred(test_env* te) {
    print_test_announce("eval_atom_pred()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* cons_sym = symbol_tp_from_name(env, "cons");
    typed_ptr fn_tp = {.type=TYPE_FUNCTION, .ptr={.idx=0}};
    typed_ptr* x_sym;
    x_sym = install_symbol(env, "x", &fn_tp);
    bool pass = true;
    #define NUM_TYPES 5
    builtin_code bi_codes[NUM_TYPES] = {BUILTIN_PAIRPRED, BUILTIN_NUMBERPRED, \
                                        BUILTIN_BOOLPRED, BUILTIN_VOIDPRED, \
                                        BUILTIN_PROCPRED};
    s_expr* cmd = NULL;
    typed_ptr* expected = NULL;
    // ([any_atomic]?) -> EVAL_ERROR_FEW_ARGS
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? 1 2) -> EVAL_ERROR_MANY_ARGS
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_number_tp(2));
        expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // there's currently no way to run these predicates on an undefined symbol,
    //   because they just get an EVAL_ERROR_UNDEF_SYM error
    // so we skip testing this predicate
    // ([any_atomic]? TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
        expected = create_error_tp(TEST_ERROR_DUMMY);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? <void>) -> #t if [void] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, create_void_tp());
        expected = create_atom_tp(TYPE_BOOL, bi_codes[i] == BUILTIN_VOIDPRED);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? 1) -> #t if [num] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, create_number_tp(1));
        expected = create_atom_tp(TYPE_BOOL, bi_codes[i] == BUILTIN_NUMBERPRED);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? #f) -> #t if [bool] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
        expected = create_atom_tp(TYPE_BOOL, bi_codes[i] == BUILTIN_BOOLPRED);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? +) -> #t if [builtin] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, ADD);
        expected = create_atom_tp(TYPE_BOOL, bi_codes[i] == BUILTIN_PROCPRED);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? (cons 1 2)) -> #t if [pair] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr* cons_subexpr = unit_list(CONS_SYM);
        s_expr_append(cons_subexpr, create_number_tp(1));
        s_expr_append(cons_subexpr, create_number_tp(2));
        s_expr_append(cmd, create_s_expr_tp(cons_subexpr));
        expected = create_atom_tp(TYPE_BOOL, bi_codes[i] == BUILTIN_PAIRPRED);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? (list 1 2)) -> #t if [pair] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
        expected = create_atom_tp(TYPE_BOOL, bi_codes[i] == BUILTIN_PAIRPRED);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? 'x) -> #t if [symbol] else #f
    //   there's not currently any way to obtain a bare symbol typed-pointer
    //   because the arguments are evaluated. Once I write (quote ...) I'll be
    //   able to test this...
    // (define ...) + ([any_atomic]? fn) -> #t if [function] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, copy_typed_ptr(x_sym));
        expected = create_atom_tp(TYPE_BOOL, bi_codes[i] == BUILTIN_PROCPRED);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? null) -> #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr_append(cmd, NULL_SYM);
        expected = create_atom_tp(TYPE_BOOL, false);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    // ([any_atomic]? (/ 0)) -> EVAL_ERROR_DIV_ZERO
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_BUILTIN, bi_codes[i]));
        s_expr* divide_subexpr = unit_list(DIVIDE);
        s_expr_append(divide_subexpr, create_number_tp(0));
        s_expr_append(cmd, create_s_expr_tp(divide_subexpr));
        expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
        pass = run_test_expect(eval_atom_pred, cmd, env, expected) && pass;
    }
    delete_environment_full(env);
    free(cons_sym);
    free(x_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_null_pred(test_env* te) {
    print_test_announce("eval_null_pred()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* nullpred_builtin = builtin_tp_from_name(env, "null?");
    typed_ptr *x_sym = install_symbol(env, "x", &undef);
    bool pass = true;
    // (null?) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? 1) -> #f
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? #t) -> #f
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? (list 1 2)) -> #f
    // (null? +) -> #f
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, ADD);
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? null) -> #t
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, NULL_SYM);
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? (list)) -> #t
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(LIST_SYM)));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? <void>) -> #f
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, create_void_tp());
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? (lambda () 1)) -> #f
    // (null? TEST_ERROR_DUMMY) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    // (null? (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(nullpred_builtin));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_null_pred, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(nullpred_builtin);
    free(x_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_lambda(test_env* te) {
    print_test_announce("eval_lambda()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* lambda_builtin = builtin_tp_from_name(env, "lambda");
    typed_ptr *x_sym, *y_sym;
    x_sym = install_symbol(env, "x", &undef);
    y_sym = install_symbol(env, "y", &undef);
    bool pass = true;
    // (lambda) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(lambda_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda x) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda 1 1) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda (x 1) 2) -> EVAL_ERROR_NOT_SYMBOL
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr* x_one = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(x_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(x_one));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_SYMBOL);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda (<weird symbol number>) 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr* param_list = unit_list(create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_s_expr_tp(param_list));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda x 1) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda (x) 1 2) -> currently EVAL_ERROR_MANY_ARGS; should be ok
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr* lone_x = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(lone_x));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda null 1) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, NULL_SYM);
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_lambda, cmd, env, expected) && pass;
    // (lambda (x) 1) -> <#procedure> + side effects
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    lone_x = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(lone_x));
    typed_ptr* body = create_number_tp(1);
    s_expr_append(cmd, body);
    expected = create_atom_tp(TYPE_FUNCTION, 0);
    typed_ptr* out = eval_lambda(cmd, env);
    if (!match_typed_ptrs(out, expected)) {
        pass = false;
    }
    Function_Node* resulting_fn = function_lookup_index(env, expected);
    if (env->function_table->length != 1 || \
        resulting_fn == NULL || \
        strcmp(resulting_fn->name, "") || \
        resulting_fn->param_list == NULL || \
        strcmp(resulting_fn->param_list->name, "x") || \
        resulting_fn->param_list->next != NULL || \
        resulting_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(resulting_fn->body, body)) {
        pass = false;
    }
    delete_s_expr_recursive(cmd, true);
    free(expected);
    free(out);
    // (lambda () 1) -> <#procedure> + side effects
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, create_s_expr_tp(create_empty_s_expr()));
    body = create_number_tp(1);
    s_expr_append(cmd, body);
    expected = create_atom_tp(TYPE_FUNCTION, 1);
    out = eval_lambda(cmd, env);
    if (!match_typed_ptrs(out, expected)) {
        pass = false;
    }
    resulting_fn = function_lookup_index(env, expected);
    if (env->function_table->length != 2 || \
        resulting_fn == NULL || \
        strcmp(resulting_fn->name, "") || \
        resulting_fn->param_list != NULL || \
        resulting_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(resulting_fn->body, body)) {
        pass = false;
    }
    delete_s_expr_recursive(cmd, true);
    free(expected);
    free(out);
    // (lambda (x y) (+ x y)) -> <#procedure> + side effects
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr* x_and_y = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(x_and_y, copy_typed_ptr(y_sym));
    s_expr_append(cmd, create_s_expr_tp(x_and_y));
    body = create_s_expr_tp(unit_list(ADD));
    s_expr_append(body->ptr.se_ptr, copy_typed_ptr(x_sym));
    s_expr_append(body->ptr.se_ptr, copy_typed_ptr(y_sym));
    s_expr_append(cmd, body);
    expected = create_atom_tp(TYPE_FUNCTION, 2);
    out = eval_lambda(cmd, env);
    if (!match_typed_ptrs(out, expected)) {
        pass = false;
    }
    resulting_fn = function_lookup_index(env, expected);
    if (env->function_table->length != 3 || \
        resulting_fn == NULL || \
        strcmp(resulting_fn->name, "") || \
        resulting_fn->param_list == NULL || \
        strcmp(resulting_fn->param_list->name, "x") || \
        resulting_fn->param_list->next == NULL || \
        strcmp(resulting_fn->param_list->next->name, "y") || \
        resulting_fn->param_list->next->next != NULL || \
        resulting_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(resulting_fn->body, body)) {
        pass = false;
    }
    delete_s_expr_recursive(cmd, true);
    free(expected);
    free(out);
    // (lambda (x) (+ x y)) -> <#procedure> + side effects
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    lone_x = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(lone_x));
    body = create_s_expr_tp(unit_list(ADD));
    s_expr_append(body->ptr.se_ptr, copy_typed_ptr(x_sym));
    s_expr_append(body->ptr.se_ptr, copy_typed_ptr(y_sym));
    s_expr_append(cmd, body);
    expected = create_atom_tp(TYPE_FUNCTION, 3);
    out = eval_lambda(cmd, env);
    if (!match_typed_ptrs(out, expected)) {
        pass = false;
    }
    resulting_fn = function_lookup_index(env, expected);
    if (env->function_table->length != 4 || \
        resulting_fn == NULL || \
        strcmp(resulting_fn->name, "") || \
        resulting_fn->param_list == NULL || \
        strcmp(resulting_fn->param_list->name, "x") || \
        resulting_fn->param_list->next != NULL || \
        resulting_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(resulting_fn->body, body)) {
        pass = false;
    }
    delete_s_expr_recursive(cmd, true);
    free(expected);
    free(out);
    // (lambda () (/ 0)) -> <#procedure> + side effects
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, create_s_expr_tp(create_empty_s_expr()));
    body = create_s_expr_tp(unit_list(DIVIDE));
    s_expr_append(body->ptr.se_ptr, create_number_tp(0));
    s_expr_append(cmd, body);
    expected = create_atom_tp(TYPE_FUNCTION, 4);
    out = eval_lambda(cmd, env);
    if (!match_typed_ptrs(out, expected)) {
        pass = false;
    }
    resulting_fn = function_lookup_index(env, expected);
    if (env->function_table->length != 5 || \
        resulting_fn == NULL || \
        strcmp(resulting_fn->name, "") || \
        resulting_fn->param_list != NULL || \
        resulting_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(resulting_fn->body, body)) {
        pass = false;
    }
    delete_s_expr_recursive(cmd, true);
    free(expected);
    free(out);
    delete_environment_full(env);
    free(lambda_builtin);
    free(x_sym);
    free(y_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_cond(test_env* te) {
    print_test_announce("eval_cond()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* cond_builtin = builtin_tp_from_name(env, "cond");
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr* equals_sym = symbol_tp_from_name(env, "=");
    bool pass = true;
    // (cond) -> <void>
    s_expr* cmd = unit_list(copy_typed_ptr(cond_builtin));
    typed_ptr* expected = create_void_tp();
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond 1) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond ()) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    s_expr* first_case = create_empty_s_expr();
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t)) -> #t
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (1)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (1 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_number_tp(1));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (() 1)) -> EVAL_ERROR_MISSING_PROCEDURE
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_s_expr_tp(create_empty_s_expr()));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (null 1)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(NULL_SYM);
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f)) -> <void>
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_void_tp();
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f 2)) -> <void>
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_void_tp();
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t 1) 2) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t 1) (#t 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    s_expr* second_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f 1) (#t 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t 1) (#f 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f 1) (#t 2) (#f 3)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    s_expr* third_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(third_case, create_number_tp(3));
    s_expr_append(cmd, create_s_expr_tp(third_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f 1) (#f 2) (#f 3)) -> <void>
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    third_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(third_case, create_number_tp(3));
    s_expr_append(cmd, create_s_expr_tp(third_case));
    expected = create_void_tp();
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (else 1)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f 1) (else)) -> EVAL_ERROR_EMPTY_ELSE
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_error_tp(EVAL_ERROR_EMPTY_ELSE);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f 1) (else 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t 1) (else 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (else 2) (#t 1)) -> EVAL_ERROR_NONTERMINAL_ELSE
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(second_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_error_tp(EVAL_ERROR_NONTERMINAL_ELSE);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond ((+ 1 1))) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = create_empty_s_expr();
    s_expr_append(first_case, create_s_expr_tp(add_one_one_s_expr(env)));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond ((= 1 1) (+ 1 1))) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = create_empty_s_expr();
    s_expr* equals_one_one = unit_list(copy_typed_ptr(equals_sym));
    s_expr_append(equals_one_one, create_number_tp(1));
    s_expr_append(equals_one_one, create_number_tp(1));
    s_expr_append(first_case, create_s_expr_tp(equals_one_one));
    s_expr_append(first_case, create_s_expr_tp(add_one_one_s_expr(env)));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#f (/ 0))) -> <void>
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_void_tp();
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t 1) ((/ 0) 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = create_empty_s_expr();
    s_expr_append(second_case, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t 1 (/ 0))) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(first_case, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t (/ 0) 1)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (TEST_ERROR_DUMMY 1)) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_error_tp(TEST_ERROR_DUMMY));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t TEST_ERROR_DUMMY)) -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_error_tp(TEST_ERROR_DUMMY));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond ((/ 0) 1)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = create_empty_s_expr();
    s_expr_append(first_case, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (cond (#t (/ 0))) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(cond_builtin);
    free(equals_sym);
    free(else_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_define(test_env* te) {
    print_test_announce("eval_define()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* define_builtin = builtin_tp_from_name(env, "define");
    typed_ptr *x_sym, *y_sym, *z_sym;
    x_sym = install_symbol(env, "x", &undef);
    y_sym = install_symbol(env, "y", &undef);
    z_sym = install_symbol(env, "z", &undef);
    bool pass = true;
    // (define) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(define_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define x) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define x 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define 1 2) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define x 1) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_void_tp();
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    typed_ptr* x_value = value_lookup_index(env, x_sym);
    expected = create_number_tp(1);
    if (!deep_match_typed_ptrs(x_value, expected)) {
        pass = false;
    }
    free(expected);
    free(x_value);
    // (define x (+ 1 1)) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(add_one_one_s_expr(env)));
    expected = create_void_tp();
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    x_value = value_lookup_index(env, x_sym);
    expected = create_number_tp(2);
    if (!deep_match_typed_ptrs(x_value, expected)) {
        pass = false;
    }
    free(expected);
    free(x_value);
    // (define x (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define <weird symbol number> 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_SYMBOL, 1000)); // kludge
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define (x)) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define (x) 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define () 1) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(create_empty_s_expr()));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define (1) 2) -> EVAL_ERROR_NOT_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_SYMBOL);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define (<weird symbol number>) 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr* fn_name_args = unit_list(create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define (x 1) 2) -> EVAL_ERROR_NOT_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(fn_name_args, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_SYMBOL);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define (x) 1) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    typed_ptr* body = create_number_tp(1);
    s_expr_append(cmd, copy_typed_ptr(body));
    expected = create_void_tp();
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    x_value = value_lookup_index(env, x_sym);
    Function_Node* x_fn = function_lookup_index(env, x_value);
    if (x_fn == NULL || \
        strcmp(x_fn->name, "x") || \
        x_fn->param_list != NULL || \
        x_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(x_fn->body, body)) {
        pass = false;
    }
    free(x_value);
    free(body);
    // (define (x) (+ 1 1)) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_s_expr_tp(add_one_one_s_expr(env)));
    expected = create_void_tp();
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    x_value = value_lookup_index(env, x_sym);
    x_fn = function_lookup_index(env, x_value);
    body = create_s_expr_tp(add_one_one_s_expr(env));
    if (x_fn == NULL || \
        strcmp(x_fn->name, "x") || \
        x_fn->param_list != NULL || \
        x_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(x_fn->body, body)) {
        pass = false;
    }
    free(x_value);
    delete_s_expr_recursive(body->ptr.se_ptr, true);
    free(body);
    // (define (x <weird symbol number>) 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(fn_name_args, create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    // (define (x y z) (+ y z)) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(fn_name_args, copy_typed_ptr(y_sym));
    s_expr_append(fn_name_args, copy_typed_ptr(z_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr* add_y_z = unit_list(ADD);
    s_expr_append(add_y_z, copy_typed_ptr(y_sym));
    s_expr_append(add_y_z, copy_typed_ptr(z_sym));
    s_expr_append(cmd, create_s_expr_tp(add_y_z));
    expected = create_void_tp();
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    x_value = value_lookup_index(env, x_sym);
    x_fn = function_lookup_index(env, x_value);
    add_y_z = unit_list(ADD);
    s_expr_append(add_y_z, copy_typed_ptr(y_sym));
    s_expr_append(add_y_z, copy_typed_ptr(z_sym));
    body = create_s_expr_tp(add_y_z);
    if (x_fn == NULL || \
        strcmp(x_fn->name, "x") || \
        x_fn->param_list == NULL || \
        strcmp(x_fn->param_list->name, "y") || \
        x_fn->param_list->next == NULL || \
        strcmp(x_fn->param_list->next->name, "z") || \
        x_fn->param_list->next->next != NULL || \
        x_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(x_fn->body, body)) {
        pass = false;
    }
    free(x_value);
    delete_s_expr_recursive(add_y_z, true);
    free(body);
    // (define (x) (/ 0)) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_void_tp();
    pass = run_test_expect(eval_define, cmd, env, expected) && pass;
    x_value = value_lookup_index(env, x_sym);
    x_fn = function_lookup_index(env, x_value);
    body = create_s_expr_tp(divide_zero_s_expr(env));
    if (x_fn == NULL || \
        strcmp(x_fn->name, "x") || \
        x_fn->param_list != NULL || \
        x_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(x_fn->body, body)) {
        pass = false;
    }
    free(x_value);
    delete_s_expr_recursive(body->ptr.se_ptr, true);
    free(body);
    delete_environment_full(env);
    free(define_builtin);
    free(x_sym);
    free(y_sym);
    free(z_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_setvar(test_env* te) {
    print_test_announce("eval_set_variable()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* setvar_builtin = builtin_tp_from_name(env, "set!");
    typed_ptr *x_sym;
    x_sym = install_symbol(env, "x", &undef);
    bool pass = true;
    // (set!) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(setvar_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! x) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! x 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! 1 2) -> EVAL_ERROR_NOT_SYMBOL
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_SYMBOL);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! (x) 1) -> EVAL_ERROR_NOT_SYMBOL
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! x 1) [with x undefined] -> EVAL_ERROR_UNDEF_SYM
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_UNDEF_SYM);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! <weird symbol number> 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! x 1) [with x defined] -> <void> + side effect
    symbol_lookup_index(env, x_sym)->type = TYPE_BOOL;
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_void_tp();
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    typed_ptr* x_value = value_lookup_index(env, x_sym);
    if (x_value == NULL || \
        x_value->type != TYPE_FIXNUM || \
        x_value->ptr.idx != 1) {
        pass = false;
    }
    free(x_value);
    // (set! x (+ 1 1)) [with x defined] -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(add_one_one_s_expr(env)));
    expected = create_void_tp();
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    x_value = value_lookup_index(env, x_sym);
    if (x_value == NULL || \
        x_value->type != TYPE_FIXNUM || \
        x_value->ptr.idx != 2) {
        pass = false;
    }
    free(x_value);
    // (set! x (list 1 2)) [with x defined] -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_void_tp();
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    x_value = value_lookup_index(env, x_sym);
    s_expr* result_list = unit_list(create_number_tp(1));
    s_expr_append(result_list, create_number_tp(2));
    if (x_value == NULL || \
        x_value->type != TYPE_S_EXPR || \
        !match_s_exprs(x_value->ptr.se_ptr, result_list)) {
        pass = false;
    }
    if (x_value != NULL && x_value->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(x_value->ptr.se_ptr, true);
    }
    free(x_value);
    delete_s_expr_recursive(result_list, true);
    // (set! x (/ 0)) [with x defined] -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    // (set! x TEST_ERROR_DUMMY) [with x defined] -> TEST_ERROR_DUMMY
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(eval_set_variable, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(setvar_builtin);
    free(x_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_builtin(test_env* te) {
    print_test_announce("eval_builtin()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr *x_sym;
    x_sym = install_symbol(env, "x", &undef);
    bool pass = true;
    // (+ 1 1) -> 2
    s_expr* cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_ADD));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    typed_ptr* expected = create_number_tp(2);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (- 1 1) -> 0
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_SUB));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(0);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (* 2 3) -> 6
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_MUL));
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    expected = create_number_tp(6);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (/ 10 2) -> 5
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_DIV));
    s_expr_append(cmd, create_number_tp(10));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(5);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (= 1 1) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBEREQ));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (< 1 0) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERLT));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (> 1 0) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERGT));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (<= 1 0) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERLE));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (>= 1 0) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERGE));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (define x 1) -> <void> + side effect
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_DEFINE));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_void_tp();
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    expected = create_number_tp(1);
    typed_ptr* x_value = value_lookup_index(env, x_sym);
    if (!match_typed_ptrs(x_value, expected)) {
        pass = false;
    }
    free(expected);
    free(x_value);
    // (set! x 2) -> <void> + side effect
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_SETVAR));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_void_tp();
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    expected = create_number_tp(2);
    x_value = value_lookup_index(env, x_sym);
    if (!match_typed_ptrs(x_value, expected)) {
        pass = false;
    }
    free(expected);
    free(x_value);
    // (exit) -> EVAL_ERROR_EXIT
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_EXIT));
    expected = create_error_tp(EVAL_ERROR_EXIT);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (cons 1 null) -> '(1)
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_CONS));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, NULL_SYM);
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (car (list 1 2)) -> 1
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_CAR));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_number_tp(1);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (cdr (list 1 2)) -> '(2)
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_CDR));
    s_expr_append(cmd, create_s_expr_tp(list_one_two_s_expr(env)));
    expected = create_s_expr_tp(unit_list(create_number_tp(2)));
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (list 1 2) -> '(1 2)
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_LIST));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(2));
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (and #t #f) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_AND));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (or #f #t) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_OR));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (not #t) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NOT));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (cond (#f 1) (else 2)) -> 2
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_COND));
    s_expr* first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    s_expr* second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_cond, cmd, env, expected) && pass;
    // (list? 1) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_LISTPRED));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (pair? #t) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_PAIRPRED));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (number? 1) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERPRED));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (boolean? #f) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_BOOLPRED));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (void? <void>) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_VOIDPRED));
    s_expr_append(cmd, create_void_tp());
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (procedure? +) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_PROCPRED));
    s_expr_append(cmd, ADD);
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (lambda () 1) -> <procedure> + side effect
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_LAMBDA));
    s_expr_append(cmd, create_s_expr_tp(create_empty_s_expr()));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_FUNCTION, 0);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    typed_ptr* body = create_number_tp(1);
    expected = create_atom_tp(TYPE_FUNCTION, 0);
    Function_Node* fn = function_lookup_index(env, expected);
    if (fn == NULL || \
        fn->param_list != NULL || \
        fn->closure_env == NULL || \
        !match_typed_ptrs(fn->body, body)) {
        pass = false;
    }
    free(expected);
    free(body);
    // (<malformed builtin> 1) -> EVAL_ERROR_UNDEF_BUILTIN
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, 1000));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    // (/ 0) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_DIV));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_builtin, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(x_sym);
    free(else_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_s_expr(test_env* te) {
    print_test_announce("eval_s_expr()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr *x_sym, *x2_sym;
    x_sym = install_symbol(env, "x", &undef);
    x2_sym = install_symbol(env, "x2", &undef);
    bool pass = true;
    // NULL -> EVAL_ERROR_NULL_S_EXPR
    s_expr* cmd = NULL;
    typed_ptr* expected = create_error_tp(EVAL_ERROR_NULL_S_EXPR);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // () -> EVAL_ERROR_MISSING_PROCEDURE
    cmd = create_empty_s_expr();
    expected = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // <malformed s-expression> -> EVAL_ERROR_MALFORMED_S_EXPR
    cmd = create_s_expr(create_number_tp(1), NULL);
    expected = create_error_tp(EVAL_ERROR_MALFORMED_S_EXPR);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    cmd = create_s_expr(NULL, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_MALFORMED_S_EXPR);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // ((/ 0) 1) -> EVAL_ERROR_DIV_ZERO
    cmd = create_empty_s_expr();
    s_expr* nested_cmd = create_empty_s_expr();
    s_expr_append(nested_cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    s_expr_append(nested_cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // ((+ 1 1) 1) -> EVAL_ERROR_CAR_NOT_CALLABLE
    cmd = create_empty_s_expr();
    nested_cmd = create_empty_s_expr();
    s_expr_append(nested_cmd, create_s_expr_tp(add_one_one_s_expr(env)));
    s_expr_append(nested_cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_error_tp(EVAL_ERROR_CAR_NOT_CALLABLE);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // ('+ 1 1) -> 2
    cmd = unit_list(ADD);
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(2);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // ('x2 10) [if x2 previously defined as doubling function] -> 20
    cmd = unit_list(builtin_tp_from_name(env, "define"));
    s_expr* fn_sig = unit_list(copy_typed_ptr(x2_sym));
    s_expr_append(fn_sig, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_sig));
    s_expr* body = unit_list(MULTIPLY);
    s_expr_append(body, copy_typed_ptr(x_sym));
    s_expr_append(body, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(body));
    expected = create_void_tp();
    run_test_expect(eval_define, cmd, env, expected);
    cmd = unit_list(copy_typed_ptr(x2_sym));
    s_expr_append(cmd, create_number_tp(10));
    expected = create_number_tp(20);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // ((lambda (x) (* x 3)) 10) -> 30
    cmd = create_empty_s_expr();
    s_expr* lambda = unit_list(LAMBDA_SYM);
    s_expr_append(lambda, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    body = unit_list(MULTIPLY);
    s_expr_append(body, copy_typed_ptr(x_sym));
    s_expr_append(body, create_number_tp(3));
    s_expr_append(lambda, create_s_expr_tp(body));
    s_expr_append(cmd, create_s_expr_tp(lambda));
    s_expr_append(cmd, create_number_tp(10));
    expected = create_number_tp(30);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    // ('/ 0) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(DIVIDE);
    s_expr_append(cmd, create_number_tp(0));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_s_expr, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(x_sym);
    free(x2_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_function(test_env* te) {
    print_test_announce("eval_function()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr *x_sym, *y_sym, *my_fun;
    x_sym = install_symbol(env, "x", &undef);
    y_sym = install_symbol(env, "y", &undef);
    char mf[] = "my-fun";
    my_fun = install_symbol(env, mf, &undef);
    bool pass = true;
    // given my-fun, a two-parameter function that doubles its first argument
    //   and prepends it to its second
    //   (i.e.: (define (my-fun x y) (cons (* x 2) y))     )
    s_expr* cmd = unit_list(builtin_tp_from_name(env, "define"));
    s_expr* fn_sig = unit_list(copy_typed_ptr(my_fun));
    s_expr_append(fn_sig, copy_typed_ptr(x_sym));
    s_expr_append(fn_sig, copy_typed_ptr(y_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_sig));
    s_expr* body = unit_list(CONS_SYM);
    s_expr* multiply_x_two = unit_list(MULTIPLY);
    s_expr_append(multiply_x_two, copy_typed_ptr(x_sym));
    s_expr_append(multiply_x_two, create_number_tp(2));
    s_expr_append(body, create_s_expr_tp(multiply_x_two));
    s_expr_append(body, copy_typed_ptr(y_sym));
    s_expr_append(cmd, create_s_expr_tp(body));
    typed_ptr* expected = create_void_tp();
    run_test_expect(eval_define, cmd, env, expected);
    // (<undefined function> 1) -> EVAL_ERROR_UNDEF_FUNCTION
    cmd = unit_list(create_atom_tp(TYPE_FUNCTION, 1000));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_UNDEF_FUNCTION);
    pass = run_test_expect(eval_function, cmd, env, expected) && pass;
    // (my-fun (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_s_expr_tp(divide_zero_s_expr(env)));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = run_test_expect(eval_function, cmd, env, expected) && pass;
    // (my-fun) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(value_lookup_index(env, my_fun));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_function, cmd, env, expected) && pass;
    // (my-fun 1) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = run_test_expect(eval_function, cmd, env, expected) && pass;
    // (my-fun 1 2 3) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = run_test_expect(eval_function, cmd, env, expected) && pass;
    // (my-fun 2 (list 8 16)) -> '(4 8 16)
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_number_tp(2));
    s_expr* list_eight_sixteen = unit_list(LIST_SYM);
    s_expr_append(list_eight_sixteen, create_number_tp(8));
    s_expr_append(list_eight_sixteen, create_number_tp(16));
    s_expr_append(cmd, create_s_expr_tp(list_eight_sixteen));
    expected = create_s_expr_tp(unit_list(create_number_tp(4)));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(8));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(16));
    pass = run_test_expect(eval_function, cmd, env, expected) && pass;
    // (my-fun #t (list 8 16)) -> EVAL_ERROR_NEED_NUM
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    list_eight_sixteen = unit_list(LIST_SYM);
    s_expr_append(list_eight_sixteen, create_number_tp(8));
    s_expr_append(list_eight_sixteen, create_number_tp(16));
    s_expr_append(cmd, create_s_expr_tp(list_eight_sixteen));
    expected = create_error_tp(EVAL_ERROR_NEED_NUM);
    pass = run_test_expect(eval_function, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(x_sym);
    free(y_sym);
    free(my_fun);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

typed_ptr* wrapper_evaluate(const s_expr* cmd, Environment* env) {
    return evaluate(cmd->car, env);
}

void test_evaluate(test_env* te) {
    print_test_announce("evaluate()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr number = {.type=TYPE_FIXNUM, .ptr={.idx=1}};
    typed_ptr *x_sym;
    x_sym = install_symbol(env, "x", &number);
    bool pass = true;
    // eval[ NULL ] -> EVAL_ERROR_NULL_S_EXPR
    s_expr* cmd = create_empty_s_expr();
    typed_ptr* expected = create_error_tp(EVAL_ERROR_NULL_S_EXPR);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ <undefined> ] -> EVAL_ERROR_UNDEF_SYM
    cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
    expected = create_error_tp(EVAL_ERROR_UNDEF_SYM);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ TEST_ERROR_DUMMY ] -> TEST_ERROR_DUMMY
    cmd = unit_list(create_error_tp(TEST_ERROR_DUMMY));
    expected = create_error_tp(TEST_ERROR_DUMMY);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ <void> ] -> <void>
    cmd = unit_list(create_void_tp());
    expected = create_void_tp();
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ 1 ] -> 1
    cmd = unit_list(create_number_tp(1));
    expected = create_number_tp(1);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ #t ] -> #t
    cmd = unit_list(create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ <procedure:+> ] -> <procedure:+>
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_ADD));
    expected = create_atom_tp(TYPE_BUILTIN, BUILTIN_ADD);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ () ] -> EVAL_ERROR_MISSING_PROCEDURE
    cmd = unit_list(create_s_expr_tp(create_empty_s_expr()));
    expected = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ <malformed s-expression> ] -> EVAL_ERROR_MALFORMED_S_EXPR
    cmd = unit_list(create_s_expr_tp(create_s_expr(create_number_tp(1), NULL)));
    expected = create_error_tp(EVAL_ERROR_MALFORMED_S_EXPR);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    cmd = unit_list(create_s_expr_tp(create_s_expr(NULL, create_number_tp(1))));
    expected = create_error_tp(EVAL_ERROR_MALFORMED_S_EXPR);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ (+ 1 1) ] -> 2
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_ADD));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    cmd = unit_list(create_s_expr_tp(cmd));
    expected = create_number_tp(2);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ ((lambda (x) (* x 10)) 2) ] -> <procedure>
    cmd = create_empty_s_expr();
    s_expr* lambda = unit_list(LAMBDA_SYM);
    s_expr_append(lambda, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    s_expr* multiply_x_ten = unit_list(MULTIPLY);
    s_expr_append(multiply_x_ten, copy_typed_ptr(x_sym));
    s_expr_append(multiply_x_ten, create_number_tp(10));
    s_expr_append(lambda, create_s_expr_tp(multiply_x_ten));
    s_expr_append(cmd, create_s_expr_tp(lambda));
    s_expr_append(cmd, create_number_tp(2));
    cmd = unit_list(create_s_expr_tp(cmd));
    expected = create_atom_tp(TYPE_FIXNUM, 20);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ 'x ] (assuming x is defined to be 1) -> 1
    cmd = unit_list(copy_typed_ptr(x_sym));
    expected = create_number_tp(1);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ <symbol not in symbol table> ] -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(create_atom_tp(TYPE_SYMBOL, 1000));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ <procedure from above> ] -> <procedure from above>
    cmd = unit_list(create_atom_tp(TYPE_FUNCTION, 0));
    expected = create_atom_tp(TYPE_FUNCTION, 0);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    // eval[ <undefined type> ] -> EVAL_ERROR_UNDEF_TYPE
    cmd = unit_list(create_atom_tp(1000, 1));
    expected = create_error_tp(EVAL_ERROR_UNDEF_TYPE);
    pass = run_test_expect(wrapper_evaluate, cmd, env, expected) && pass;
    delete_environment_full(env);
    free(x_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

#undef ADD
#undef SUBTRACT
#undef MULTIPLY
#undef DIVIDE

#undef CONS_SYM
#undef NULL_SYM
#undef LIST_SYM
#undef LAMBDA_SYM

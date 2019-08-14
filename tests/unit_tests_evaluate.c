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
    test_eval_lambda(te);
    test_eval_cond(te);
    test_eval_define(te);
    test_eval_setvar(te);
    test_eval_builtin(te);
    test_eval_sexpr(te);
    test_eval_function(te);
    test_evaluate(te);
    return;
}

void test_collect_parameters(test_env* te) {
    print_test_announce("collect_parameters()");
    bool pass = 1;
    // pass an empty list
    Environment* env = create_environment(0, 0);
    typed_ptr* se_tp = create_s_expr_tp(create_empty_s_expr());
    Symbol_Node* params = collect_parameters(se_tp, env);
    if (params != NULL) {
        pass = 0;
    }
    // pass a list whose first car is not a symbol
    s_expr_append(se_tp->ptr.se_ptr, create_atom_tp(TYPE_NUM, 1000));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->name != NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_NOT_ID || \
        params->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a list whose middle car is not a symbol
    typed_ptr *sym_1, *sym_2, *sym_3;
    sym_1 = install_symbol(env, NULL, TYPE_NUM, (tp_value){.idx=1000});
    sym_2 = install_symbol(env, NULL, TYPE_NUM, (tp_value){.idx=1000});
    sym_3 = install_symbol(env, NULL, TYPE_NUM, (tp_value){.idx=1000});
    se_tp = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_1));
    s_expr_append(se_tp->ptr.se_ptr, create_atom_tp(TYPE_NUM, 1000));
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_2));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->name != NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_NOT_ID || \
        params->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    // pass a list whose last car is not a symbol
    se_tp = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_1));
    s_expr_append(se_tp->ptr.se_ptr, copy_typed_ptr(sym_2));
    s_expr_append(se_tp->ptr.se_ptr, create_atom_tp(TYPE_NUM, 1000));
    params = collect_parameters(se_tp, env);
    if (params == NULL || \
        params->name != NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_NOT_ID || \
        params->next != NULL) {
        pass = 0;
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
        params->name != NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_BAD_ARG_TYPE || \
        params->next != NULL) {
        pass = 0;
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
        params->name != NULL || \
        params->type != TYPE_UNDEF || \
        params->next != NULL) {
        pass = 0;
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
        params->name != NULL || \
        params->type != TYPE_ERROR || \
        params->value.idx != EVAL_ERROR_BAD_SYMBOL || \
        params->next != NULL) {
        pass = 0;
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
        params->name != NULL || \
        params->type != TYPE_UNDEF || \
        params->next == NULL || \
        params->next->name != NULL || \
        params->next->type != TYPE_UNDEF || \
        params->next->next == NULL || \
        params->next->next->name != NULL || \
        params->next->next->type != TYPE_UNDEF || \
        params->next->next->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(params);
    params = NULL;
    delete_s_expr_recursive(se_tp->ptr.se_ptr, true);
    free(se_tp);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_bind_args(test_env* te) {
    print_test_announce("bind_args()");
    bool pass = 1;
    // no params, no args
    Function_Node* fn_no_params = create_function_node(0, NULL, NULL, NULL);
    typed_ptr* empty_args = create_s_expr_tp(create_empty_s_expr());
    Symbol_Node* bound_args = bind_args(NULL, fn_no_params, empty_args);
    if (bound_args != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // no params, one arg
    typed_ptr* one_arg = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(one_arg->ptr.se_ptr, create_atom_tp(TYPE_NUM, 1000));
    bound_args = bind_args(NULL, fn_no_params, one_arg);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_MANY_ARGS || \
        bound_args->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // one param, no args
    Symbol_Node* one_param = create_symbol_node(0, \
                                                strdup("x"), \
                                                TYPE_UNDEF, \
                                                (tp_value){.idx=0});
    Function_Node* fn_1_param = create_function_node(0, one_param, NULL, NULL);
    bound_args = bind_args(NULL, fn_1_param, empty_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_FEW_ARGS || \
        bound_args->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // one param, one arg
    bound_args = bind_args(NULL, fn_1_param, one_arg);
    if (bound_args == NULL || \
        strcmp(bound_args->name, "x") || \
        bound_args->type != TYPE_NUM || \
        bound_args->value.idx != 1000 || \
        bound_args->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // one param, two args
    typed_ptr* two_args = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(two_args->ptr.se_ptr, create_atom_tp(TYPE_NUM, 1000));
    s_expr* se = create_empty_s_expr();
    s_expr_append(two_args->ptr.se_ptr, create_s_expr_tp(se));
    bound_args = bind_args(NULL, fn_1_param, two_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_MANY_ARGS || \
        bound_args->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // two params, no args
    Symbol_Node* two_params = create_symbol_node(0, \
                                                 strdup("x"), \
                                                 TYPE_UNDEF, \
                                                 (tp_value){.idx=0});
    two_params->next = create_symbol_node(0, \
                                          strdup("y"), \
                                          TYPE_UNDEF, \
                                          (tp_value){.idx=0});
    Function_Node* fn_2_params = create_function_node(0, \
                                                      two_params, \
                                                      NULL, \
                                                      NULL);
    bound_args = bind_args(NULL, fn_2_params, empty_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_FEW_ARGS || \
        bound_args->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // two params, one arg
    bound_args = bind_args(NULL, fn_2_params, one_arg);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_FEW_ARGS || \
        bound_args->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // two params, two args
    bound_args = bind_args(NULL, fn_2_params, two_args);
    if (bound_args == NULL || \
        strcmp(bound_args->name, "y") || \
        bound_args->type != TYPE_SEXPR || \
        bound_args->value.se_ptr != se || \
        !is_empty_list(bound_args->value.se_ptr) || \
        bound_args->next == NULL || \
        strcmp(bound_args->next->name, "x") || \
        bound_args->next->type != TYPE_NUM || \
        bound_args->next->value.idx != 1000 || \
        bound_args->next->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    // two params, three args
    typed_ptr* three_args = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(three_args->ptr.se_ptr, create_atom_tp(TYPE_NUM, 1000));
    s_expr_append(three_args->ptr.se_ptr, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(three_args->ptr.se_ptr, create_atom_tp(TYPE_NUM, 2000));
    bound_args = bind_args(NULL, fn_2_params, three_args);
    if (bound_args == NULL || \
        bound_args->type != TYPE_ERROR || \
        bound_args->value.idx != EVAL_ERROR_MANY_ARGS || \
        bound_args->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(bound_args);
    free(fn_no_params);
    delete_s_expr_recursive(empty_args->ptr.se_ptr, true);
    free(empty_args);
    delete_s_expr_recursive(one_arg->ptr.se_ptr, true);
    free(one_arg);
    delete_symbol_node_list(one_param);
    free(fn_1_param);
    delete_s_expr_recursive(two_args->ptr.se_ptr, true);
    free(two_args);
    delete_symbol_node_list(two_params);
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
    bool pass = 1;
    // empty list of bound args
    Environment* env = create_environment(0, 0);
    blind_install_symbol_atom(env, strdup("x"), TYPE_UNDEF, 0);
    blind_install_symbol_atom(env, strdup("y"), TYPE_UNDEF, 0);
    blind_install_symbol_atom(env, strdup("z"), TYPE_UNDEF, 0);
    Symbol_Node* args = NULL;
    Environment* out = make_eval_env(env, args);
    if (out == env || \
        symbol_lookup_string(env, "x") == symbol_lookup_string(out, "x") || \
        symbol_lookup_string(env, "y") == symbol_lookup_string(out, "y") || \
        symbol_lookup_string(env, "z") == symbol_lookup_string(out, "z") || \
        symbol_lookup_string(env, "x") == NULL || \
        symbol_lookup_string(env, "x")->type != TYPE_UNDEF || \
        symbol_lookup_string(env, "y") == NULL || \
        symbol_lookup_string(env, "y")->type != TYPE_UNDEF || \
        symbol_lookup_string(env, "z") == NULL || \
        symbol_lookup_string(env, "z")->type != TYPE_UNDEF || \
        symbol_lookup_string(out, "x") == NULL || \
        symbol_lookup_string(out, "x")->type != TYPE_UNDEF || \
        symbol_lookup_string(out, "y") == NULL || \
        symbol_lookup_string(out, "y")->type != TYPE_UNDEF || \
        symbol_lookup_string(out, "z") == NULL || \
        symbol_lookup_string(out, "z")->type != TYPE_UNDEF) {
        pass = 0;
    }
    delete_environment_shared(out);
    // one bound arg
    args = create_symbol_node(0, strdup("x"), TYPE_NUM, (tp_value){.idx=1000});
    out = make_eval_env(env, args);
    if (out == env || \
        symbol_lookup_string(env, "x") == symbol_lookup_string(out, "x") || \
        symbol_lookup_string(env, "y") == symbol_lookup_string(out, "y") || \
        symbol_lookup_string(env, "z") == symbol_lookup_string(out, "z") || \
        symbol_lookup_string(env, "x") == NULL || \
        symbol_lookup_string(env, "x")->type != TYPE_UNDEF || \
        symbol_lookup_string(env, "y") == NULL || \
        symbol_lookup_string(env, "y")->type != TYPE_UNDEF || \
        symbol_lookup_string(env, "z") == NULL || \
        symbol_lookup_string(env, "z")->type != TYPE_UNDEF || \
        symbol_lookup_string(out, "x") == NULL || \
        symbol_lookup_string(out, "x")->type != TYPE_NUM || \
        symbol_lookup_string(out, "x")->value.idx != 1000 || \
        symbol_lookup_string(out, "y") == NULL || \
        symbol_lookup_string(out, "y")->type != TYPE_UNDEF || \
        symbol_lookup_string(out, "z") == NULL || \
        symbol_lookup_string(out, "z")->type != TYPE_UNDEF) {
        pass = 0;
    }
    delete_environment_shared(out);
    // two bound args
    args->value.idx = 2000;
    s_expr* se = create_empty_s_expr();
    args->next = create_symbol_node(0, \
                                    strdup("y"), \
                                    TYPE_SEXPR, \
                                    (tp_value){.se_ptr=se});
    out = make_eval_env(env, args);
    if (out == env || \
        symbol_lookup_string(env, "x") == symbol_lookup_string(out, "x") || \
        symbol_lookup_string(env, "y") == symbol_lookup_string(out, "y") || \
        symbol_lookup_string(env, "z") == symbol_lookup_string(out, "z") || \
        symbol_lookup_string(env, "x") == NULL || \
        symbol_lookup_string(env, "x")->type != TYPE_UNDEF || \
        symbol_lookup_string(env, "y") == NULL || \
        symbol_lookup_string(env, "y")->type != TYPE_UNDEF || \
        symbol_lookup_string(env, "z") == NULL || \
        symbol_lookup_string(env, "z")->type != TYPE_UNDEF || \
        symbol_lookup_string(out, "x") == NULL || \
        symbol_lookup_string(out, "x")->type != TYPE_NUM || \
        symbol_lookup_string(out, "x")->value.idx != 2000 || \
        symbol_lookup_string(out, "y") == NULL || \
        symbol_lookup_string(out, "y")->type != TYPE_SEXPR || \
        symbol_lookup_string(out, "y")->value.se_ptr != se || \
        !is_empty_list(symbol_lookup_string(out, "y")->value.se_ptr) || \
        symbol_lookup_string(out, "z") == NULL || \
        symbol_lookup_string(out, "z")->type != TYPE_UNDEF) {
        pass = 0;
    }
    delete_environment_shared(out);
    delete_symbol_node_list(args);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

bool match_error(const typed_ptr* tp, interpreter_error err) {
    return (tp != NULL && tp->type == TYPE_ERROR && tp->ptr.idx == err);
}

bool tp_is_empty_list(const typed_ptr* tp) {
    return (tp != NULL && \
            tp->type == TYPE_SEXPR && \
            is_empty_list(tp->ptr.se_ptr));
}

typed_ptr* symbol_tp_from_name(Environment* env, const char name[]) {
    Symbol_Node* found = symbol_lookup_string(env, name);
    return (found == NULL) ? NULL : \
                             create_atom_tp(TYPE_SYMBOL, found->symbol_idx);
}

void test_collect_arguments(test_env* te) {
    print_test_announce("collect_arguments()");
    bool pass = 1;
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    // arg s-expr is a pair -> error
    s_expr* call_pair = create_s_expr(create_atom_tp(TYPE_BUILTIN, 0), \
                                      create_atom_tp(TYPE_NUM, 1000));
    typed_ptr* out = collect_arguments(call_pair, env, 0, -1, true);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(call_pair, true);
    free(out);
    // arg s-expr ends in a pair
    s_expr* call_bad_list = create_empty_s_expr();
    s_expr_append(call_bad_list, create_atom_tp(TYPE_BUILTIN, 0));
    s_expr_next(call_bad_list)->car = create_atom_tp(TYPE_NUM, 1000);
    s_expr_next(call_bad_list)->cdr = create_atom_tp(TYPE_NUM, 2000);
    out = collect_arguments(call_bad_list, env, 0, -1, true);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(call_bad_list, true);
    free(out);
    // empty arg s-expr, with min_args == 0 & max_args == 0
    s_expr* call_no_args = create_empty_s_expr();
    s_expr_append(call_no_args, create_atom_tp(TYPE_BUILTIN, 0));
    out = collect_arguments(call_no_args, env, 0, 0, true);
    if (!tp_is_empty_list(out) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    //    -> and without evaluate_all_args
    out = collect_arguments(call_no_args, env, 0, 0, false);
    if (!tp_is_empty_list(out) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // empty arg s-expr, with min_args == 0 & max_args > 0
    out = collect_arguments(call_no_args, env, 0, 1, true);
    if (!tp_is_empty_list(out) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // empty arg s-expr, with min_args == 0 & max_args < 0
    out = collect_arguments(call_no_args, env, 0, -1, true);
    if (!tp_is_empty_list(out) || \
        out->ptr.se_ptr == s_expr_next(call_no_args)) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // empty arg s-expr, with min_args > 0
    out = collect_arguments(call_no_args, env, 1, 1, true);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    free(out);
    delete_s_expr_recursive(call_no_args, true);
    // one-elt arg s-expr, with min_args == 0 & max_args == 0
    s_expr* call_one_arg = create_empty_s_expr();
    s_expr_append(call_one_arg, create_atom_tp(TYPE_BUILTIN, 0));
    typed_ptr* value_1 = create_atom_tp(TYPE_NUM, 1000);
    s_expr_append(call_one_arg, value_1);
    out = collect_arguments(call_one_arg, env, 0, 0, true);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = 0;
    }
    free(out);
    // one-elt arg s-expr, with min_args == 0 & max_args == 1
    out = collect_arguments(call_one_arg, env, 0, 1, true);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    //    -> and without evaluate_all_args
    out = collect_arguments(call_one_arg, env, 0, 1, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 0 & max_args == 2
    out = collect_arguments(call_one_arg, env, 0, 2, true);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 0 & max_args == -1
    out = collect_arguments(call_one_arg, env, 0, -1, true);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 1 & max_args == 1
    out = collect_arguments(call_one_arg, env, 1, 1, true);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 1 & max_args == 2
    out = collect_arguments(call_one_arg, env, 1, 2, true);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 1 & max_args == -1
    out = collect_arguments(call_one_arg, env, 1, -1, true);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_one_arg)) || \
        out->ptr.se_ptr->car == value_1) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    // one-elt arg s-expr, with min_args == 2 & max_args == 2
    out = collect_arguments(call_one_arg, env, 2, 2, true);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    free(out);
    // one-elt arg s-expr, with min_args == 2 & max_args == -1
    out = collect_arguments(call_one_arg, env, 2, -1, true);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    free(out);
    delete_s_expr_recursive(call_one_arg, true);
    // two-elt arg s-expr, with min_args == 0 & max_args == 0
    s_expr* call_two_args = create_empty_s_expr();
    s_expr_append(call_two_args, create_atom_tp(TYPE_BUILTIN, 0));
    value_1 = create_atom_tp(TYPE_NUM, 1000);
    s_expr_append(call_two_args, value_1);
    typed_ptr* value_2 = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(value_2->ptr.se_ptr, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(call_two_args, value_2);
    out = collect_arguments(call_two_args, env, 0, 0, false);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = 0;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == 1
    out = collect_arguments(call_two_args, env, 0, 1, false);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = 0;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == 2
    out = collect_arguments(call_two_args, env, 0, 2, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == 3
    out = collect_arguments(call_two_args, env, 0, 3, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 0 & max_args == -1
    out = collect_arguments(call_two_args, env, 0, -1, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == 1
    out = collect_arguments(call_two_args, env, 1, 1, false);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = 0;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == 2
    out = collect_arguments(call_two_args, env, 1, 2, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == 3
    out = collect_arguments(call_two_args, env, 1, 3, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 1 & max_args == -1
    out = collect_arguments(call_two_args, env, 1, -1, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 2 & max_args == 2
    out = collect_arguments(call_two_args, env, 2, 2, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 2 & max_args == 3
    out = collect_arguments(call_two_args, env, 2, 3, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 2 & max_args == -1
    out = collect_arguments(call_two_args, env, 2, -1, false);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, s_expr_next(call_two_args)) || \
        out->ptr.se_ptr->car == value_1 || \
        s_expr_next(out->ptr.se_ptr)->car->ptr.se_ptr != value_2->ptr.se_ptr) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, false);
    free(out);
    // two-elt arg s-expr, with min_args == 3 & max_args == 3
    out = collect_arguments(call_two_args, env, 3, 3, false);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    free(out);
    // two-elt arg s-expr, with min_args == 3 & max_args == -1
    out = collect_arguments(call_two_args, env, 3, -1, false);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    free(out);
    delete_s_expr_recursive(call_two_args, true);
    // and with evaluating
    s_expr* addition_arg = create_empty_s_expr();
    s_expr_append(addition_arg, symbol_tp_from_name(env, "+"));
    s_expr_append(addition_arg, create_atom_tp(TYPE_NUM, 1));
    s_expr_append(addition_arg, create_atom_tp(TYPE_NUM, 2));
    s_expr* list_arg = create_empty_s_expr();
    s_expr_append(list_arg, symbol_tp_from_name(env, "list"));
    s_expr_append(list_arg, create_atom_tp(TYPE_BOOL, 0));
    s_expr_append(list_arg, create_atom_tp(TYPE_BOOL, 1));
    s_expr* call_with_eval = create_empty_s_expr();
    s_expr_append(call_with_eval, create_atom_tp(TYPE_BUILTIN, 0));
    s_expr_append(call_with_eval, create_s_expr_tp(addition_arg));
    s_expr_append(call_with_eval, create_s_expr_tp(list_arg));
    s_expr* expected = create_empty_s_expr();
    s_expr_append(expected, create_atom_tp(TYPE_NUM, 3));
    s_expr* list_result = create_empty_s_expr();
    s_expr_append(list_result, create_atom_tp(TYPE_BOOL, 0));
    s_expr_append(list_result, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(expected, create_s_expr_tp(list_result));
    out = collect_arguments(call_with_eval, env, 2, 2, true);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = 0;
    }
    delete_s_expr_recursive(out->ptr.se_ptr, true);
    free(out);
    delete_s_expr_recursive(expected, true);
    // with evaluating but an error
    s_expr* plus_error = create_empty_s_expr();
    s_expr_append(plus_error, symbol_tp_from_name(env, "+"));
    s_expr_append(plus_error, create_atom_tp(TYPE_NUM, 1));
    s_expr_append(plus_error, create_atom_tp(TYPE_BOOL, 0));
    s_expr_append(call_with_eval, create_s_expr_tp(plus_error));
    out = collect_arguments(call_with_eval, env, 3, 3, true);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    free(out);
    delete_s_expr_recursive(call_with_eval, true);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

typed_ptr* create_number_tp(long value) {
    return create_typed_ptr(TYPE_NUM, (tp_value){.idx=value});
}

s_expr* unit_list(typed_ptr* tp) {
    return create_s_expr(tp, create_s_expr_tp(create_empty_s_expr()));
}

bool check_number_tp(typed_ptr* tp, long expected) {
    return (tp != NULL && tp->type == TYPE_NUM && tp->ptr.idx == expected);
}

// NOTE: frees cmd
bool run_test_expect_number(Environment* env, s_expr* cmd, long expected) {
    typed_ptr* out = eval_arithmetic(cmd, env);
    bool passed = check_number_tp(out, expected);
    delete_s_expr_recursive(cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    return passed;
}

typed_ptr* builtin_tp_from_name(Environment* env, const char name[]) {
    Symbol_Node* found = symbol_lookup_string(env, name);
    if (found == NULL || found->type != TYPE_BUILTIN) {
        return NULL;
    } else {
        return create_atom_tp(TYPE_BUILTIN, found->value.idx);
    }
}

void test_eval_arithmetic(test_env* te) {
    print_test_announce("eval_arithmetic()");
    // setup
    bool pass = 1;
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr *add, *subtract, *multiply, *divide;
    add = builtin_tp_from_name(env, "+");
    subtract = builtin_tp_from_name(env, "-");
    multiply = builtin_tp_from_name(env, "*");
    divide = builtin_tp_from_name(env, "/");
    // (+ . 1) -> EVAL_ERROR_ILLEGAL_PAIR
    s_expr* pair_cmd = create_s_expr(copy_typed_ptr(add), create_number_tp(1));
    typed_ptr* out = eval_arithmetic(pair_cmd, env);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(pair_cmd, true);
    free(out);
    // (+ 1 . 2) -> EVAL_ERROR_ILLEGAL_PAIR
    pair_cmd = create_s_expr(create_number_tp(1), create_number_tp(2));
    pair_cmd = create_s_expr(copy_typed_ptr(add), create_s_expr_tp(pair_cmd));
    out = eval_arithmetic(pair_cmd, env);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(pair_cmd, true);
    free(out);
    // (+ 1 2 . 3) -> EVAL_ERROR_ILLEGAL_PAIR
    pair_cmd = create_s_expr(create_number_tp(2), create_number_tp(3));
    pair_cmd = create_s_expr(create_number_tp(1), create_s_expr_tp(pair_cmd));
    pair_cmd = create_s_expr(copy_typed_ptr(add), create_s_expr_tp(pair_cmd));
    out = eval_arithmetic(pair_cmd, env);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(pair_cmd, true);
    free(out);
    // (+) -> 0
    s_expr* lone_add_cmd = unit_list(copy_typed_ptr(add));
    pass = pass && run_test_expect_number(env, lone_add_cmd, 0);
    // (-) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_subtract_cmd = unit_list(copy_typed_ptr(subtract));
    out = eval_arithmetic(lone_subtract_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(lone_subtract_cmd, true);
    free(out);
    // (*) -> 1
    s_expr* lone_multiply_cmd = unit_list(copy_typed_ptr(multiply));
    pass = pass && run_test_expect_number(env, lone_multiply_cmd, 1);
    // (/) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_divide_cmd = unit_list(copy_typed_ptr(divide));
    out = eval_arithmetic(lone_divide_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(lone_divide_cmd, true);
    free(out);
    // (+ 0) -> 0
    s_expr* add_zero_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(add_zero_cmd, create_number_tp(0));
    pass = pass && run_test_expect_number(env, add_zero_cmd, 0);
    // (- 0) -> 0
    s_expr* subtract_zero_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_zero_cmd, create_number_tp(0));
    pass = pass && run_test_expect_number(env, subtract_zero_cmd, 0);
    // (* 0) -> 0
    s_expr* multiply_zero_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(multiply_zero_cmd, create_number_tp(0));
    pass = pass && run_test_expect_number(env, multiply_zero_cmd, 0);
    // (/ 0) -> EVAL_ERROR_DIV_ZERO
    s_expr* divide_zero_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(divide_zero_cmd, create_number_tp(0));
    out = eval_arithmetic(divide_zero_cmd, env);
    if (!match_error(out, EVAL_ERROR_DIV_ZERO)) {
        pass = 0;
    }
    delete_s_expr_recursive(divide_zero_cmd, true);
    free(out);
    // (+ 2) -> 2
    s_expr* add_two_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(add_two_cmd, create_number_tp(2));
    pass = pass && run_test_expect_number(env, add_two_cmd, 2);
    // (- 2) -> -2
    s_expr* subtract_two_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_two_cmd, create_number_tp(2));
    pass = pass && run_test_expect_number(env, subtract_two_cmd, -2);
    // (* 2) -> 2
    s_expr* multiply_two_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(multiply_two_cmd, create_number_tp(2));
    pass = pass && run_test_expect_number(env, multiply_two_cmd, 2);
    // (/ 2) -> 0 (since floats don't work yet - integer division w/flooring)
    s_expr* divide_two_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(divide_two_cmd, create_number_tp(2));
    pass = pass && run_test_expect_number(env, divide_two_cmd, 0);
    // (+ 2 3) -> 5
    s_expr* add_two_three_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(add_two_three_cmd, create_number_tp(2));
    s_expr_append(add_two_three_cmd, create_number_tp(3));
    pass = pass && run_test_expect_number(env, add_two_three_cmd, 5);
    // (- 2 3) -> -1
    s_expr* subtract_two_three_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_two_three_cmd, create_number_tp(2));
    s_expr_append(subtract_two_three_cmd, create_number_tp(3));
    pass = pass && run_test_expect_number(env, subtract_two_three_cmd, -1);
    // (* 2 3) -> 6
    s_expr* multiply_two_three_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(multiply_two_three_cmd, create_number_tp(2));
    s_expr_append(multiply_two_three_cmd, create_number_tp(3));
    pass = pass && run_test_expect_number(env, multiply_two_three_cmd, 6);
    // (* 2 0) -> 0
    s_expr* multiply_two_zero_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(multiply_two_zero_cmd, create_number_tp(2));
    s_expr_append(multiply_two_zero_cmd, create_number_tp(0));
    pass = pass && run_test_expect_number(env, multiply_two_zero_cmd, 0);
    // (/ 6 2) -> 3
    s_expr* divide_six_two_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(divide_six_two_cmd, create_number_tp(6));
    s_expr_append(divide_six_two_cmd, create_number_tp(2));
    pass = pass && run_test_expect_number(env, divide_six_two_cmd, 3);
    // (/ 5 2) -> 2 (again - integer division)
    s_expr* divide_five_two_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(divide_five_two_cmd, create_number_tp(5));
    s_expr_append(divide_five_two_cmd, create_number_tp(2));
    pass = pass && run_test_expect_number(env, divide_five_two_cmd, 2);
    // (/ 6 0) -> EVAL_ERROR_DIV_ZERO
    s_expr* divide_six_zero_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(divide_six_zero_cmd, create_number_tp(6));
    s_expr_append(divide_six_zero_cmd, create_number_tp(0));
    out = eval_arithmetic(divide_six_zero_cmd, env);
    if (!match_error(out, EVAL_ERROR_DIV_ZERO)) {
        pass = 0;
    }
    delete_s_expr_recursive(divide_six_zero_cmd, true);
    free(out);
    // (/ 0 6) -> 0
    s_expr* divide_zero_six_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(divide_zero_six_cmd, create_number_tp(0));
    s_expr_append(divide_zero_six_cmd, create_number_tp(6));
    pass = pass && run_test_expect_number(env, divide_zero_six_cmd, 0);
    // (+ 2 (+ 2 2)) -> 6
    s_expr* nested_add_twos_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(nested_add_twos_cmd, create_number_tp(2));
    s_expr* inner_add_twos = unit_list(symbol_tp_from_name(env, "+"));
    s_expr_append(inner_add_twos, create_number_tp(2));
    s_expr_append(inner_add_twos, create_number_tp(2));
    s_expr_append(nested_add_twos_cmd, create_s_expr_tp(inner_add_twos));
    pass = pass && run_test_expect_number(env, nested_add_twos_cmd, 6);
    // (- 2 (+ 2 2)) -> -2
    s_expr* nested_subtract_twos_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(nested_subtract_twos_cmd, create_number_tp(2));
    inner_add_twos = unit_list(symbol_tp_from_name(env, "+"));
    s_expr_append(inner_add_twos, create_number_tp(2));
    s_expr_append(inner_add_twos, create_number_tp(2));
    s_expr_append(nested_subtract_twos_cmd, create_s_expr_tp(inner_add_twos));
    pass = pass && run_test_expect_number(env, nested_subtract_twos_cmd, -2);
    // (* 2 (+ 2 2)) -> 8
    s_expr* nested_multiply_twos_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(nested_multiply_twos_cmd, create_number_tp(2));
    inner_add_twos = unit_list(symbol_tp_from_name(env, "+"));
    s_expr_append(inner_add_twos, create_number_tp(2));
    s_expr_append(inner_add_twos, create_number_tp(2));
    s_expr_append(nested_multiply_twos_cmd, create_s_expr_tp(inner_add_twos));
    pass = pass && run_test_expect_number(env, nested_multiply_twos_cmd, 8);
    // (/ 10 (+ 1 1)) -> 5
    s_expr* nested_divide_twos_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(nested_divide_twos_cmd, create_number_tp(10));
    inner_add_twos = unit_list(symbol_tp_from_name(env, "+"));
    s_expr_append(inner_add_twos, create_number_tp(1));
    s_expr_append(inner_add_twos, create_number_tp(1));
    s_expr_append(nested_divide_twos_cmd, create_s_expr_tp(inner_add_twos));
    pass = pass && run_test_expect_number(env, nested_divide_twos_cmd, 5);
    // (+ 1 1 1) -> 3
    s_expr* triple_add_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(triple_add_cmd, create_number_tp(1));
    s_expr_append(triple_add_cmd, create_number_tp(1));
    s_expr_append(triple_add_cmd, create_number_tp(1));
    pass = pass && run_test_expect_number(env, triple_add_cmd, 3);
    // (- 10 1 1) -> 8
    s_expr* triple_subtract_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(triple_subtract_cmd, create_number_tp(10));
    s_expr_append(triple_subtract_cmd, create_number_tp(1));
    s_expr_append(triple_subtract_cmd, create_number_tp(1));
    pass = pass && run_test_expect_number(env, triple_subtract_cmd, 8);
    // (* 2 3 4) -> 24
    s_expr* triple_multiply_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(triple_multiply_cmd, create_number_tp(2));
    s_expr_append(triple_multiply_cmd, create_number_tp(3));
    s_expr_append(triple_multiply_cmd, create_number_tp(4));
    pass = pass && run_test_expect_number(env, triple_multiply_cmd, 24);
    // (/ 72 3 4) -> 6
    s_expr* triple_divide_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(triple_divide_cmd, create_number_tp(72));
    s_expr_append(triple_divide_cmd, create_number_tp(3));
    s_expr_append(triple_divide_cmd, create_number_tp(4));
    pass = pass && run_test_expect_number(env, triple_divide_cmd, 6);
    // (/ 72 0 2) -> EVAL_ERROR_DIV_ZERO
    s_expr* triple_divide_zero_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(triple_divide_zero_cmd, create_number_tp(72));
    s_expr_append(triple_divide_zero_cmd, create_number_tp(0));
    s_expr_append(triple_divide_zero_cmd, create_number_tp(2));
    out = eval_arithmetic(triple_divide_zero_cmd, env);
    if (!match_error(out, EVAL_ERROR_DIV_ZERO)) {
        pass = 0;
    }
    delete_s_expr_recursive(triple_divide_zero_cmd, true);
    free(out);
    // (/ 72 2 0) -> EVAL_ERROR_DIV_ZERO
    triple_divide_zero_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(triple_divide_zero_cmd, create_number_tp(72));
    s_expr_append(triple_divide_zero_cmd, create_number_tp(2));
    s_expr_append(triple_divide_zero_cmd, create_number_tp(0));
    out = eval_arithmetic(triple_divide_zero_cmd, env);
    if (!match_error(out, EVAL_ERROR_DIV_ZERO)) {
        pass = 0;
    }
    delete_s_expr_recursive(triple_divide_zero_cmd, true);
    free(out);
    // (+ 1 #t) -> EVAL_ERROR_NEED_NUM
    s_expr* arith_bool_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (+ #t 1) -> EVAL_ERROR_NEED_NUM
    arith_bool_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (- 1 #t) -> EVAL_ERROR_NEED_NUM
    arith_bool_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (- #t 1) -> EVAL_ERROR_NEED_NUM
    arith_bool_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (* 1 #t) -> EVAL_ERROR_NEED_NUM
    arith_bool_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (* #t 1) -> EVAL_ERROR_NEED_NUM
    arith_bool_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (/ 1 #t) -> EVAL_ERROR_NEED_NUM
    arith_bool_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (/ #t 1) -> EVAL_ERROR_NEED_NUM
    arith_bool_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(arith_bool_cmd, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(arith_bool_cmd, create_number_tp(1));
    out = eval_arithmetic(arith_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(arith_bool_cmd, true);
    free(out);
    // (+ 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    s_expr* literal_error_arg_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (+ EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    literal_error_arg_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (- 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    literal_error_arg_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (- EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    literal_error_arg_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (* 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    literal_error_arg_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (* EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    literal_error_arg_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (/ 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    literal_error_arg_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (/ EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    literal_error_arg_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(literal_error_arg_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(literal_error_arg_cmd, create_number_tp(1));
    out = eval_arithmetic(literal_error_arg_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(literal_error_arg_cmd, true);
    free(out);
    // (+ 1 (-)) -> EVAL_ERROR_FEW_ARGS
    s_expr* error_passthrough_cmd = unit_list(copy_typed_ptr(add));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    s_expr* lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    // (+ (-) 1) -> EVAL_ERROR_FEW_ARGS
    error_passthrough_cmd = unit_list(copy_typed_ptr(add));
    lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    // (- 1 (-)) -> EVAL_ERROR_FEW_ARGS
    error_passthrough_cmd = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    // (- (-) 1) -> EVAL_ERROR_FEW_ARGS
    error_passthrough_cmd = unit_list(copy_typed_ptr(subtract));
    lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    // (* 1 (-)) -> EVAL_ERROR_FEW_ARGS
    error_passthrough_cmd = unit_list(copy_typed_ptr(multiply));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    // (* (-) 1) -> EVAL_ERROR_FEW_ARGS
    error_passthrough_cmd = unit_list(copy_typed_ptr(multiply));
    lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    // (/ 1 (-)) -> EVAL_ERROR_FEW_ARGS
    error_passthrough_cmd = unit_list(copy_typed_ptr(divide));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    // (/ (-) 1) -> EVAL_ERROR_FEW_ARGS
    error_passthrough_cmd = unit_list(copy_typed_ptr(divide));
    lone_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(error_passthrough_cmd, create_s_expr_tp(lone_subtract));
    s_expr_append(error_passthrough_cmd, create_number_tp(1));
    out = eval_arithmetic(error_passthrough_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(error_passthrough_cmd, true);
    free(out);
    free(add);
    free(subtract);
    free(multiply);
    free(divide);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

bool check_bool_tp(typed_ptr* tp, bool expected) {
    return (tp != NULL && tp->type == TYPE_BOOL && tp->ptr.idx == expected);
}

// NOTE: frees cmd
bool run_compare_expect_bool(Environment* env, s_expr* cmd, bool expected) {
    typed_ptr* out = eval_comparison(cmd, env);
    bool passed = check_bool_tp(out, expected);
    delete_s_expr_recursive(cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
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
    typed_ptr* out = eval_comparison(cmd, env);
    bool passed = check_bool_tp(out, expected);
    delete_s_expr_recursive(cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    return passed;
}

void test_eval_comparison(test_env* te) {
    print_test_announce("eval_comparison()");
    // setup
    bool pass = 1;
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* subtract = symbol_tp_from_name(env, "-");
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
    // (= . 1) -> EVAL_ERROR_ILLEGAL_PAIR
    s_expr* pair_cmd = create_s_expr(copy_typed_ptr(eq_tp), \
                                     create_number_tp(1));
    typed_ptr* out = eval_comparison(pair_cmd, env);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(pair_cmd, true);
    free(out);
    // (= 1 . 2) -> EVAL_ERROR_ILLEGAL_PAIR
    pair_cmd = create_s_expr(create_number_tp(1), create_number_tp(2));
    pair_cmd = create_s_expr(copy_typed_ptr(eq_tp), create_s_expr_tp(pair_cmd));
    out = eval_comparison(pair_cmd, env);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(pair_cmd, true);
    free(out);
    // (= 1 2 . 3) -> EVAL_ERROR_ILLEGAL_PAIR
    pair_cmd = create_s_expr(create_number_tp(2), create_number_tp(3));
    pair_cmd = create_s_expr(create_number_tp(1), create_s_expr_tp(pair_cmd));
    pair_cmd = create_s_expr(copy_typed_ptr(eq_tp), create_s_expr_tp(pair_cmd));
    out = eval_comparison(pair_cmd, env);
    if (!match_error(out, EVAL_ERROR_ILLEGAL_PAIR)) {
        pass = 0;
    }
    delete_s_expr_recursive(pair_cmd, true);
    free(out);
    // (=) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_equal_cmd = unit_list(copy_typed_ptr(eq_tp));
    out = eval_comparison(lone_equal_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(lone_equal_cmd, true);
    free(out);
    // (<) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_less_cmd = unit_list(copy_typed_ptr(lt_tp));
    out = eval_comparison(lone_less_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(lone_less_cmd, true);
    free(out);
    // (>) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_greater_cmd = unit_list(copy_typed_ptr(gt_tp));
    out = eval_comparison(lone_greater_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(lone_greater_cmd, true);
    free(out);
    // (<=) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_greater_equal_cmd = unit_list(copy_typed_ptr(le_tp));
    out = eval_comparison(lone_greater_equal_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(lone_greater_equal_cmd, true);
    free(out);
    // (>=) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_less_equal_cmd = unit_list(copy_typed_ptr(ge_tp));
    out = eval_comparison(lone_less_equal_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(lone_less_equal_cmd, true);
    free(out);
    // (= 1) -> EVAL_ERROR_FEW_ARGS
    s_expr* equal_one_cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(equal_one_cmd, create_number_tp(1));
    out = eval_comparison(equal_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(equal_one_cmd, true);
    free(out);
    // (< 1) -> EVAL_ERROR_FEW_ARGS
    s_expr* less_one_cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(less_one_cmd, create_number_tp(1));
    out = eval_comparison(less_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(less_one_cmd, true);
    free(out);
    // (> 1) -> EVAL_ERROR_FEW_ARGS
    s_expr* greater_one_cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(greater_one_cmd, create_number_tp(1));
    out = eval_comparison(greater_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(greater_one_cmd, true);
    free(out);
    // (<= 1) -> EVAL_ERROR_FEW_ARGS
    s_expr* less_equal_one_cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(less_equal_one_cmd, create_number_tp(1));
    out = eval_comparison(less_equal_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(less_equal_one_cmd, true);
    free(out);
    // (>= 1) -> EVAL_ERROR_FEW_ARGS
    s_expr* greater_equal_one_cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(greater_equal_one_cmd, create_number_tp(1));
    out = eval_comparison(greater_equal_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(greater_equal_one_cmd, true);
    free(out);
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
    // (= 1 #t) -> EVAL_ERROR_NEED_NUM
    s_expr* cmp_bool_cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (= #t 1) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (< 1 #t) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (< #t 1) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (> 1 #t) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (> #t 1) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (<= 1 #t) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (<= #t 1) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (>= 1 #t) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (>= #t 1) -> EVAL_ERROR_NEED_NUM
    cmp_bool_cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(cmp_bool_cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmp_bool_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bool_cmd, env);
    if (!match_error(out, EVAL_ERROR_NEED_NUM)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bool_cmd, true);
    free(out);
    // (= 1 (- 3 2)) -> #t
    s_expr* nested_cmp_cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(nested_cmp_cmd, create_number_tp(1));
    s_expr* subtract_three_two = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_three_two, create_number_tp(3));
    s_expr_append(subtract_three_two, create_number_tp(2));
    s_expr_append(nested_cmp_cmd, create_s_expr_tp(subtract_three_two));
    pass = pass && run_compare_expect_bool(env, nested_cmp_cmd, true);
    // (< 1 (- 3 2)) -> #f
    nested_cmp_cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(nested_cmp_cmd, create_number_tp(1));
    subtract_three_two = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_three_two, create_number_tp(3));
    s_expr_append(subtract_three_two, create_number_tp(2));
    s_expr_append(nested_cmp_cmd, create_s_expr_tp(subtract_three_two));
    pass = pass && run_compare_expect_bool(env, nested_cmp_cmd, false);
    // (> 1 (- 3 2)) -> #f
    nested_cmp_cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(nested_cmp_cmd, create_number_tp(1));
    subtract_three_two = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_three_two, create_number_tp(3));
    s_expr_append(subtract_three_two, create_number_tp(2));
    s_expr_append(nested_cmp_cmd, create_s_expr_tp(subtract_three_two));
    pass = pass && run_compare_expect_bool(env, nested_cmp_cmd, false);
    // (<= 1 (- 3 2)) -> #t
    nested_cmp_cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(nested_cmp_cmd, create_number_tp(1));
    subtract_three_two = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_three_two, create_number_tp(3));
    s_expr_append(subtract_three_two, create_number_tp(2));
    s_expr_append(nested_cmp_cmd, create_s_expr_tp(subtract_three_two));
    pass = pass && run_compare_expect_bool(env, nested_cmp_cmd, true);
    // (>= 1 (- 3 2)) -> #t
    nested_cmp_cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(nested_cmp_cmd, create_number_tp(1));
    subtract_three_two = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_three_two, create_number_tp(3));
    s_expr_append(subtract_three_two, create_number_tp(2));
    s_expr_append(nested_cmp_cmd, create_s_expr_tp(subtract_three_two));
    pass = pass && run_compare_expect_bool(env, nested_cmp_cmd, true);
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
    // (= 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    s_expr* cmp_bare_error_cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (= EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (< 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (< EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (> 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (> EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (<= 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (<= EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (>= 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (>= EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    cmp_bare_error_cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(cmp_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(cmp_bare_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_bare_error_cmd, true);
    free(out);
    // (= 1 (-)) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmp_eval_error_cmd = unit_list(copy_typed_ptr(eq_tp));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    s_expr* bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (= (-) 1) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(eq_tp));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (< 1 (-)) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(lt_tp));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (< (-) 1) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(lt_tp));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (> 1 (-)) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(gt_tp));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (> (-) 1) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(gt_tp));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (<= 1 (-)) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(le_tp));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (<= (-) 1) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(le_tp));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (>= 1 (-)) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(ge_tp));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    // (>= (-) 1) -> EVAL_ERROR_FEW_ARGS
    cmp_eval_error_cmd = unit_list(copy_typed_ptr(ge_tp));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cmp_eval_error_cmd, create_s_expr_tp(bare_subtract));
    s_expr_append(cmp_eval_error_cmd, create_number_tp(1));
    out = eval_comparison(cmp_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(cmp_eval_error_cmd, true);
    free(out);
    free(eq_tp);
    free(lt_tp);
    free(gt_tp);
    free(le_tp);
    free(ge_tp);
    free(subtract);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_exit(test_env* te) {
    print_test_announce("eval_exit()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    bool pass = 1;
    // (exit) -> EVAL_ERROR_EXIT
    s_expr* exit_cmd = unit_list(builtin_tp_from_name(env, "exit"));
    typed_ptr* out = eval_exit(exit_cmd, env);
    if (!match_error(out, EVAL_ERROR_EXIT)) {
        pass = 0;
    }
    delete_s_expr_recursive(exit_cmd, true);
    free(out);
    // (exit 1) -> EVAL_ERROR_MANY_ARGS
    exit_cmd = unit_list(builtin_tp_from_name(env, "exit"));
    s_expr_append(exit_cmd, create_number_tp(1));
    out = eval_exit(exit_cmd, env);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(exit_cmd, true);
    free(out);
    // (exit EVAL_ERROR_NOT_ID) -> EVAL_ERROR_MANY_ARGS
    exit_cmd = unit_list(builtin_tp_from_name(env, "exit"));
    s_expr_append(exit_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_exit(exit_cmd, env);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(exit_cmd, true);
    free(out);
    // (exit (-)) -> EVAL_ERROR_MANY_ARGS
    exit_cmd = unit_list(builtin_tp_from_name(env, "exit"));
    s_expr* lone_subtract = unit_list(symbol_tp_from_name(env, "-"));
    s_expr_append(exit_cmd, create_s_expr_tp(lone_subtract));
    out = eval_exit(exit_cmd, env);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = 0;
    }
    delete_s_expr_recursive(exit_cmd, true);
    free(out);
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
    typed_ptr* subtract = symbol_tp_from_name(env, "-");
    bool pass = true;
    // (cons) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_cons_cmd = unit_list(copy_typed_ptr(cons));
    typed_ptr* out = eval_cons(lone_cons_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(lone_cons_cmd, true);
    free(out);
    // (cons 1) -> EVAL_ERROR_FEW_ARGS
    s_expr* cons_one_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_one_cmd, create_number_tp(1));
    out = eval_cons(cons_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(cons_one_cmd, true);
    free(out);
    // (cons 1 #t) -> '(1 . #t)
    s_expr* cons_one_true_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_one_true_cmd, create_number_tp(1));
    s_expr_append(cons_one_true_cmd, create_atom_tp(TYPE_BOOL, 1));
    out = eval_cons(cons_one_true_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        out->ptr.se_ptr->car == NULL || \
        out->ptr.se_ptr->car->type != TYPE_NUM || \
        out->ptr.se_ptr->car->ptr.idx != 1 || \
        out->ptr.se_ptr->cdr == NULL || \
        out->ptr.se_ptr->cdr->type != TYPE_BOOL || \
        out->ptr.se_ptr->cdr->ptr.idx != 1) {
        pass = false;
    }
    delete_s_expr_recursive(cons_one_true_cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (cons 1 #t 2) -> EVAL_ERROR_MANY_ARGS
    s_expr* cons_one_true_two_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_one_true_two_cmd, create_number_tp(1));
    s_expr_append(cons_one_true_two_cmd, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(cons_one_true_two_cmd, create_number_tp(2));
    out = eval_cons(cons_one_true_two_cmd, env);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(cons_one_true_two_cmd, true);
    free(out);
    // (cons (- 3 1) #t) -> '(2 . #t)
    s_expr* cons_subtract_true_cmd = unit_list(copy_typed_ptr(cons));
    s_expr* subtract_three_one = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_three_one, create_number_tp(3));
    s_expr_append(subtract_three_one, create_number_tp(1));
    s_expr_append(cons_subtract_true_cmd, create_s_expr_tp(subtract_three_one));
    s_expr_append(cons_subtract_true_cmd, create_atom_tp(TYPE_BOOL, 1));
    out = eval_cons(cons_subtract_true_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        out->ptr.se_ptr->car == NULL || \
        out->ptr.se_ptr->car->type != TYPE_NUM || \
        out->ptr.se_ptr->car->ptr.idx != 2 || \
        out->ptr.se_ptr->cdr == NULL || \
        out->ptr.se_ptr->cdr->type != TYPE_BOOL || \
        out->ptr.se_ptr->cdr->ptr.idx != 1) {
        pass = false;
    }
    delete_s_expr_recursive(cons_subtract_true_cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (cons #t (- 3 1)) -> '(#t . 2)
    s_expr* cons_true_subtract_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_true_subtract_cmd, create_atom_tp(TYPE_BOOL, 1));
    subtract_three_one = unit_list(copy_typed_ptr(subtract));
    s_expr_append(subtract_three_one, create_number_tp(3));
    s_expr_append(subtract_three_one, create_number_tp(1));
    s_expr_append(cons_true_subtract_cmd, create_s_expr_tp(subtract_three_one));
    out = eval_cons(cons_true_subtract_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        out->ptr.se_ptr->car == NULL || \
        out->ptr.se_ptr->car->type != TYPE_BOOL || \
        out->ptr.se_ptr->car->ptr.idx != 1 || \
        out->ptr.se_ptr->cdr == NULL || \
        out->ptr.se_ptr->cdr->type != TYPE_NUM || \
        out->ptr.se_ptr->cdr->ptr.idx != 2) {
        pass = false;
    }
    delete_s_expr_recursive(cons_true_subtract_cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (cons EVAL_ERROR_NOT_ID 1) -> EVAL_ERROR_NOT_ID
    s_expr* cons_err_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_err_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(cons_err_cmd, create_number_tp(1));
    out = eval_cons(cons_err_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = false;
    }
    delete_s_expr_recursive(cons_err_cmd, true);
    free(out);
    // (cons 1 EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    cons_err_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_err_cmd, create_number_tp(1));
    s_expr_append(cons_err_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_cons(cons_err_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = false;
    }
    delete_s_expr_recursive(cons_err_cmd, true);
    free(out);
    // (cons (-) 1) -> EVAL_ERROR_FEW_ARGS
    s_expr* cons_eval_err_cmd = unit_list(copy_typed_ptr(cons));
    s_expr* bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cons_eval_err_cmd, create_s_expr_tp(bare_subtract));
    s_expr_append(cons_eval_err_cmd, create_number_tp(1));
    out = eval_cons(cons_eval_err_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(cons_eval_err_cmd, true);
    free(out);
    // (cons 1 (-)) -> EVAL_ERROR_FEW_ARGS
    cons_eval_err_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_eval_err_cmd, create_number_tp(1));
    bare_subtract = unit_list(copy_typed_ptr(subtract));
    s_expr_append(cons_eval_err_cmd, create_s_expr_tp(bare_subtract));
    out = eval_cons(cons_eval_err_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(cons_eval_err_cmd, true);
    free(out);
    // (cons 1 (cons 2 (cons 3 null))) -> '(1 2 3)
    typed_ptr* cons_sym = symbol_tp_from_name(env, "cons");
    s_expr* cons_one_list_cmd = unit_list(copy_typed_ptr(cons));
    s_expr_append(cons_one_list_cmd, create_number_tp(1));
    s_expr* cons_two_list = unit_list(copy_typed_ptr(cons_sym));
    s_expr_append(cons_two_list, create_number_tp(2));
    s_expr* cons_three_list = unit_list(copy_typed_ptr(cons_sym));
    s_expr_append(cons_three_list, create_number_tp(3));
    s_expr_append(cons_three_list, symbol_tp_from_name(env, "null"));
    s_expr_append(cons_two_list, create_s_expr_tp(cons_three_list));
    s_expr_append(cons_one_list_cmd, create_s_expr_tp(cons_two_list));
    out = eval_cons(cons_one_list_cmd, env);
    s_expr* expected = unit_list(create_number_tp(1));
    s_expr_append(expected, create_number_tp(2));
    s_expr_append(expected, create_number_tp(3));
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = false;
    }
    delete_s_expr_recursive(cons_one_list_cmd, true);
    delete_s_expr_recursive(expected, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    free(cons);
    free(subtract);
    free(cons_sym);
    delete_environment_full(env);
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
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* subtract_sym = symbol_tp_from_name(env, "-");
    bool pass = true;
    // (car) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_car_cmd = unit_list(copy_typed_ptr(car));
    typed_ptr* out = eval_car_cdr(lone_car_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(lone_car_cmd, true);
    free(out);
    // (cdr) -> EVAL_ERROR_FEW_ARGS
    s_expr* lone_cdr_cmd = unit_list(copy_typed_ptr(cdr));
    out = eval_car_cdr(lone_cdr_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(lone_cdr_cmd, true);
    free(out);
    // (car 1) -> EVAL_ERROR_BAD_ARG_TYPE
    s_expr* car_one_cmd = unit_list(copy_typed_ptr(car));
    s_expr_append(car_one_cmd, create_number_tp(1));
    out = eval_car_cdr(car_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_BAD_ARG_TYPE)) {
        pass = false;
    }
    delete_s_expr_recursive(car_one_cmd, true);
    free(out);
    // (cdr 1) -> EVAL_ERROR_BAD_ARG_TYPE
    s_expr* cdr_one_cmd = unit_list(copy_typed_ptr(cdr));
    s_expr_append(cdr_one_cmd, create_number_tp(1));
    out = eval_car_cdr(cdr_one_cmd, env);
    if (!match_error(out, EVAL_ERROR_BAD_ARG_TYPE)) {
        pass = false;
    }
    delete_s_expr_recursive(cdr_one_cmd, true);
    free(out);
    // (car <a list of 1 2>) -> EVAL_ERROR_CAR_NOT_CALLABLE
    s_expr* car_bare_list_cmd = unit_list(copy_typed_ptr(car));
    s_expr* bare_list = unit_list(create_number_tp(1));
    s_expr_append(bare_list, create_number_tp(2));
    s_expr_append(car_bare_list_cmd, create_s_expr_tp(bare_list));
    out = eval_car_cdr(car_bare_list_cmd, env);
    if (!match_error(out, EVAL_ERROR_CAR_NOT_CALLABLE)) {
        pass = false;
    }
    delete_s_expr_recursive(car_bare_list_cmd, true);
    free(out);
    // (cdr <a list of 1 2>) -> EVAL_ERROR_NOT_CALLABLE
    s_expr* cdr_bare_list_cmd = unit_list(copy_typed_ptr(cdr));
    bare_list = unit_list(create_number_tp(1));
    s_expr_append(bare_list, create_number_tp(2));
    s_expr_append(cdr_bare_list_cmd, create_s_expr_tp(bare_list));
    out = eval_car_cdr(cdr_bare_list_cmd, env);
    if (!match_error(out, EVAL_ERROR_CAR_NOT_CALLABLE)) {
        pass = false;
    }
    delete_s_expr_recursive(cdr_bare_list_cmd, true);
    free(out);
    // (car (list 1 2)) -> 1
    s_expr* car_list_cmd = unit_list(copy_typed_ptr(car));
    s_expr* list_call = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_call, create_number_tp(1));
    s_expr_append(list_call, create_number_tp(2));
    s_expr_append(car_list_cmd, create_s_expr_tp(list_call));
    out = eval_car_cdr(car_list_cmd, env);
    if (!check_typed_ptr(out, TYPE_NUM, (tp_value){.idx=1})) {
        pass = false;
    }
    delete_s_expr_recursive(car_list_cmd, true);
    free(out);
    // (cdr (list 1 2)) -> '(2)
    s_expr* cdr_list_cmd = unit_list(copy_typed_ptr(cdr));
    list_call = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_call, create_number_tp(1));
    s_expr_append(list_call, create_number_tp(2));
    s_expr_append(cdr_list_cmd, create_s_expr_tp(list_call));
    s_expr* expected = unit_list(create_number_tp(2));
    out = eval_car_cdr(cdr_list_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = false;
    }
    delete_s_expr_recursive(cdr_list_cmd, true);
    delete_s_expr_recursive(expected, true);
    free(out);
    // (car (list 1) (list 2)) -> EVAL_ERROR_MANY_ARGS
    s_expr* car_two_lists_cmd = unit_list(copy_typed_ptr(car));
    s_expr* list_one = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_one, create_number_tp(1));
    s_expr* list_two = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_two, create_number_tp(1));
    s_expr_append(car_two_lists_cmd, create_s_expr_tp(list_one));
    s_expr_append(car_two_lists_cmd, create_s_expr_tp(list_two));
    out = eval_car_cdr(car_two_lists_cmd, env);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(car_two_lists_cmd, true);
    free(out);
    // (cdr (list 1) (list 2)) -> EVAL_ERROR_MANY_ARGS
    s_expr* cdr_two_lists_cmd = unit_list(copy_typed_ptr(cdr));
    list_one = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_one, create_number_tp(1));
    list_two = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_two, create_number_tp(1));
    s_expr_append(cdr_two_lists_cmd, create_s_expr_tp(list_one));
    s_expr_append(cdr_two_lists_cmd, create_s_expr_tp(list_two));
    out = eval_car_cdr(cdr_two_lists_cmd, env);
    if (!match_error(out, EVAL_ERROR_MANY_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(cdr_two_lists_cmd, true);
    free(out);
    // (car EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    s_expr* car_bare_error_cmd = unit_list(copy_typed_ptr(car));
    s_expr_append(car_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_car_cdr(car_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = false;
    }
    delete_s_expr_recursive(car_bare_error_cmd, true);
    free(out);
    // (cdr EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    s_expr* cdr_bare_error_cmd = unit_list(copy_typed_ptr(cdr));
    s_expr_append(cdr_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_car_cdr(cdr_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = false;
    }
    delete_s_expr_recursive(cdr_bare_error_cmd, true);
    free(out);
    // (car (-)) -> EVAL_ERROR_FEW_ARGS
    s_expr* car_eval_error_cmd = unit_list(copy_typed_ptr(car));
    s_expr* lone_subtract = unit_list(copy_typed_ptr(subtract_sym));
    s_expr_append(car_eval_error_cmd, create_s_expr_tp(lone_subtract));
    out = eval_car_cdr(car_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(car_eval_error_cmd, true);
    free(out);
    // (cdr (-)) -> EVAL_ERROR_FEW_ARGS
    s_expr* cdr_eval_error_cmd = unit_list(copy_typed_ptr(cdr));
    lone_subtract = unit_list(copy_typed_ptr(subtract_sym));
    s_expr_append(cdr_eval_error_cmd, create_s_expr_tp(lone_subtract));
    out = eval_car_cdr(cdr_eval_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(cdr_eval_error_cmd, true);
    free(out);
    delete_environment_full(env);
    free(car);
    free(cdr);
    free(list_sym);
    free(subtract_sym);
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
    typed_ptr* subtract_sym = symbol_tp_from_name(env, "-");
    bool pass = true;
    // (list) -> '()
    s_expr* lone_list_cmd = unit_list(copy_typed_ptr(list));
    typed_ptr* out = eval_list_construction(lone_list_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !is_empty_list(out->ptr.se_ptr)) {
        pass = false;
    }
    delete_s_expr_recursive(lone_list_cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (list 1) -> '(1)
    s_expr* list_one_cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(list_one_cmd, create_number_tp(1));
    s_expr* expected = unit_list(create_number_tp(1));
    out = eval_list_construction(list_one_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = false;
    }
    delete_s_expr_recursive(list_one_cmd, true);
    delete_s_expr_recursive(expected, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (list 1 #t) -> '(1 #t)
    s_expr* list_one_true_cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(list_one_true_cmd, create_number_tp(1));
    s_expr_append(list_one_true_cmd, create_atom_tp(TYPE_BOOL, 1));
    expected = unit_list(create_number_tp(1));
    s_expr_append(expected, create_atom_tp(TYPE_BOOL, 1));
    out = eval_list_construction(list_one_true_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = false;
    }
    delete_s_expr_recursive(list_one_true_cmd, true);
    delete_s_expr_recursive(expected, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (list 1 #t (- 3 1)) -> '(1 #t 2)
    s_expr* list_one_true_subtract_cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(list_one_true_subtract_cmd, create_number_tp(1));
    s_expr_append(list_one_true_subtract_cmd, create_atom_tp(TYPE_BOOL, 1));
    s_expr* subtract_three_one = unit_list(copy_typed_ptr(subtract_sym));
    s_expr_append(subtract_three_one, create_number_tp(3));
    s_expr_append(subtract_three_one, create_number_tp(1));
    s_expr_append(list_one_true_subtract_cmd, \
                  create_s_expr_tp(subtract_three_one));
    expected = unit_list(create_number_tp(1));
    s_expr_append(expected, create_atom_tp(TYPE_BOOL, 1));
    s_expr_append(expected, create_number_tp(2));
    out = eval_list_construction(list_one_true_subtract_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = false;
    }
    delete_s_expr_recursive(list_one_true_subtract_cmd, true);
    delete_s_expr_recursive(expected, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (list EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    s_expr* list_bare_error_cmd = unit_list(copy_typed_ptr(list));
    s_expr_append(list_bare_error_cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    out = eval_list_construction(list_bare_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_NOT_ID)) {
        pass = false;
    }
    delete_s_expr_recursive(list_bare_error_cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (list (-)) -> EVAL_ERROR_FEW_ARGS
    s_expr* list_subtract_error_cmd = unit_list(copy_typed_ptr(list));
    s_expr* lone_subtract = unit_list(copy_typed_ptr(subtract_sym));
    s_expr_append(list_subtract_error_cmd, create_s_expr_tp(lone_subtract));
    out = eval_list_construction(list_subtract_error_cmd, env);
    if (!match_error(out, EVAL_ERROR_FEW_ARGS)) {
        pass = false;
    }
    delete_s_expr_recursive(list_subtract_error_cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // (list (cons 1 null) 2) -> '((1) 2)
    s_expr* list_nested_cmd = unit_list(copy_typed_ptr(list));
    s_expr* cons_one_null = unit_list(symbol_tp_from_name(env, "cons"));
    s_expr_append(cons_one_null, create_number_tp(1));
    s_expr_append(cons_one_null, symbol_tp_from_name(env, "null"));
    s_expr_append(list_nested_cmd, create_s_expr_tp(cons_one_null));
    s_expr_append(list_nested_cmd, create_number_tp(2));
    expected = unit_list(create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(expected, create_number_tp(2));
    out = eval_list_construction(list_nested_cmd, env);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !match_s_exprs(out->ptr.se_ptr, expected)) {
        pass = false;
    }
    delete_s_expr_recursive(list_nested_cmd, true);
    delete_s_expr_recursive(expected, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    delete_environment_full(env);
    free(list);
    free(subtract_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

bool deep_match_typed_ptrs(typed_ptr* first, typed_ptr* second) {
    if (first == NULL && second == NULL) {
        return true;
    } else if (first == NULL || second == NULL) {
        return false;
    } else if (first->type != second->type) {
        return false;
    } else {
        switch (first->type) {
            case TYPE_SEXPR:
                return match_s_exprs(first->ptr.se_ptr, second->ptr.se_ptr);
            default:
                return first->ptr.idx == second->ptr.idx;
        }
    }
}

// NOTE: frees cmd AND expected
bool run_test_expect(typed_ptr* (*function)(const s_expr*, Environment*), \
                     s_expr* cmd, \
                     Environment* env, \
                     typed_ptr* expected) {
    typed_ptr* out = (*function)(cmd, env);
    bool passed = deep_match_typed_ptrs(out, expected);
    delete_s_expr_recursive(cmd, true);
    if (out != NULL && out->type == TYPE_SEXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    if (expected != NULL && expected->type == TYPE_SEXPR) {
        delete_s_expr_recursive(expected->ptr.se_ptr, true);
    }
    free(expected);
    return passed;
}

void test_eval_and_or(test_env* te) {
    print_test_announce("eval_and_or()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* and_builtin = builtin_tp_from_name(env, "and");
    typed_ptr* or_builtin = builtin_tp_from_name(env, "or");
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* cond_sym = symbol_tp_from_name(env, "cond");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* define_sym = symbol_tp_from_name(env, "define");
    typed_ptr* boolpred_sym = symbol_tp_from_name(env, "boolean?");
    typed_ptr* x_sym;
    x_sym = install_symbol(env, strdup("x"), TYPE_UNDEF, (tp_value){.idx=0});
    bool pass = true;
    // (and) -> #t
    s_expr* cmd = unit_list(copy_typed_ptr(and_builtin));
    typed_ptr* expected = create_atom_tp(TYPE_BOOL, 1);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and 1) -> 1
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #f) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #t #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #t #f) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #f #t) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #f #f) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and null #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, copy_typed_ptr(null_sym));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and 0 #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and (cond) #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(cond_sym))));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #t 1) -> 1
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #t (+ 1 1)) -> 2
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr* add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(add_one_one));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #t #t #t) -> #t
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and #t #f #t) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (and 1 2 3 #f #t) -> #f
    cmd = unit_list(copy_typed_ptr(and_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
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
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    typed_ptr* curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_NUM || \
        curr_x_val->ptr.idx != 2) {
        pass = 0;
    }
    free(curr_x_val);
    // (and (define x 3) #f (define x 4)) -> #f AND SIDE EFFECT x == 4
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
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_NUM || \
        curr_x_val->ptr.idx != 4) {
        pass = 0;
    }
    free(curr_x_val);
    // (or) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    expected = create_atom_tp(TYPE_BOOL, 0);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or 1) -> 1
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #t) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #t #t) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #t #f) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f #t) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f #f) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or null #t) -> null
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, copy_typed_ptr(null_sym));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_s_expr_tp(create_empty_s_expr());
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or 0 #t) -> 0
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_number_tp(0));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_number_tp(0);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or (cond) #t) -> <void>
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(cond_sym))));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f 1) -> 1
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f (+ 1 1)) -> 2
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(add_one_one));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f #f #f) -> #f
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f #t #f) -> #t
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    // (or #f #f #f 1 #f) -> 1
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
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
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_NUM || \
        curr_x_val->ptr.idx != 6) {
        pass = 0;
    }
    free(curr_x_val);
    // (or (define x 7) (define x 8)) -> <void> AND SIDE EFFECT x == 8
    cmd = unit_list(copy_typed_ptr(or_builtin));
    s_expr* define_x_seven = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_seven, copy_typed_ptr(x_sym));
    s_expr_append(define_x_seven, create_number_tp(7));
    s_expr* define_x_eight = unit_list(copy_typed_ptr(define_sym));
    s_expr_append(define_x_eight, copy_typed_ptr(x_sym));
    s_expr_append(define_x_eight, create_number_tp(8));
    s_expr_append(cmd, create_s_expr_tp(define_x_seven));
    s_expr_append(cmd, create_s_expr_tp(define_x_eight));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_and_or, cmd, env, expected);
    curr_x_val = value_lookup_index(env, x_sym);
    if (curr_x_val == NULL || \
        curr_x_val->type != TYPE_NUM || \
        curr_x_val->ptr.idx != 8) {
        pass = 0;
    }
    free(curr_x_val);
    delete_environment_full(env);
    free(and_builtin);
    free(or_builtin);
    free(null_sym);
    free(cond_sym);
    free(add_sym);
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
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* cond_sym = symbol_tp_from_name(env, "cond");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* subtract_sym = symbol_tp_from_name(env, "-");
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
    s_expr_append(cmd, copy_typed_ptr(null_sym));
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
    s_expr* list_one_two = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_one_two, create_number_tp(1));
    s_expr_append(list_one_two, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(list_one_two));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not #f) -> #t
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr_append(cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    // (not (-)) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(not_builtin));
    s_expr* lone_subtract = unit_list(copy_typed_ptr(subtract_sym));
    s_expr_append(cmd, create_s_expr_tp(lone_subtract));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_not, cmd, env, expected);
    delete_environment_full(env);
    free(not_builtin);
    free(null_sym);
    free(cond_sym);
    free(list_sym);
    free(subtract_sym);
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
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* cons_sym = symbol_tp_from_name(env, "cons");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* subtract_sym = symbol_tp_from_name(env, "-");
    bool pass = true;
    // (list?) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(listpred_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? 1) -> #f
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? #t) -> #f
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? (cons 1 2)) -> #f
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr* cons_subexpr = unit_list(copy_typed_ptr(cons_sym));
    s_expr_append(cons_subexpr, create_number_tp(1));
    s_expr_append(cons_subexpr, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(cons_subexpr));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? (list 1 2)) -> #t
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr* list_subexpr = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_subexpr, create_number_tp(1));
    s_expr_append(list_subexpr, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(list_subexpr));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? null) -> #t
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, copy_typed_ptr(null_sym));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr_append(cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    // (list? (-)) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(listpred_builtin));
    s_expr* lone_subtract = unit_list(copy_typed_ptr(subtract_sym));
    s_expr_append(cmd, create_s_expr_tp(lone_subtract));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_list_pred, cmd, env, expected);
    delete_environment_full(env);
    free(listpred_builtin);
    free(null_sym);
    free(cons_sym);
    free(list_sym);
    free(subtract_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

typed_ptr* wrapper_eval_undef_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_UNDEF);
}

typed_ptr* wrapper_eval_error_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_ERROR);
}

typed_ptr* wrapper_eval_void_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_VOID);
}

typed_ptr* wrapper_eval_num_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_NUM);
}

typed_ptr* wrapper_eval_bool_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_BOOL);
}

typed_ptr* wrapper_eval_builtin_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_BUILTIN);
}

typed_ptr* wrapper_eval_pair_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_SEXPR);
}

typed_ptr* wrapper_eval_symbol_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_SYMBOL);
}

typed_ptr* wrapper_eval_function_pred(const s_expr* se, Environment* env) {
    return eval_atom_pred(se, env, TYPE_FUNCTION);
}

void test_eval_atom_pred(test_env* te) {
    print_test_announce("eval_list_pred()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* cons_sym = symbol_tp_from_name(env, "cons");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* x_sym;
    x_sym = install_symbol(env, strdup("x"), TYPE_FUNCTION, (tp_value){.idx=0});
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    bool pass = true;
    #define NUM_TYPES 9
    type type_list[NUM_TYPES] = {TYPE_UNDEF, TYPE_ERROR, TYPE_VOID, \
                                 TYPE_NUM, TYPE_BOOL, TYPE_BUILTIN, \
                                 TYPE_SEXPR, TYPE_SYMBOL, TYPE_FUNCTION};
    typed_ptr* (*pred_fns[NUM_TYPES])(const s_expr*, Environment*) = \
                                {wrapper_eval_undef_pred, \
                                 wrapper_eval_error_pred, \
                                 wrapper_eval_void_pred, \
                                 wrapper_eval_num_pred, \
                                 wrapper_eval_bool_pred, \
                                 wrapper_eval_builtin_pred, \
                                 wrapper_eval_pair_pred, \
                                 wrapper_eval_symbol_pred, \
                                 wrapper_eval_function_pred};
    s_expr* cmd = NULL;
    typed_ptr* expected = NULL;
    // ([any_atomic]?) -> EVAL_ERROR_FEW_ARGS
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? 1 2) -> EVAL_ERROR_MANY_ARGS
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, create_number_tp(1));
        s_expr_append(cmd, create_number_tp(2));
        expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // there's currently no way to run these predicates on an undefined symbol,
    //   because they just get an EVAL_ERROR_UNDEF_SYM error
    // so we skip testing this predicate
    // ([any_atomic]? EVAL_ERROR_NOT_ID) -> EVAL_ERROR_NOT_ID
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, create_error_tp(EVAL_ERROR_NOT_ID));
        expected = create_error_tp(EVAL_ERROR_NOT_ID);
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? <void>) -> #t if [void] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, create_atom_tp(TYPE_VOID, 0));
        if (type_list[i] == TYPE_VOID) {
            expected = create_atom_tp(TYPE_BOOL, true);
        } else {
            expected = create_atom_tp(TYPE_BOOL, false);
        }
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? 1) -> #t if [num] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, create_number_tp(1));
        if (type_list[i] == TYPE_NUM) {
            expected = create_atom_tp(TYPE_BOOL, true);
        } else {
            expected = create_atom_tp(TYPE_BOOL, false);
        }
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? #f) -> #t if [bool] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
        if (type_list[i] == TYPE_BOOL) {
            expected = create_atom_tp(TYPE_BOOL, true);
        } else {
            expected = create_atom_tp(TYPE_BOOL, false);
        }
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? +) -> #t if [builtin] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, copy_typed_ptr(add_sym));
        if (type_list[i] == TYPE_BUILTIN) {
            expected = create_atom_tp(TYPE_BOOL, true);
        } else {
            expected = create_atom_tp(TYPE_BOOL, false);
        }
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? (cons 1 2)) -> #t if [pair] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr* cons_subexpr = unit_list(copy_typed_ptr(cons_sym));
        s_expr_append(cons_subexpr, create_number_tp(1));
        s_expr_append(cons_subexpr, create_number_tp(2));
        s_expr_append(cmd, create_s_expr_tp(cons_subexpr));
        if (type_list[i] == TYPE_SEXPR) {
            expected = create_atom_tp(TYPE_BOOL, true);
        } else {
            expected = create_atom_tp(TYPE_BOOL, false);
        }
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? (list 1 2)) -> #t if [pair] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr* list_subexpr = unit_list(copy_typed_ptr(list_sym));
        s_expr_append(list_subexpr, create_number_tp(1));
        s_expr_append(list_subexpr, create_number_tp(2));
        s_expr_append(cmd, create_s_expr_tp(list_subexpr));
        if (type_list[i] == TYPE_SEXPR) {
            expected = create_atom_tp(TYPE_BOOL, true);
        } else {
            expected = create_atom_tp(TYPE_BOOL, false);
        }
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? 'x) -> #t if [symbol] else #f
    //   there's not currently any way to obtain a bare symbol typed-pointer
    //   because the arguments are evaluated. Once I write (quote ...) I'll be
    //   able to test this...
    // (define ...) + ([any_atomic]? fn) -> #t if [function] else #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, copy_typed_ptr(x_sym));
        if (type_list[i] == TYPE_FUNCTION) {
            expected = create_atom_tp(TYPE_BOOL, true);
        } else {
            expected = create_atom_tp(TYPE_BOOL, false);
        }
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? null) -> #f
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr_append(cmd, copy_typed_ptr(null_sym));
        expected = create_atom_tp(TYPE_BOOL, false);
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    // ([any_atomic]? (/ 0)) -> EVAL_ERROR_DIV_ZERO
    for (unsigned int i = 0; i < NUM_TYPES; i++) {
        cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
        s_expr* divide_subexpr = unit_list(copy_typed_ptr(divide_sym));
        s_expr_append(divide_subexpr, create_number_tp(0));
        s_expr_append(cmd, create_s_expr_tp(divide_subexpr));
        expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
        pass = pass && run_test_expect(pred_fns[i], cmd, env, expected);
    }
    delete_environment_full(env);
    free(add_sym);
    free(cons_sym);
    free(list_sym);
    free(null_sym);
    free(x_sym);
    free(divide_sym);
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
    x_sym = install_symbol(env, strdup("x"), TYPE_UNDEF, (tp_value){.idx=0});
    y_sym = install_symbol(env, strdup("y"), TYPE_UNDEF, (tp_value){.idx=0});
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    bool pass = true;
    // (lambda) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(lambda_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
    // (lambda x) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
    // (lambda 1 1) -> EVAL_ERROR_BAD_ARG_TYPE
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
    // (lambda (x 1) 2) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr* x_one = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(x_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(x_one));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
    // (lambda (<weird symbol number>) 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr* arg_list = unit_list(create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_s_expr_tp(arg_list));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
    // (lambda x 1) -> currently EVAL_ERROR_BAD_ARG_TYPE; should be ok
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
    // (lambda (x) 1 2) -> currently EVAL_ERROR_MANY_ARGS; should be ok
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr* lone_x = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(lone_x));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
    // (lambda null 1) -> currently EVAL_ERROR_BAD_ARG_TYPE; should be ok
    cmd = unit_list(copy_typed_ptr(lambda_builtin));
    s_expr_append(cmd, copy_typed_ptr(null_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    pass = pass && run_test_expect(eval_lambda, cmd, env, expected);
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
        resulting_fn->arg_list == NULL || \
        strcmp(resulting_fn->arg_list->name, "x") || \
        resulting_fn->arg_list->next != NULL || \
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
        resulting_fn->arg_list != NULL || \
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
    body = create_s_expr_tp(unit_list(copy_typed_ptr(add_sym)));
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
        resulting_fn->arg_list == NULL || \
        strcmp(resulting_fn->arg_list->name, "x") || \
        resulting_fn->arg_list->next == NULL || \
        strcmp(resulting_fn->arg_list->next->name, "y") || \
        resulting_fn->arg_list->next->next != NULL || \
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
    body = create_s_expr_tp(unit_list(copy_typed_ptr(add_sym)));
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
        resulting_fn->arg_list == NULL || \
        strcmp(resulting_fn->arg_list->name, "x") || \
        resulting_fn->arg_list->next != NULL || \
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
    body = create_s_expr_tp(unit_list(copy_typed_ptr(divide_sym)));
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
        resulting_fn->arg_list != NULL || \
        resulting_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(resulting_fn->body, body)) {
        pass = false;
    }
    delete_s_expr_recursive(cmd, true);
    free(expected);
    free(out);
    delete_environment_full(env);
    free(lambda_builtin);
    free(add_sym);
    free(null_sym);
    free(x_sym);
    free(y_sym);
    free(divide_sym);
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
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* equals_sym = symbol_tp_from_name(env, "=");
    typed_ptr* subtract_sym = symbol_tp_from_name(env, "-");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    bool pass = true;
    // (cond) -> <void>
    s_expr* cmd = unit_list(copy_typed_ptr(cond_builtin));
    typed_ptr* expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond 1) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond ()) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    s_expr* first_case = create_empty_s_expr();
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t)) -> #t
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (1)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (1 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_number_tp(1));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (() 1)) -> EVAL_ERROR_MISSING_PROCEDURE
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_s_expr_tp(create_empty_s_expr()));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (null 1)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(copy_typed_ptr(null_sym));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#f)) -> <void>
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#f 2)) -> <void>
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t 1) 2) -> currently 1, but it should be EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t 1) (#t 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    s_expr* second_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#f 1) (#t 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t 1) (#f 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
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
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
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
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (else 1)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#f 1) (else)) -> EVAL_ERROR_EMPTY_ELSE
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_error_tp(EVAL_ERROR_EMPTY_ELSE);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#f 1) (else 2)) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t 1) (else 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (else 2) (#t 1)) -> EVAL_ERROR_NONTERMINAL_ELSE
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(first_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(second_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_error_tp(EVAL_ERROR_NONTERMINAL_ELSE);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond ((+ 1 1))) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = create_empty_s_expr();
    s_expr* add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(first_case, create_s_expr_tp(add_one_one));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond ((= 1 1) (+ 1 1))) -> 2
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = create_empty_s_expr();
    s_expr* equals_one_one = unit_list(copy_typed_ptr(equals_sym));
    s_expr_append(equals_one_one, create_number_tp(1));
    s_expr_append(equals_one_one, create_number_tp(1));
    s_expr_append(first_case, create_s_expr_tp(equals_one_one));
    add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(first_case, create_s_expr_tp(add_one_one));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#f (/ 0))) -> <void>
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr* divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(first_case, create_s_expr_tp(divide_zero));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t 1) ((/ 0) 2)) -> 1
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    second_case = create_empty_s_expr();
    divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(second_case, create_s_expr_tp(divide_zero));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t 1 (/ 0))) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_number_tp(1));
    divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(first_case, create_s_expr_tp(divide_zero));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t (/ 0) 1)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(first_case, create_s_expr_tp(divide_zero));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (EVAL_ERROR_NOT_ID 1)) -> ???
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t EVAL_ERROR_NOT_ID)) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    s_expr_append(first_case, create_error_tp(EVAL_ERROR_NOT_ID));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond ((/ 0) 1)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = create_empty_s_expr();
    divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(first_case, create_s_expr_tp(divide_zero));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (cond (#t (/ 0))) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(cond_builtin));
    first_case = unit_list(create_atom_tp(TYPE_BOOL, true));
    divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(first_case, create_s_expr_tp(divide_zero));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    delete_environment_full(env);
    free(cond_builtin);
    free(add_sym);
    free(equals_sym);
    free(null_sym);
    free(else_sym);
    free(subtract_sym);
    free(divide_sym);
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
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* equals_sym = symbol_tp_from_name(env, "=");
    typed_ptr* subtract_sym = symbol_tp_from_name(env, "-");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    typed_ptr *x_sym, *y_sym, *z_sym;
    x_sym = install_symbol(env, strdup("x"), TYPE_UNDEF, (tp_value){.idx=0});
    y_sym = install_symbol(env, strdup("y"), TYPE_UNDEF, (tp_value){.idx=0});
    z_sym = install_symbol(env, strdup("z"), TYPE_UNDEF, (tp_value){.idx=0});
    bool pass = true;
    // (define) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(define_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define x) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define x 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define 1 2) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define x 1) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
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
    s_expr* add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(add_one_one));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
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
    s_expr* divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(cmd, create_s_expr_tp(divide_zero));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define <weird symbol number> 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_SYMBOL, 1000)); // kludge
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define (x)) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define (x) 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define () 1) -> EVAL_ERROR_BAD_SYNTAX
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(create_empty_s_expr()));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define (1) 2) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(create_number_tp(1))));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define (<weird symbol number>) 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr* fn_name_args = unit_list(create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define (x 1) 2) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(fn_name_args, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define (x) 1) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    typed_ptr* body = create_number_tp(1);
    s_expr_append(cmd, copy_typed_ptr(body));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    x_value = value_lookup_index(env, x_sym);
    Function_Node* x_fn = function_lookup_index(env, x_value);
    if (x_fn == NULL || \
        x_fn->arg_list != NULL || \
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
    add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(add_one_one));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    x_value = value_lookup_index(env, x_sym);
    x_fn = function_lookup_index(env, x_value);
    add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    body = create_s_expr_tp(add_one_one);
    if (x_fn == NULL || \
        x_fn->arg_list != NULL || \
        x_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(x_fn->body, body)) {
        pass = false;
    }
    free(x_value);
    delete_s_expr_recursive(add_one_one, true);
    free(body);
    // (define (x <weird symbol number>) 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(fn_name_args, create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    // (define (x y z) (+ y z)) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(fn_name_args, copy_typed_ptr(y_sym));
    s_expr_append(fn_name_args, copy_typed_ptr(z_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, copy_typed_ptr(y_sym));
    s_expr_append(add_one_one, copy_typed_ptr(z_sym));
    s_expr_append(cmd, create_s_expr_tp(add_one_one));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    x_value = value_lookup_index(env, x_sym);
    x_fn = function_lookup_index(env, x_value);
    add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, copy_typed_ptr(y_sym));
    s_expr_append(add_one_one, copy_typed_ptr(z_sym));
    body = create_s_expr_tp(add_one_one);
    if (x_fn == NULL || \
        x_fn->arg_list == NULL || \
        strcmp(x_fn->arg_list->name, "y") || \
        x_fn->arg_list->next == NULL || \
        strcmp(x_fn->arg_list->next->name, "z") || \
        x_fn->arg_list->next->next != NULL || \
        x_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(x_fn->body, body)) {
        pass = false;
    }
    free(x_value);
    delete_s_expr_recursive(add_one_one, true);
    free(body);
    // (define (x) (/ 0)) -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(define_builtin));
    fn_name_args = unit_list(copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_name_args));
    divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(cmd, create_s_expr_tp(divide_zero));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_define, cmd, env, expected);
    x_value = value_lookup_index(env, x_sym);
    x_fn = function_lookup_index(env, x_value);
    add_one_one = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(add_one_one, create_number_tp(0));
    body = create_s_expr_tp(add_one_one);
    if (x_fn == NULL || \
        x_fn->arg_list != NULL || \
        x_fn->closure_env == NULL || \
        !deep_match_typed_ptrs(x_fn->body, body)) {
        pass = false;
    }
    free(x_value);
    delete_s_expr_recursive(add_one_one, true);
    free(body);
    delete_environment_full(env);
    free(define_builtin);
    free(add_sym);
    free(equals_sym);
    free(null_sym);
    free(else_sym);
    free(subtract_sym);
    free(divide_sym);
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
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr *x_sym;
    x_sym = install_symbol(env, strdup("x"), TYPE_UNDEF, (tp_value){.idx=0});
    bool pass = true;
    // (set!) -> EVAL_ERROR_FEW_ARGS
    s_expr* cmd = unit_list(copy_typed_ptr(setvar_builtin));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! x) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! x 1 2) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! 1 2) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! (x) 1) -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! x 1) [with x undefined] -> EVAL_ERROR_UNDEF_SYM
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_UNDEF_SYM);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! <weird symbol number> 1) -> EVAL_ERROR_BAD_SYMBOL
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, create_atom_tp(TYPE_SYMBOL, 1000));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! x 1) [with x defined] -> <void> + side effect
    symbol_lookup_index(env, x_sym)->type = TYPE_BOOL;
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    typed_ptr* x_value = value_lookup_index(env, x_sym);
    if (x_value == NULL || \
        x_value->type != TYPE_NUM || \
        x_value->ptr.idx != 1) {
        pass = false;
    }
    free(x_value);
    // (set! x (+ 1 1)) [with x defined] -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr* add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(add_one_one));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    x_value = value_lookup_index(env, x_sym);
    if (x_value == NULL || \
        x_value->type != TYPE_NUM || \
        x_value->ptr.idx != 2) {
        pass = false;
    }
    free(x_value);
    // (set! x (list 1 2)) [with x defined] -> <void> + side effect
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr* list_one_two = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_one_two, create_number_tp(1));
    s_expr_append(list_one_two, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(list_one_two));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    x_value = value_lookup_index(env, x_sym);
    s_expr* result_list = unit_list(create_number_tp(1));
    s_expr_append(result_list, create_number_tp(2));
    if (x_value == NULL || \
        x_value->type != TYPE_SEXPR || \
        !match_s_exprs(x_value->ptr.se_ptr, result_list)) {
        pass = false;
    }
    free(x_value);
    delete_s_expr_recursive(result_list, true);
    // (set! x (/ 0)) [with x defined] -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr* divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(cmd, create_s_expr_tp(divide_zero));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    // (set! x EVAL_ERROR_NOT_ID) [with x defined] -> EVAL_ERROR_NOT_ID
    cmd = unit_list(copy_typed_ptr(setvar_builtin));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_error_tp(EVAL_ERROR_NOT_ID));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(eval_set_variable, cmd, env, expected);
    delete_environment_full(env);
    free(setvar_builtin);
    free(add_sym);
    free(divide_sym);
    free(list_sym);
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
    typed_ptr* setvar_builtin = builtin_tp_from_name(env, "set!");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr *x_sym;
    x_sym = install_symbol(env, strdup("x"), TYPE_UNDEF, (tp_value){.idx=0});
    bool pass = true;
    // (+ 1 1) -> 2
    s_expr* cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_ADD));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    typed_ptr* expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (- 1 1) -> 0
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_SUB));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(0);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (* 2 3) -> 6
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_MUL));
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    expected = create_number_tp(6);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (/ 10 2) -> 5
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_DIV));
    s_expr_append(cmd, create_number_tp(10));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_number_tp(5);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (= 1 1) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBEREQ));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (< 1 0) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERLT));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (> 1 0) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERGT));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (<= 1 0) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERLE));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (>= 1 0) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERGE));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (define x 1) -> <void> + side effect
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_DEFINE));
    s_expr_append(cmd, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
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
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
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
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (cons 1 null) -> '(1)
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_CONS));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, copy_typed_ptr(null_sym));
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (car (list 1 2)) -> 1
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_CAR));
    s_expr* list_one_two = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_one_two, create_number_tp(1));
    s_expr_append(list_one_two, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(list_one_two));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (cdr (list 1 2)) -> '(2)
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_CDR));
    list_one_two = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_one_two, create_number_tp(1));
    s_expr_append(list_one_two, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(list_one_two));
    expected = create_s_expr_tp(unit_list(create_number_tp(2)));
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (list 1 2) -> '(1 2)
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_LIST));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_s_expr_tp(unit_list(create_number_tp(1)));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(2));
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (and #t #f) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_AND));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (or #f #t) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_OR));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (not #t) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NOT));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (cond (#f 1) (else 2)) -> 2
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_COND));
    s_expr* first_case = unit_list(create_atom_tp(TYPE_BOOL, false));
    s_expr_append(first_case, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(first_case));
    s_expr* second_case = unit_list(copy_typed_ptr(else_sym));
    s_expr_append(second_case, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(second_case));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_cond, cmd, env, expected);
    // (list? 1) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_LISTPRED));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (pair? #t) -> #f
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_PAIRPRED));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, false);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (number? 1) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_NUMBERPRED));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (boolean? #f) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_BOOLPRED));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, false));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (void? <void>) -> #t
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_VOIDPRED));
    s_expr_append(cmd, create_atom_tp(TYPE_VOID, 0));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (lambda () 1) -> <procedure> + side effect
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_LAMBDA));
    s_expr_append(cmd, create_s_expr_tp(create_empty_s_expr()));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_atom_tp(TYPE_FUNCTION, 0);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    typed_ptr* body = create_number_tp(1);
    expected = create_atom_tp(TYPE_FUNCTION, 0);
    Function_Node* fn = function_lookup_index(env, expected);
    if (fn == NULL || \
        fn->arg_list != NULL || \
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
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    // (/ 0) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_DIV));
    s_expr_append(cmd, create_number_tp(0));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_builtin, cmd, env, expected);
    delete_environment_full(env);
    free(setvar_builtin);
    free(add_sym);
    free(divide_sym);
    free(list_sym);
    free(x_sym);
    free(null_sym);
    free(else_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_sexpr(test_env* te) {
    print_test_announce("eval_sexpr()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* define_builtin = builtin_tp_from_name(env, "define");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* multiply_sym = symbol_tp_from_name(env, "*");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* null_sym = symbol_tp_from_name(env, "null");
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr* lambda_sym = symbol_tp_from_name(env, "lambda");
    typed_ptr *x_sym, *x2_sym;
    x_sym = install_symbol(env, strdup("x"), TYPE_UNDEF, (tp_value){.idx=0});
    x2_sym = install_symbol(env, strdup("x2"), TYPE_UNDEF, (tp_value){.idx=0});
    bool pass = true;
    // () -> EVAL_ERROR_MISSING_PROCEDURE
    s_expr* cmd = create_empty_s_expr();
    s_expr* nested_cmd = create_empty_s_expr();
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    typed_ptr* expected = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    pass = pass && run_test_expect(eval_sexpr, cmd, env, expected);
    // ((/ 0) 1) -> EVAL_ERROR_DIV_ZERO
    cmd = create_empty_s_expr();
    nested_cmd = create_empty_s_expr();
    s_expr* divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(nested_cmd, create_s_expr_tp(divide_zero));
    s_expr_append(nested_cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_sexpr, cmd, env, expected);
    // ((+ 1 1) 1) -> EVAL_ERROR_CAR_NOT_CALLABLE
    cmd = create_empty_s_expr();
    nested_cmd = create_empty_s_expr();
    s_expr* add_one_one = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(add_one_one, create_number_tp(1));
    s_expr_append(nested_cmd, create_s_expr_tp(add_one_one));
    s_expr_append(nested_cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_error_tp(EVAL_ERROR_CAR_NOT_CALLABLE);
    pass = pass && run_test_expect(eval_sexpr, cmd, env, expected);
    // ('+ 1 1) -> 2
    cmd = create_empty_s_expr();
    nested_cmd = unit_list(copy_typed_ptr(add_sym));
    s_expr_append(nested_cmd, create_number_tp(1));
    s_expr_append(nested_cmd, create_number_tp(1));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(eval_sexpr, cmd, env, expected);
    // ('x2 10) [if x2 previously defined as doubling function] -> 20
    cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr* fn_sig = unit_list(copy_typed_ptr(x2_sym));
    s_expr_append(fn_sig, copy_typed_ptr(x_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_sig));
    s_expr* body = unit_list(copy_typed_ptr(multiply_sym));
    s_expr_append(body, copy_typed_ptr(x_sym));
    s_expr_append(body, create_number_tp(2));
    s_expr_append(cmd, create_s_expr_tp(body));
    expected = create_atom_tp(TYPE_VOID, 0);
    run_test_expect(eval_define, cmd, env, expected);
    cmd = create_empty_s_expr();
    nested_cmd = unit_list(copy_typed_ptr(x2_sym));
    s_expr_append(nested_cmd, create_number_tp(10));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_number_tp(20);
    pass = pass && run_test_expect(eval_sexpr, cmd, env, expected);
    // ((lambda (x) (* x 3)) 10) -> 30
    cmd = create_empty_s_expr();
    nested_cmd = create_empty_s_expr();
    s_expr* lambda = unit_list(copy_typed_ptr(lambda_sym));
    s_expr_append(lambda, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    body = unit_list(copy_typed_ptr(multiply_sym));
    s_expr_append(body, copy_typed_ptr(x_sym));
    s_expr_append(body, create_number_tp(3));
    s_expr_append(lambda, create_s_expr_tp(body));
    s_expr_append(nested_cmd, create_s_expr_tp(lambda));
    s_expr_append(nested_cmd, create_number_tp(10));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_number_tp(30);
    pass = pass && run_test_expect(eval_sexpr, cmd, env, expected);
    // ('/ 0) -> EVAL_ERROR_DIV_ZERO
    cmd = create_empty_s_expr();
    nested_cmd = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(nested_cmd, create_number_tp(0));
    s_expr_append(cmd, create_s_expr_tp(nested_cmd));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_sexpr, cmd, env, expected);
    delete_environment_full(env);
    free(define_builtin);
    free(add_sym);
    free(multiply_sym);
    free(divide_sym);
    free(list_sym);
    free(x_sym);
    free(null_sym);
    free(else_sym);
    free(lambda_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_eval_function(test_env* te) {
    print_test_announce("eval_function()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* define_builtin = builtin_tp_from_name(env, "define");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* multiply_sym = symbol_tp_from_name(env, "*");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* cons_sym = symbol_tp_from_name(env, "cons");
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr* lambda_sym = symbol_tp_from_name(env, "lambda");
    typed_ptr *x_sym, *y_sym, *my_fun;
    x_sym = install_symbol(env, strdup("x"), TYPE_UNDEF, (tp_value){.idx=0});
    y_sym = install_symbol(env, strdup("y"), TYPE_UNDEF, (tp_value){.idx=0});
    char mf[] = "my-fun";
    my_fun = install_symbol(env, strdup(mf), TYPE_UNDEF, (tp_value){.idx=0});
    bool pass = true;
    // given my-fun, a two-parameter function that doubles its first argument
    //   and prepends it to its second
    //   (i.e.: (define (my-fun x y) (cons (* x 2) y))     )
    s_expr* cmd = unit_list(copy_typed_ptr(define_builtin));
    s_expr* fn_sig = unit_list(copy_typed_ptr(my_fun));
    s_expr_append(fn_sig, copy_typed_ptr(x_sym));
    s_expr_append(fn_sig, copy_typed_ptr(y_sym));
    s_expr_append(cmd, create_s_expr_tp(fn_sig));
    s_expr* body = unit_list(copy_typed_ptr(cons_sym));
    s_expr* multiply_x_two = unit_list(copy_typed_ptr(multiply_sym));
    s_expr_append(multiply_x_two, copy_typed_ptr(x_sym));
    s_expr_append(multiply_x_two, create_number_tp(2));
    s_expr_append(body, create_s_expr_tp(multiply_x_two));
    s_expr_append(body, copy_typed_ptr(y_sym));
    s_expr_append(cmd, create_s_expr_tp(body));
    typed_ptr* expected = create_atom_tp(TYPE_VOID, 0);
    run_test_expect(eval_define, cmd, env, expected);
    // (<undefined function> 1) -> EVAL_ERROR_UNDEF_FUNCTION
    cmd = unit_list(create_atom_tp(TYPE_FUNCTION, 1000));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_UNDEF_FUNCTION);
    pass = pass && run_test_expect(eval_function, cmd, env, expected);
    // (my-fun (/ 0)) -> EVAL_ERROR_DIV_ZERO
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr* divide_zero = unit_list(copy_typed_ptr(divide_sym));
    s_expr_append(divide_zero, create_number_tp(0));
    s_expr_append(cmd, create_s_expr_tp(divide_zero));
    expected = create_error_tp(EVAL_ERROR_DIV_ZERO);
    pass = pass && run_test_expect(eval_function, cmd, env, expected);
    // (my-fun) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(value_lookup_index(env, my_fun));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_function, cmd, env, expected);
    // (my-fun 1) -> EVAL_ERROR_FEW_ARGS
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_FEW_ARGS);
    pass = pass && run_test_expect(eval_function, cmd, env, expected);
    // (my-fun 1 2 3) -> EVAL_ERROR_MANY_ARGS
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(2));
    s_expr_append(cmd, create_number_tp(3));
    expected = create_error_tp(EVAL_ERROR_MANY_ARGS);
    pass = pass && run_test_expect(eval_function, cmd, env, expected);
    // (my-fun 2 (list 8 16)) -> '(4 8 16)
    /*cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_number_tp(2));
    s_expr* list_eight_sixteen = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_eight_sixteen, create_number_tp(8));
    s_expr_append(list_eight_sixteen, create_number_tp(16));
    s_expr_append(cmd, create_s_expr_tp(list_eight_sixteen));
    expected = create_s_expr_tp(unit_list(create_number_tp(4)));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(8));
    s_expr_append(expected->ptr.se_ptr, create_number_tp(16));
    pass = pass && run_test_expect(eval_function, cmd, env, expected);
    // (my-fun #t (list 8 16)) -> EVAL_ERROR_NEED_NUM
    cmd = unit_list(value_lookup_index(env, my_fun));
    s_expr_append(cmd, create_atom_tp(TYPE_BOOL, true));
    list_eight_sixteen = unit_list(copy_typed_ptr(list_sym));
    s_expr_append(list_eight_sixteen, create_number_tp(8));
    s_expr_append(list_eight_sixteen, create_number_tp(16));
    s_expr_append(cmd, create_s_expr_tp(list_eight_sixteen));
    expected = create_error_tp(EVAL_ERROR_NEED_NUM);
    pass = pass && run_test_expect(eval_function, cmd, env, expected);*/
    delete_environment_full(env);
    free(define_builtin);
    free(add_sym);
    free(multiply_sym);
    free(divide_sym);
    free(list_sym);
    free(x_sym);
    free(cons_sym);
    free(else_sym);
    free(lambda_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_evaluate(test_env* te) {
    print_test_announce("eval_function()");
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    typed_ptr* define_builtin = builtin_tp_from_name(env, "define");
    typed_ptr* add_sym = symbol_tp_from_name(env, "+");
    typed_ptr* multiply_sym = symbol_tp_from_name(env, "*");
    typed_ptr* divide_sym = symbol_tp_from_name(env, "/");
    typed_ptr* list_sym = symbol_tp_from_name(env, "list");
    typed_ptr* cond_sym = symbol_tp_from_name(env, "cond");
    typed_ptr* else_sym = symbol_tp_from_name(env, "else");
    typed_ptr* lambda_sym = symbol_tp_from_name(env, "lambda");
    typed_ptr *x_sym;
    x_sym = install_symbol(env, strdup("x"), TYPE_NUM, (tp_value){.idx=1});
    bool pass = true;
    // eval[ NULL ] -> EVAL_ERROR_NULL_SEXPR
    s_expr* cmd = NULL;
    typed_ptr* expected = create_error_tp(EVAL_ERROR_NULL_SEXPR);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ () ] -> EVAL_ERROR_MISSING_PROCEDURE
    cmd = create_empty_s_expr();
    expected = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ <malformed s-expression> ] -> EVAL_ERROR_MALFORMED_SEXPR
    cmd = create_s_expr(create_number_tp(1), NULL);
    expected = create_error_tp(EVAL_ERROR_MALFORMED_SEXPR);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    cmd = create_s_expr(NULL, create_number_tp(1));
    expected = create_error_tp(EVAL_ERROR_MALFORMED_SEXPR);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ <undefined> ] -> EVAL_ERROR_UNDEF_SYM
    cmd = unit_list(create_atom_tp(TYPE_UNDEF, 0));
    expected = create_error_tp(EVAL_ERROR_UNDEF_SYM);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ EVAL_ERROR_NOT_ID ] -> EVAL_ERROR_NOT_ID
    cmd = unit_list(create_error_tp(EVAL_ERROR_NOT_ID));
    expected = create_error_tp(EVAL_ERROR_NOT_ID);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ <void> ] -> <void>
    cmd = unit_list(create_atom_tp(TYPE_VOID, 0));
    expected = create_atom_tp(TYPE_VOID, 0);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ 1 ] -> 1
    cmd = unit_list(create_number_tp(1));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ #t ] -> #t
    cmd = unit_list(create_atom_tp(TYPE_BOOL, true));
    expected = create_atom_tp(TYPE_BOOL, true);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ (+ 1 1) ] -> 2
    cmd = unit_list(create_atom_tp(TYPE_BUILTIN, BUILTIN_ADD));
    s_expr_append(cmd, create_number_tp(1));
    s_expr_append(cmd, create_number_tp(1));
    expected = create_number_tp(2);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ ((lambda (x) (* x 10)) 2) ] -> <procedure>
    cmd = create_empty_s_expr();
    s_expr* lambda = unit_list(copy_typed_ptr(lambda_sym));
    s_expr_append(lambda, create_s_expr_tp(unit_list(copy_typed_ptr(x_sym))));
    s_expr* multiply_x_ten = unit_list(copy_typed_ptr(multiply_sym));
    s_expr_append(multiply_x_ten, copy_typed_ptr(x_sym));
    s_expr_append(multiply_x_ten, create_number_tp(10));
    s_expr_append(lambda, create_s_expr_tp(multiply_x_ten));
    s_expr_append(cmd, create_s_expr_tp(lambda));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_atom_tp(TYPE_FUNCTION, 0);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ 'x ] (assuming x is defined to be 1) -> 1
    cmd = unit_list(copy_typed_ptr(x_sym));
    expected = create_number_tp(1);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ (<procedure from above> 2) ] -> 20
    cmd = unit_list(create_atom_tp(TYPE_FUNCTION, 0));
    s_expr_append(cmd, create_number_tp(2));
    expected = create_atom_tp(TYPE_NUM, 20);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    // eval[ <undefined type> ] -> EVAL_ERROR_UNDEF_TYPE
    cmd = unit_list(create_atom_tp(1000, 1));
    expected = create_error_tp(EVAL_ERROR_UNDEF_TYPE);
    pass = pass && run_test_expect(evaluate, cmd, env, expected);
    delete_environment_full(env);
    free(define_builtin);
    free(add_sym);
    free(multiply_sym);
    free(divide_sym);
    free(list_sym);
    free(x_sym);
    free(cond_sym);
    free(else_sym);
    free(lambda_sym);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

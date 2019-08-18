#include "unit_tests_environment.h"

#define TEST_NUM 42
static tp_value TEST_NUM_TP_VAL = {.idx=TEST_NUM};

void unit_tests_environment(test_env* te) {
    printf("# environment.c #\n");
    test_create_symbol_node(te);
    test_create_error_symbol_node(te);
    test_create_symbol_table(te);
    test_merge_symbol_tables(te);
    test_delete_symbol_node_list(te);
    test_create_function_node(te);
    test_create_function_table(te);
    test_create_environment(te);
    test_copy_environment(te);
    test_delete_environment_shared_full(te);
    test_install_symbol_regular_and_blind(te);
    test_install_function(te);
    test_symbol_lookup_name(te);
    test_symbol_lookup_index(te);
    test_builtin_lookup_index(te);
    test_value_lookup_index(te);
    test_function_lookup_index(te);
    return;
}

void test_create_symbol_node(test_env* te) {
    print_test_announce("create_symbol_node()");
    Symbol_Node* out = create_symbol_node(0, "test", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    bool pass = true;
    if (out == NULL || \
        out->symbol_idx != 0 || \
        strcmp(out->name, "test") || \
        out->type != TYPE_FIXNUM || \
        out->value.idx != TEST_NUM || \
        out->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_error_symbol_node(test_env* te) {
    print_test_announce("create_error_symbol_node()");
    Symbol_Node* out = create_error_symbol_node(EVAL_ERROR_EXIT);
    bool pass = true;
    if (out == NULL || \
        out->symbol_idx != 0 || \
        strcmp(out->name, "") || \
        out->type != TYPE_ERROR || \
        out->value.idx != EVAL_ERROR_EXIT || \
        out->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_symbol_table(test_env* te) {
    print_test_announce("create_symbol_table()");
    Symbol_Table* out = create_symbol_table(42);
    bool pass = true;
    if (out == NULL || \
        out->head != NULL || \
        out->length != 0 || \
        out->offset != 42) {
        pass = false;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_merge_symbol_tables(test_env* te) {
    print_test_announce("merge_symbol_tables()");
    Symbol_Table* first = create_symbol_table(0);
    Symbol_Table* second = create_symbol_table(10);
    bool pass = true;
    // both empty
    merge_symbol_tables(first, second);
    if (first == NULL || \
        second == NULL || \
        first->head != NULL || \
        first->length != 0 || \
        first->offset != 0 || \
        second->head != NULL) {
        pass = false;
    }
    // first empty
    Symbol_Node *first_node1, *first_node2, *second_node1, *second_node2;
    first_node1 = create_symbol_node(0, "x", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    first_node2 = create_symbol_node(1, "y", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    second_node1 = create_symbol_node(10, "z", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    second_node2 = create_symbol_node(11, "w", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    second->head = second_node1;
    second_node1->next = second_node2;
    second->length = 2;
    merge_symbol_tables(first, second);
    if (first == NULL || \
        second == NULL || \
        first->head != second_node1 || \
        first->head->next != second_node2 || \
        first->head->next->next != NULL || \
        first->length != 2 || \
        first->offset != 0 || \
        second->head != NULL || \
        second->length != 0) {
        pass = false;
    }
    second_node1->next = NULL;
    first->head = NULL;
    first->length = 0;
    // second empty
    first->head = first_node1;
    first_node1->next = first_node2;
    first->length = 2;
    merge_symbol_tables(first, second);
    if (first == NULL || \
        second == NULL || \
        first->head != first_node1 || \
        first->head->next != first_node2 || \
        first->head->next->next != NULL || \
        first->length != 2 || \
        first->offset != 0 || \
        second->head != NULL || \
        second->length != 0) {
        pass = false;
    }
    // neither empty
    second->head = second_node1;
    second_node1->next = second_node2;
    second->length = 2;
    merge_symbol_tables(first, second);
    if (first == NULL || \
        second == NULL || \
        first->head != first_node1 || \
        first->head->next != first_node2 || \
        first->head->next->next != second_node1 || \
        first->head->next->next->next != second_node2 || \
        first->head->next->next->next->next != NULL || \
        first->length != 4 || \
        first->offset != 0 || \
        second->head != NULL || \
        second->length != 0) {
        pass = false;
    }
    delete_symbol_node_list(first->head);
    free(first);
    free(second);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

// smoke tests + valgrind checks
void test_delete_symbol_node_list(test_env* te) {
    print_test_announce("delete_symbol_node_list()");
    Symbol_Node *first_node, *second_node, *third_node, *lone_node;
    first_node = create_symbol_node(0, "x", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    second_node = create_symbol_node(1, "y", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    third_node = create_symbol_node(10, "z", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    lone_node = create_symbol_node(11, "w", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    first_node->next = second_node;
    second_node->next = third_node;
    // NULL
    delete_symbol_node_list(NULL);
    // one node
    delete_symbol_node_list(lone_node);
    // multiple nodes
    delete_symbol_node_list(first_node);
    print_test_result(true);
    te->passed++;
    te->run++;
    return;
}

void test_create_function_node(test_env* te) {
    print_test_announce("create_function_node()");
    Symbol_Node* params = create_symbol_node(0, "x", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    params->next = create_symbol_node(1, "y", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    Function_Node* out = create_function_node(0, params, closure, body);
    bool pass = true;
    if (out == NULL || \
        out->function_idx != 0 || \
        out->param_list != params || \
        out->closure_env != closure || \
        out->body != body || \
        out->next != NULL) {
        pass = false;
    }
    delete_symbol_node_list(params);
    delete_environment_full(closure);
    free(body->ptr.se_ptr);
    free(body);
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_function_table(test_env* te) {
    print_test_announce("create_function_table()");
    Function_Table* out = create_function_table(42);
    bool pass = true;
    if (out == NULL || \
        out->head != NULL || \
        out->length != 0 || \
        out->offset != 42) {
        pass = false;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_environment(test_env* te) {
    print_test_announce("create_environment()");
    Environment* out = create_environment(2, 4);
    bool pass = true;
    if (out == NULL || \
        out->symbol_table == NULL || \
        out->symbol_table->head != NULL || \
        out->symbol_table->length != 0 || \
        out->symbol_table->offset != 2 || \
        out->function_table == NULL || \
        out->function_table->head != NULL || \
        out->function_table->length != 0 || \
        out->function_table->offset != 4) {
        pass = false;
    }
    delete_environment_full(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_copy_environment(test_env* te) {
    print_test_announce("copy_environment()");
    Environment* original = create_environment(0, 0);
    typed_ptr number_64 = {.type=TYPE_FIXNUM, .ptr={.idx=64}};
    typed_ptr number_128 = {.type=TYPE_FIXNUM, .ptr={.idx=128}};
    blind_install_symbol(original, "test_atom_1", &number_64);
    blind_install_symbol(original, "test_atom_2", &number_128);
    s_expr* se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 512), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 256), create_s_expr_tp(se));
    typed_ptr se_tp = {.type=TYPE_S_EXPR, .ptr={.se_ptr=se}};
    blind_install_symbol(original, "test_se_1", &se_tp);
    Symbol_Node* params = create_symbol_node(0, "x", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    params->next = create_symbol_node(1, "y", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(original, params, closure, body);
    free(out);
    Environment* copied = copy_environment(original);
    bool pass = true;
    if (copied == NULL || \
        copied->symbol_table == NULL || \
        copied->symbol_table == original->symbol_table || \
        copied->symbol_table->length != 3 || \
        copied->symbol_table->offset != 0 || \
        copied->function_table == NULL || \
        copied->function_table != original->function_table) {
        pass = false;
    }
    for (Symbol_Node* osn = original->symbol_table->head; \
         pass && osn != NULL; \
         osn = osn->next) {
        Symbol_Node* csn = symbol_lookup_name(copied, osn->name);
        if (csn == NULL || \
            osn == csn || \
            osn->symbol_idx != csn->symbol_idx || \
            osn->type != csn->type || \
            (osn->type == TYPE_S_EXPR && \
             osn->value.se_ptr == csn->value.se_ptr) || \
            (osn->type != TYPE_S_EXPR && \
             osn->value.idx != csn->value.idx)) {
            pass = false;
        }
    }
    free(closure->function_table);
    delete_environment_shared(copied);
    delete_environment_full(original);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

// smoke tests & valgrind checks
void test_delete_environment_shared_full(test_env* te) {
    print_test_announce("delete_environment_shared/full()");
    Environment* original = create_environment(0, 0);
    typed_ptr number_64 = {.type=TYPE_FIXNUM, .ptr={.idx=64}};
    typed_ptr number_128 = {.type=TYPE_FIXNUM, .ptr={.idx=128}};
    blind_install_symbol(original, "test_atom_1", &number_64);
    blind_install_symbol(original, "test_atom_2", &number_128);
    s_expr* se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 512), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_FIXNUM, 256), create_s_expr_tp(se));
    typed_ptr se_tp = {.type=TYPE_S_EXPR, .ptr={.se_ptr=se}};
    blind_install_symbol(original, "test_se_1", &se_tp);
    Symbol_Node* params = create_symbol_node(0, "x", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    params->next = create_symbol_node(1, "y", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(original, params, closure, body);
    free(out);
    Environment* copied = copy_environment(original);
    free(closure->function_table);
    delete_environment_shared(copied);
    delete_environment_full(original);
    print_test_result(true);
    te->passed++;
    te->run++;
    return;
}

void test_install_symbol_regular_and_blind(test_env* te) {
    print_test_announce("(blind_)install_symbol()");
    Environment* env = create_environment(0, 0);
    char name1[] = "test_sym_1";
    char name2[] = "test_sym_2";
    char name3[] = "test_sym_3";
    char name4[] = "test_sym_4";
    typed_ptr number = {.type=TYPE_FIXNUM, .ptr={.idx=TEST_NUM}};
    typed_ptr boolean = {.type=TYPE_BOOL, .ptr={.idx=TEST_NUM}};
    typed_ptr* out = install_symbol(env, name1, &number);
    bool pass = true;
    if (out == NULL || \
        out->type != TYPE_SYMBOL || \
        out->ptr.idx != symbol_lookup_name(env, name1)->symbol_idx) {
        pass = false;
    }
    free(out);
    out = install_symbol(env, name2, &boolean);
    if (out == NULL || \
        out->type != TYPE_SYMBOL || \
        out->ptr.idx != symbol_lookup_name(env, name2)->symbol_idx) {
        pass = false;
    }
    free(out);
    typed_ptr exit_error = {.type=TYPE_ERROR, .ptr={.idx=EVAL_ERROR_EXIT}};
    blind_install_symbol(env, name3, &exit_error);
    s_expr* se = create_empty_s_expr();
    typed_ptr se_tp = {.type=TYPE_S_EXPR, .ptr={.se_ptr=se}};
    blind_install_symbol(env, name4, &se_tp);
    if (env->symbol_table->length != 4) {
        pass = false;
    }
    if (symbol_lookup_name(env, name1) == NULL || \
        strcmp(symbol_lookup_name(env, name1)->name, name1) || \
        symbol_lookup_name(env, name1)->type != TYPE_FIXNUM || \
        symbol_lookup_name(env, name1)->value.idx != TEST_NUM) {
        pass = false;
    }
    if (symbol_lookup_name(env, name2) == NULL || \
        strcmp(symbol_lookup_name(env, name2)->name, name2) || \
        symbol_lookup_name(env, name2)->type != TYPE_BOOL || \
        symbol_lookup_name(env, name2)->value.idx != TEST_NUM) {
        pass = false;
    }
    if (symbol_lookup_name(env, name3) == NULL || \
        strcmp(symbol_lookup_name(env, name3)->name, name3) || \
        symbol_lookup_name(env, name3)->type != TYPE_ERROR || \
        symbol_lookup_name(env, name3)->value.idx != EVAL_ERROR_EXIT) {
        pass = false;
    }
    if (symbol_lookup_name(env, name4) == NULL || \
        strcmp(symbol_lookup_name(env, name4)->name, name4) || \
        symbol_lookup_name(env, name4)->type != TYPE_S_EXPR || \
        symbol_lookup_name(env, name4)->value.se_ptr != se) {
        pass = false;
    }
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_install_function(test_env* te) {
    print_test_announce("install_function()");
    Environment* env = create_environment(0, 0);
    Symbol_Node* params;
    params = create_symbol_node(0, "x", TYPE_FIXNUM, TEST_NUM_TP_VAL);
    params->next = create_symbol_node(1, "y", TYPE_BOOL, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(env, params, closure, body);
    bool pass = true;
    if (out == NULL || \
        out->type != TYPE_FUNCTION || \
        out->ptr.idx != env->function_table->head->function_idx || \
        function_lookup_index(env, out)->param_list != params || \
        function_lookup_index(env, out)->closure_env != closure || \
        function_lookup_index(env, out)->body != body) {
        pass = false;
    }
    free(out);
    free(closure->function_table);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_symbol_lookup_name(test_env* te) {
    print_test_announce("symbol_lookup_name()");
    Environment* env = create_environment(0, 0);
    char name1[] = "test_symbol_1";
    char name2[] = "test_symbol_2";
    char name3[] = "test_symbol_3";
    char absent_name[] = "test_absent_symbol";
    typed_ptr number = {.type=TYPE_FIXNUM, .ptr={.idx=TEST_NUM}};
    typed_ptr boolean = {.type=TYPE_BOOL, .ptr={.idx=TEST_NUM}};
    typed_ptr exit_error = {.type=TYPE_ERROR, .ptr={.idx=EVAL_ERROR_EXIT}};
    blind_install_symbol(env, name1, &number);
    blind_install_symbol(env, name2, &boolean);
    blind_install_symbol(env, name3, &exit_error);
    bool pass = true;
    if (symbol_lookup_name(env, name1) == NULL || \
        strcmp(symbol_lookup_name(env, name1)->name, name1) || \
        symbol_lookup_name(env, name1)->type != TYPE_FIXNUM || \
        symbol_lookup_name(env, name1)->value.idx != TEST_NUM || \
        symbol_lookup_name(env, name2) == NULL || \
        strcmp(symbol_lookup_name(env, name2)->name, name2) || \
        symbol_lookup_name(env, name2)->type != TYPE_BOOL || \
        symbol_lookup_name(env, name2)->value.idx != TEST_NUM || \
        symbol_lookup_name(env, name3) == NULL || \
        strcmp(symbol_lookup_name(env, name3)->name, name3) || \
        symbol_lookup_name(env, name3)->type != TYPE_ERROR || \
        symbol_lookup_name(env, name3)->value.idx != EVAL_ERROR_EXIT || \
        symbol_lookup_name(env, absent_name) != NULL) {
        pass = false;
    }
    if (symbol_lookup_name(env, NULL) != NULL) {
        pass = false;
    }
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_symbol_lookup_index(test_env* te) {
    print_test_announce("symbol_lookup_index()");
    Environment* env = create_environment(0, 0);
    char name1[] = "test_symbol_1";
    char name2[] = "test_symbol_2";
    char name3[] = "test_symbol_3";
    typed_ptr number = {.type=TYPE_FIXNUM, .ptr={.idx=TEST_NUM}};
    typed_ptr boolean = {.type=TYPE_BOOL, .ptr={.idx=TEST_NUM}};
    typed_ptr error = {.type=TYPE_ERROR, .ptr={.idx=TEST_NUM}};
    typed_ptr *symbol_1, *symbol_2, *symbol_3;
    symbol_1 = install_symbol(env, name1, &number);
    symbol_2 = install_symbol(env, name2, &boolean);
    symbol_3 = install_symbol(env, name3, &error);
    typed_ptr* absent_symbol = create_atom_tp(TYPE_SYMBOL, 1000);
    typed_ptr* not_a_symbol = create_atom_tp(TYPE_FIXNUM, 1000);
    bool pass = true;
    if (symbol_lookup_index(env, symbol_1) == NULL || \
        strcmp(symbol_lookup_index(env, symbol_1)->name, name1) || \
        symbol_lookup_index(env, symbol_1)->type != TYPE_FIXNUM || \
        symbol_lookup_index(env, symbol_1)->value.idx != TEST_NUM || \
        symbol_lookup_index(env, symbol_2) == NULL || \
        strcmp(symbol_lookup_index(env, symbol_2)->name, name2) || \
        symbol_lookup_index(env, symbol_2)->type != TYPE_BOOL || \
        symbol_lookup_index(env, symbol_2)->value.idx != TEST_NUM || \
        symbol_lookup_index(env, symbol_3) == NULL || \
        strcmp(symbol_lookup_index(env, symbol_3)->name, name3) || \
        symbol_lookup_index(env, symbol_3)->type != TYPE_ERROR || \
        symbol_lookup_index(env, symbol_3)->value.idx != TEST_NUM || \
        symbol_lookup_index(env, absent_symbol) != NULL) {
        pass = false;
    }
    if (symbol_lookup_index(env, NULL) != NULL) {
        pass = false;
    }
    if (symbol_lookup_index(env, not_a_symbol) != NULL) {
        pass = false;
    }
    free(symbol_1);
    free(symbol_2);
    free(symbol_3);
    free(absent_symbol);
    free(not_a_symbol);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_builtin_lookup_index(test_env* te) {
    print_test_announce("builtin_lookup_index()");
    Environment* env = create_environment(0, 0);
    long builtin_1 = 1;
    long builtin_2 = 2;
    long builtin_3 = 3;
    long absent_builtin = 4;
    typed_ptr* bi_ptr_1 = create_atom_tp(TYPE_BUILTIN, builtin_1);
    typed_ptr* bi_ptr_2 = create_atom_tp(TYPE_BUILTIN, builtin_2);
    typed_ptr* bi_ptr_3 = create_atom_tp(TYPE_BUILTIN, builtin_3);
    typed_ptr* bi_ptr_absent = create_atom_tp(TYPE_BUILTIN, absent_builtin);
    typed_ptr* not_a_builtin = create_atom_tp(TYPE_FIXNUM, 1000);
    blind_install_symbol(env, "bi_1", bi_ptr_1);
    blind_install_symbol(env, "bi_2", bi_ptr_2);
    blind_install_symbol(env, "bi_3", bi_ptr_3);
    bool pass = true;
    if (builtin_lookup_index(env, bi_ptr_1) == NULL || \
        strcmp(builtin_lookup_index(env, bi_ptr_1)->name, "bi_1") || \
        builtin_lookup_index(env, bi_ptr_1)->type != TYPE_BUILTIN || \
        builtin_lookup_index(env, bi_ptr_1)->value.idx != builtin_1 || \
        builtin_lookup_index(env, bi_ptr_2) == NULL || \
        strcmp(builtin_lookup_index(env, bi_ptr_2)->name, "bi_2") || \
        builtin_lookup_index(env, bi_ptr_2)->type != TYPE_BUILTIN || \
        builtin_lookup_index(env, bi_ptr_2)->value.idx != builtin_2 || \
        builtin_lookup_index(env, bi_ptr_3) == NULL || \
        strcmp(builtin_lookup_index(env, bi_ptr_3)->name, "bi_3") || \
        builtin_lookup_index(env, bi_ptr_3)->type != TYPE_BUILTIN || \
        builtin_lookup_index(env, bi_ptr_3)->value.idx != builtin_3 || \
        builtin_lookup_index(env, bi_ptr_absent) != NULL) {
        pass = false;
    }
    if (builtin_lookup_index(env, NULL) != NULL) {
        pass = false;
    }
    if (builtin_lookup_index(env, not_a_builtin) != NULL) {
        pass = false;
    }
    free(bi_ptr_1);
    free(bi_ptr_2);
    free(bi_ptr_3);
    free(bi_ptr_absent);
    free(not_a_builtin);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_value_lookup_index(test_env* te) {
    print_test_announce("value_lookup_index()");
    Environment* env = create_environment(0, 0);
    char name_num[] = "test_symbol_num";
    char name_bool[] = "test_symbol_bool";
    char name_se[] = "test_symbol_se";
    char name_undef[] = "test_symbol_undef";
    typed_ptr number = {.type=TYPE_FIXNUM, .ptr={.idx=TEST_NUM}};
    typed_ptr boolean = {.type=TYPE_BOOL, .ptr={.idx=TEST_NUM}};
    typed_ptr *symbol_num, *symbol_bool, *symbol_undef, *symbol_se;
    symbol_num = install_symbol(env, name_num, &number);
    symbol_bool = install_symbol(env, name_bool, &boolean);
    s_expr* se = create_empty_s_expr();
    typed_ptr se_tp = {.type=TYPE_S_EXPR, .ptr={.se_ptr=se}};
    symbol_se = install_symbol(env, name_se, &se_tp);
    typed_ptr undef = {.type=TYPE_UNDEF, .ptr={.idx=TEST_NUM}};
    symbol_undef = install_symbol(env, name_undef, &undef);
    typed_ptr* absent_symbol = create_atom_tp(TYPE_SYMBOL, 1000);
    typed_ptr* not_a_symbol = create_atom_tp(TYPE_FIXNUM, 1000);
    bool pass = true;
    typed_ptr* out = value_lookup_index(env, symbol_num);
    if (out == NULL || \
        out->type != TYPE_FIXNUM || \
        out->ptr.idx != TEST_NUM) {
        pass = false;
    }
    free(out);
    out = value_lookup_index(env, symbol_bool);
    if (out == NULL || \
        out->type != TYPE_BOOL || \
        out->ptr.idx != TEST_NUM) {
        pass = false;
    }
    free(out);
    out = value_lookup_index(env, symbol_se);
    if (out == NULL || \
        out->type != TYPE_S_EXPR || \
        !is_empty_list(out->ptr.se_ptr) || \
        out->ptr.se_ptr == se) {
        pass = false;
    }
    free(out->ptr.se_ptr);
    free(out);
    out = value_lookup_index(env, symbol_undef);
    if (out == NULL || \
        out->type != TYPE_ERROR || \
        out->ptr.idx != EVAL_ERROR_UNDEF_SYM) {
        pass = false;
    }
    free(out);
    out = value_lookup_index(env, absent_symbol);
    if (out != NULL) {
        pass = false;
    }
    free(out);
    out = value_lookup_index(env, not_a_symbol);
    if (out != NULL) {
        pass = false;
    }
    free(out);
    free(symbol_num);
    free(symbol_bool);
    free(symbol_se);
    free(symbol_undef);
    free(absent_symbol);
    free(not_a_symbol);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_function_lookup_index(test_env* te) {
    print_test_announce("function_lookup_index()");
    Environment* env = create_environment(0, 0);
    Symbol_Node* params;
    params = create_symbol_node(0, "x", TYPE_FIXNUM, (tp_value){.idx=TEST_NUM});
    params->next = create_symbol_node(1, "y", TYPE_BOOL, (tp_value){.idx=TEST_NUM});
    //Environment* closure = create_environment(0, 0);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(env, params, closure, body);
    bool pass = true;
    if (function_lookup_index(env, out) == NULL || \
        function_lookup_index(env, out)->function_idx != out->ptr.idx || \
        function_lookup_index(env, out)->param_list != params || \
        function_lookup_index(env, out)->closure_env != closure || \
        function_lookup_index(env, out)->body != body) {
        pass = false;
    }
    if (function_lookup_index(env, NULL) != NULL) {
        pass = false;
    }
    typed_ptr* not_a_function = create_atom_tp(TYPE_FIXNUM, 1000);
    if (function_lookup_index(env, not_a_function) != NULL) {
        pass = false;
    }
    typed_ptr* absent_function = create_atom_tp(TYPE_FUNCTION, 1000);
    if (function_lookup_index(env, absent_function) != NULL) {
        pass = false;
    }
    free(out);
    free(not_a_function);
    free(absent_function);
    free(closure->function_table);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

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
    test_symbol_lookup_string(te);
    test_symbol_lookup_index(te);
    test_builtin_lookup_index(te);
    test_value_lookup_index(te);
    test_function_lookup_index(te);
    return;
}

void test_create_symbol_node(test_env* te) {
    print_test_announce("create_symbol_node()");
    Symbol_Node* out = create_symbol_node(0, "test", TYPE_NUM, TEST_NUM_TP_VAL);
    bool pass = 1;
    if (out == NULL || \
        out->symbol_idx != 0 || \
        strcmp(out->name, "test") || \
        out->type != TYPE_NUM || \
        out->value.idx != TEST_NUM || \
        out->next != NULL) {
        pass = 0;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_error_symbol_node(test_env* te) {
    print_test_announce("create_error_symbol_node()");
    Symbol_Node* out = create_error_symbol_node(EVAL_ERROR_EXIT);
    bool pass = 1;
    if (out == NULL || \
        out->symbol_idx != 0 || \
        out->name != NULL || \
        out->type != TYPE_ERROR || \
        out->value.idx != EVAL_ERROR_EXIT || \
        out->next != NULL) {
        pass = 0;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_create_symbol_table(test_env* te) {
    print_test_announce("create_symbol_table()");
    Symbol_Table* out = create_symbol_table(42);
    bool pass = 1;
    if (out == NULL || \
        out->head != NULL || \
        out->length != 0 || \
        out->symbol_number_offset != 42) {
        pass = 0;
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
    bool pass = 1;
    // both empty
    merge_symbol_tables(first, second);
    if (first == NULL || \
        second == NULL || \
        first->head != NULL || \
        first->length != 0 || \
        first->symbol_number_offset != 0 || \
        second->head != NULL) {
        pass = 0;
    }
    // first empty
    Symbol_Node *first_node1, *first_node2, *second_node1, *second_node2;
    first_node1 = create_symbol_node(0, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    first_node2 = create_symbol_node(1, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    second_node1 = create_symbol_node(10, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    second_node2 = create_symbol_node(11, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
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
        first->symbol_number_offset != 0 || \
        second->head != NULL || \
        second->length != 0) {
        pass = 0;
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
        first->symbol_number_offset != 0 || \
        second->head != NULL || \
        second->length != 0) {
        pass = 0;
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
        first->symbol_number_offset != 0 || \
        second->head != NULL || \
        second->length != 0) {
        pass = 0;
    }
    free(first_node1);
    free(first_node2);
    free(second_node1);
    free(second_node2);
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
    first_node = create_symbol_node(0, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    second_node = create_symbol_node(1, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    third_node = create_symbol_node(10, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    lone_node = create_symbol_node(11, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
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
    Symbol_Node* args = create_symbol_node(0, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    args->next = create_symbol_node(1, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    Function_Node* out = create_function_node(0, args, closure, body);
    bool pass = 1;
    if (out == NULL || \
        out->function_idx != 0 || \
        out->arg_list != args || \
        out->closure_env != closure || \
        out->body != body || \
        out->next != NULL) {
        pass = 0;
    }
    delete_symbol_node_list(args);
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
    bool pass = 1;
    if (out == NULL || \
        out->head != NULL || \
        out->length != 0 || \
        out->offset != 42) {
        pass = 0;
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
    bool pass = 1;
    if (out == NULL || \
        out->symbol_table == NULL || \
        out->symbol_table->head != NULL || \
        out->symbol_table->length != 0 || \
        out->symbol_table->symbol_number_offset != 2 || \
        out->function_table == NULL || \
        out->function_table->head != NULL || \
        out->function_table->length != 0 || \
        out->function_table->offset != 4) {
        pass = 0;
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
    blind_install_symbol_atom(original, strdup("test_atom_1"), TYPE_NUM, 64);
    blind_install_symbol_atom(original, strdup("test_atom_2"), TYPE_NUM, 128);
    s_expr* se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_NUM, 512), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_NUM, 256), create_s_expr_tp(se));
    blind_install_symbol_sexpr(original, strdup("test_se_1"), TYPE_SEXPR, se);
    Symbol_Node* args = create_symbol_node(0, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    args->next = create_symbol_node(1, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(original, args, closure, body);
    free(out);
    Environment* copied = copy_environment(original);
    bool pass = 1;
    if (copied == NULL || \
        copied->symbol_table == NULL || \
        copied->symbol_table == original->symbol_table || \
        copied->symbol_table->length != 3 || \
        copied->symbol_table->symbol_number_offset != 0 || \
        copied->function_table == NULL || \
        copied->function_table != original->function_table) {
        pass = 0;
    }
    for (Symbol_Node* osn = original->symbol_table->head; \
         pass && osn != NULL; \
         osn = osn->next) {
        Symbol_Node* csn = symbol_lookup_string(copied, osn->name);
        if (csn == NULL || \
            osn == csn || \
            osn->symbol_idx != csn->symbol_idx || \
            osn->type != csn->type || \
            (osn->type == TYPE_SEXPR && \
             osn->value.se_ptr == csn->value.se_ptr) || \
            (osn->type != TYPE_SEXPR && \
             osn->value.idx != csn->value.idx)) {
            pass = 0;
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
    blind_install_symbol_atom(original, strdup("test_atom_1"), TYPE_NUM, 64);
    blind_install_symbol_atom(original, strdup("test_atom_2"), TYPE_NUM, 128);
    s_expr* se = create_empty_s_expr();
    se = create_s_expr(create_atom_tp(TYPE_NUM, 512), create_s_expr_tp(se));
    se = create_s_expr(create_atom_tp(TYPE_NUM, 256), create_s_expr_tp(se));
    blind_install_symbol_sexpr(original, strdup("test_se_1"), TYPE_SEXPR, se);
    Symbol_Node* args = create_symbol_node(0, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    args->next = create_symbol_node(1, NULL, TYPE_NUM, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(original, args, closure, body);
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
    typed_ptr* out;
    out = install_symbol(env, strdup(name1), TYPE_NUM, TEST_NUM_TP_VAL);
    bool pass = 1;
    if (out == NULL || \
        out->type != TYPE_SYMBOL || \
        out->ptr.idx != symbol_lookup_string(env, name1)->symbol_idx) {
        pass = 0;
    }
    free(out);
    out = install_symbol(env, strdup(name2), TYPE_BOOL, TEST_NUM_TP_VAL);
    if (out == NULL || \
        out->type != TYPE_SYMBOL || \
        out->ptr.idx != symbol_lookup_string(env, name2)->symbol_idx) {
        pass = 0;
    }
    free(out);
    blind_install_symbol_atom(env, strdup(name3), TYPE_ERROR, EVAL_ERROR_EXIT);
    s_expr* se = create_empty_s_expr();
    blind_install_symbol_sexpr(env, strdup(name4), TYPE_SEXPR, se);
    if (env->symbol_table->length != 4) {
        pass = 0;
    }
    if (symbol_lookup_string(env, name1) == NULL || \
        strcmp(symbol_lookup_string(env, name1)->name, name1) || \
        symbol_lookup_string(env, name1)->type != TYPE_NUM || \
        symbol_lookup_string(env, name1)->value.idx != TEST_NUM) {
        pass = 0;
    }
    if (symbol_lookup_string(env, name2) == NULL || \
        strcmp(symbol_lookup_string(env, name2)->name, name2) || \
        symbol_lookup_string(env, name2)->type != TYPE_BOOL || \
        symbol_lookup_string(env, name2)->value.idx != TEST_NUM) {
        pass = 0;
    }
    if (symbol_lookup_string(env, name3) == NULL || \
        strcmp(symbol_lookup_string(env, name3)->name, name3) || \
        symbol_lookup_string(env, name3)->type != TYPE_ERROR || \
        symbol_lookup_string(env, name3)->value.idx != EVAL_ERROR_EXIT) {
        pass = 0;
    }
    if (symbol_lookup_string(env, name4) == NULL || \
        strcmp(symbol_lookup_string(env, name4)->name, name4) || \
        symbol_lookup_string(env, name4)->type != TYPE_SEXPR || \
        symbol_lookup_string(env, name4)->value.se_ptr != se) {
        pass = 0;
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
    char x[] = "x";
    char y[] = "y";
    Symbol_Node* args;
    args = create_symbol_node(0, strdup(x), TYPE_NUM, TEST_NUM_TP_VAL);
    args->next = create_symbol_node(1, strdup(y), TYPE_BOOL, TEST_NUM_TP_VAL);
    Environment* closure = create_environment(0, 0);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(env, args, closure, body);
    bool pass = 1;
    if (out == NULL || \
        out->type != TYPE_FUNCTION || \
        out->ptr.idx != env->function_table->head->function_idx || \
        function_lookup_index(env, out)->arg_list != args || \
        function_lookup_index(env, out)->closure_env != closure || \
        function_lookup_index(env, out)->body != body) {
        pass = 0;
    }
    free(out);
    free(closure->function_table);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_symbol_lookup_string(test_env* te) {
    print_test_announce("symbol_lookup_string()");
    Environment* env = create_environment(0, 0);
    char name1[] = "test_symbol_1";
    char name2[] = "test_symbol_2";
    char name3[] = "test_symbol_3";
    char absent_name[] = "test_absent_symbol";
    blind_install_symbol_atom(env, strdup(name1), TYPE_NUM, TEST_NUM);
    blind_install_symbol_atom(env, strdup(name2), TYPE_BOOL, TEST_NUM);
    blind_install_symbol_atom(env, strdup(name3), TYPE_ERROR, EVAL_ERROR_EXIT);
    bool pass = 1;
    if (symbol_lookup_string(env, name1) == NULL || \
        strcmp(symbol_lookup_string(env, name1)->name, name1) || \
        symbol_lookup_string(env, name1)->type != TYPE_NUM || \
        symbol_lookup_string(env, name1)->value.idx != TEST_NUM || \
        symbol_lookup_string(env, name2) == NULL || \
        strcmp(symbol_lookup_string(env, name2)->name, name2) || \
        symbol_lookup_string(env, name2)->type != TYPE_BOOL || \
        symbol_lookup_string(env, name2)->value.idx != TEST_NUM || \
        symbol_lookup_string(env, name3) == NULL || \
        strcmp(symbol_lookup_string(env, name3)->name, name3) || \
        symbol_lookup_string(env, name3)->type != TYPE_ERROR || \
        symbol_lookup_string(env, name3)->value.idx != EVAL_ERROR_EXIT || \
        symbol_lookup_string(env, absent_name) != NULL) {
        pass = 0;
    }
    if (symbol_lookup_string(env, NULL) != NULL) {
        pass = 0;
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
    typed_ptr *symbol_1, *symbol_2, *symbol_3;
    symbol_1 = install_symbol(env, strdup(name1), TYPE_NUM, TEST_NUM_TP_VAL);
    symbol_2 = install_symbol(env, strdup(name2), TYPE_BOOL, TEST_NUM_TP_VAL);
    symbol_3 = install_symbol(env, strdup(name3), TYPE_ERROR, TEST_NUM_TP_VAL);
    typed_ptr* absent_symbol = create_atom_tp(TYPE_SYMBOL, 1000);
    typed_ptr* not_a_symbol = create_atom_tp(TYPE_NUM, 1000);
    bool pass = 1;
    if (symbol_lookup_index(env, symbol_1) == NULL || \
        strcmp(symbol_lookup_index(env, symbol_1)->name, name1) || \
        symbol_lookup_index(env, symbol_1)->type != TYPE_NUM || \
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
        pass = 0;
    }
    if (symbol_lookup_index(env, NULL) != NULL) {
        pass = 0;
    }
    if (symbol_lookup_index(env, not_a_symbol) != NULL) {
        pass = 0;
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
    typed_ptr* not_a_builtin = create_atom_tp(TYPE_NUM, 1000);
    blind_install_symbol_atom(env, strdup("bi_1"), TYPE_BUILTIN, builtin_1);
    blind_install_symbol_atom(env, strdup("bi_2"), TYPE_BUILTIN, builtin_2);
    blind_install_symbol_atom(env, strdup("bi_3"), TYPE_BUILTIN, builtin_3);
    bool pass = 1;
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
        pass = 0;
    }
    if (builtin_lookup_index(env, NULL) != NULL) {
        pass = 0;
    }
    if (builtin_lookup_index(env, not_a_builtin) != NULL) {
        pass = 0;
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
    char* name_num = strdup("test_symbol_num");
    char* name_bool = strdup("test_symbol_bool");
    char* name_se = strdup("test_symbol_se");
    char* name_undef = strdup("test_symbol_undef");
    typed_ptr *symbol_num, *symbol_bool, *symbol_undef, *symbol_se;
    symbol_num = install_symbol(env, name_num, TYPE_NUM, TEST_NUM_TP_VAL);
    symbol_bool = install_symbol(env, name_bool, TYPE_BOOL, TEST_NUM_TP_VAL);
    s_expr* se = create_empty_s_expr();
    tp_value tpv_se = {.se_ptr=se};
    symbol_se = install_symbol(env, name_se, TYPE_SEXPR, tpv_se);
    symbol_undef = install_symbol(env, name_undef, TYPE_UNDEF, TEST_NUM_TP_VAL);
    typed_ptr* absent_symbol = create_atom_tp(TYPE_SYMBOL, 1000);
    typed_ptr* not_a_symbol = create_atom_tp(TYPE_NUM, 1000);
    bool pass = 1;
    typed_ptr* out = value_lookup_index(env, symbol_num);
    if (out == NULL || \
        out->type != TYPE_NUM || \
        out->ptr.idx != TEST_NUM) {
        pass = 0;
    }
    free(out);
    out = value_lookup_index(env, symbol_bool);
    if (out == NULL || \
        out->type != TYPE_BOOL || \
        out->ptr.idx != TEST_NUM) {
        pass = 0;
    }
    free(out);
    out = value_lookup_index(env, symbol_se);
    if (out == NULL || \
        out->type != TYPE_SEXPR || \
        !is_empty_list(out->ptr.se_ptr) || \
        out->ptr.se_ptr == se) {
        pass = 0;
    }
    free(out->ptr.se_ptr);
    free(out);
    out = value_lookup_index(env, symbol_undef);
    if (out == NULL || \
        out->type != TYPE_ERROR || \
        out->ptr.idx != EVAL_ERROR_UNDEF_SYM) {
        pass = 0;
    }
    free(out);
    out = value_lookup_index(env, absent_symbol);
    if (out != NULL) {
        pass = 0;
    }
    free(out);
    out = value_lookup_index(env, not_a_symbol);
    if (out != NULL) {
        pass = 0;
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
    char* x = "x-param";
    char* y = "y-param";
    Symbol_Node* args;
    args = create_symbol_node(0, strdup(x), TYPE_NUM, (tp_value){.idx=TEST_NUM});
    args->next = create_symbol_node(1, strdup(y), TYPE_BOOL, (tp_value){.idx=TEST_NUM});
    //Environment* closure = create_environment(0, 0);
    Environment* closure = copy_environment(env);
    typed_ptr* body = create_s_expr_tp(create_empty_s_expr());
    typed_ptr* out = install_function(env, args, closure, body);
    bool pass = 1;
    if (function_lookup_index(env, out) == NULL || \
        function_lookup_index(env, out)->function_idx != out->ptr.idx || \
        function_lookup_index(env, out)->arg_list != args || \
        function_lookup_index(env, out)->closure_env != closure || \
        function_lookup_index(env, out)->body != body) {
        pass = 0;
    }
    if (function_lookup_index(env, NULL) != NULL) {
        pass = 0;
    }
    typed_ptr* not_a_function = create_atom_tp(TYPE_NUM, 1000);
    if (function_lookup_index(env, not_a_function) != NULL) {
        pass = 0;
    }
    typed_ptr* absent_function = create_atom_tp(TYPE_FUNCTION, 1000);
    if (function_lookup_index(env, absent_function) != NULL) {
        pass = 0;
    }
    free(out);
    free(not_a_function);
    free(absent_function);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

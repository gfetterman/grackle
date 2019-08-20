#include "unit_tests_parse.h"

void unit_tests_parse(test_env* te) {
    printf("# parse.c #\n");
    test_create_s_expr_stack(te);
    test_s_expr_stack_push_pop(te);
    test_init_new_s_expr(te);
    test_extend_s_expr(te);
    test_terminate_s_expr(te);
    test_register_symbol(te);
    test_substring(te);
    test_string_is_number(te);
    test_parse(te);
    return;
}

void test_create_s_expr_stack(test_env* te) {
    print_test_announce("create_s_expr_stack()");
    s_expr* se = create_empty_s_expr();
    s_expr_stack* stack = create_s_expr_stack(se);
    bool pass = true;
    if (stack == NULL || \
        stack->se != se || \
        stack->next != NULL) {
        pass = false;
    }
    free(se);
    free(stack);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_s_expr_stack_push_pop(test_env* te) {
    print_test_announce("s_expr_stack_push/pop()");
    s_expr_stack* stack = NULL;
    s_expr* se_1 = create_empty_s_expr();
    s_expr* se_2 = create_empty_s_expr();
    s_expr_stack_push(&stack, se_1);
    bool pass = true;
    if (stack == NULL || \
        stack->se != se_1 || \
        stack->next != NULL) {
        pass = false;
    }
    s_expr_stack_push(&stack, se_2);
    if (stack == NULL || \
        stack->se != se_2 || \
        stack->next == NULL || \
        stack->next->se != se_1 || \
        stack->next->next != NULL) {
        pass = false;
    }
    s_expr_stack_pop(&stack);
    if (stack == NULL || \
        stack->se != se_1 || \
        stack->next != NULL) {
        pass = false;
    }
    s_expr_stack_pop(&stack);
    if (stack != NULL) {
        pass = false;
    }
    free(se_1);
    free(se_2);
    // freeing the stack nodes was taken care of by s_expr_stack_pop()
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_init_new_s_expr(test_env* te) {
    print_test_announce("init_new_s_expr()");
    s_expr* se_1 = create_empty_s_expr();
    s_expr_stack* stack = create_s_expr_stack(se_1);
    init_new_s_expr(&stack);
    bool pass = true;
    if (stack == NULL || \
        !is_empty_list(stack->se) || \
        stack->next == NULL || \
        stack->next->se != se_1 || \
        stack->next->next != NULL || \
        stack->next->se->car == NULL || \
        stack->next->se->car->ptr.se_ptr != stack->se || \
        stack->next->se->cdr != NULL) {
        pass = false;
    }
    free(stack->se);
    free(se_1->car);
    free(se_1);
    free(stack->next);
    free(stack);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_extend_s_expr(test_env* te) {
    print_test_announce("extend_s_expr()");
    s_expr* se_1 = create_empty_s_expr();
    s_expr_stack* stack = create_s_expr_stack(se_1);
    extend_s_expr(&stack);
    bool pass = true;
    if (stack == NULL || \
        !is_empty_list(stack->se) || \
        stack->next == NULL || \
        stack->next->se != se_1 || \
        stack->next->next != NULL || \
        stack->next->se->car != NULL || \
        stack->next->se->cdr == NULL || \
        stack->next->se->cdr->ptr.se_ptr != stack->se) {
        pass = false;
    }
    free(stack->se);
    free(se_1->cdr);
    free(se_1);
    free(stack->next);
    free(stack);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_terminate_s_expr(test_env* te) {
    print_test_announce("terminate_s_expr()");
    s_expr_stack* stack = NULL;
    interpreter_error err = PARSE_ERROR_NONE;
    // passing in an empty stack -> parse error
    Parse_State out = terminate_s_expr(&stack, &err);
    bool pass = true;
    if (out != PARSE_ERROR || \
        err != PARSE_ERROR_UNBAL_PAREN) {
        pass = false;
    }
    err = PARSE_ERROR_NONE;
    // passing in a stack with a pair on top -> parse error
    s_expr* se = create_s_expr(NULL, create_atom_tp(TYPE_FIXNUM, 1000));
    s_expr_stack_push(&stack, se);
    out = terminate_s_expr(&stack, &err);
    if (out != PARSE_ERROR || \
        err != PARSE_ERROR_UNBAL_PAREN) {
        pass = false;
    }
    free(se->cdr);
    free(se);
    free(stack);
    stack = NULL;
    err = PARSE_ERROR_NONE;
    // passing in stack of one empty list -> parse is done
    se = create_empty_s_expr();
    s_expr_stack_push(&stack, se);
    out = terminate_s_expr(&stack, &err);
    if (stack != NULL || \
        out != PARSE_FINISH || \
        err != PARSE_ERROR_NONE || \
        !is_empty_list(se)) {
        pass = false;
    }
    free(se);
    // passing in stack of one partial s-expr -> finish s-expr, parse is done
    typed_ptr* test_atom = create_atom_tp(TYPE_FIXNUM, 1000);
    se = create_s_expr(test_atom, NULL);
    s_expr_stack_push(&stack, se);
    out = terminate_s_expr(&stack, &err);
    if (stack != NULL || \
        out != PARSE_FINISH || \
        err != PARSE_ERROR_NONE || \
        !match_typed_ptrs(se->car, test_atom) || \
        se->cdr == NULL || \
        !is_empty_list(se->cdr->ptr.se_ptr)) {
        pass = false;
    }
    delete_s_expr_recursive(se, true);
    // passing in stack of a nested s-expr -> return to spine, parse can proceed
    test_atom = create_atom_tp(TYPE_FIXNUM, 1000);
    se = create_empty_s_expr();
    s_expr_stack_push(&stack, se);
    init_new_s_expr(&stack);
    stack->se->car = test_atom;
    out = terminate_s_expr(&stack, &err);
    if (stack == NULL || \
        out != PARSE_READY || \
        err != PARSE_ERROR_NONE || \
        stack->next != NULL || \
        stack->se != se || \
        se->car == NULL || \
        se->car->ptr.se_ptr->car != test_atom || \
        se->cdr != NULL) {
        pass = false;
    }
    s_expr_stack_pop(&stack);
    se->cdr = create_s_expr_tp(create_empty_s_expr());
    delete_s_expr_recursive(se, true);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_register_symbol(test_env* te) {
    print_test_announce("register_symbol()");
    Environment* env = create_environment(0, 0);
    Environment* temp_env = create_environment(10, 0);
    s_expr_stack* stack = NULL;
    s_expr_stack_push(&stack, create_empty_s_expr());
    bool pass = true;
    interpreter_error out;
    // passing a number too small (negative)
    char symbol_num_low[100];
    snprintf(symbol_num_low, 100, "%ld", LONG_MIN);
    symbol_num_low[strlen(symbol_num_low) + 1] = '\0';
    symbol_num_low[strlen(symbol_num_low)] = '0';
    out = register_symbol(&stack, env, temp_env, symbol_num_low);
    if (out != PARSE_ERROR_INT_TOO_LOW || \
        stack->se == NULL || \
        stack->se->car != NULL) {
        pass = false;
    }
    // passing a number too large
    char symbol_num_high[100];
    snprintf(symbol_num_high, 100, "%ld", LONG_MAX);
    symbol_num_high[strlen(symbol_num_high) + 1] = '\0';
    symbol_num_high[strlen(symbol_num_high)] = '0';
    out = register_symbol(&stack, env, temp_env, symbol_num_high);
    if (out != PARSE_ERROR_INT_TOO_HIGH || \
        stack->se == NULL || \
        stack->se->car != NULL) {
        pass = false;
    }
    // passing a valid number
    char symbol_num[] = "1000";
    out = register_symbol(&stack, env, temp_env, symbol_num);
    typed_ptr* expected_tp = create_number_tp(1000);
    if (out != PARSE_ERROR_NONE || \
        stack->se == NULL || \
        stack->se->car == NULL || \
        !match_typed_ptrs(stack->se->car, expected_tp) || \
        symbol_lookup_name(env, symbol_num) != NULL || \
        symbol_lookup_name(temp_env, symbol_num) != NULL) {
        pass = false;
    }
    free(stack->se->car);
    stack->se->car = NULL;
    // passing a symbol already in env
    char symbol_env[] = "in-env";
    typed_ptr thousand = {.type=TYPE_FIXNUM, .ptr={.idx=1000}};
    typed_ptr* symbol_env_tp = install_symbol(env, symbol_env, &thousand);
    out = register_symbol(&stack, env, temp_env, symbol_env);
    if (out != PARSE_ERROR_NONE || \
        stack->se == NULL || \
        stack->se->car == NULL || \
        !match_typed_ptrs(stack->se->car, symbol_env_tp) || \
        symbol_lookup_name(temp_env, symbol_env) != NULL) {
        pass = false;
    }
    free(symbol_env_tp);
    free(stack->se->car);
    stack->se->car = NULL;
    // passing a symbol already in temp_env
    char symbol_temp_env[] = "in-temp-env";
    typed_ptr* symbol_temp_env_tp = install_symbol(temp_env, \
                                                   symbol_temp_env, \
                                                   &thousand);
    out = register_symbol(&stack, env, temp_env, symbol_temp_env);
    if (out != PARSE_ERROR_NONE || \
        stack->se == NULL || \
        stack->se->car == NULL || \
        !match_typed_ptrs(stack->se->car, symbol_temp_env_tp) || \
        symbol_lookup_name(env, symbol_temp_env) != NULL) {
        pass = false;
    }
    free(symbol_temp_env_tp);
    free(stack->se->car);
    stack->se->car = NULL;
    // passing a symbol in neither environment
    char symbol_absent[] = "absent";
    out = register_symbol(&stack, env, temp_env, symbol_absent);
    Symbol_Node* sn_from_string = symbol_lookup_name(temp_env, symbol_absent);
    Symbol_Node* sn_from_index = symbol_lookup_index(temp_env, stack->se->car);
    if (out != PARSE_ERROR_NONE || \
        stack->se == NULL || \
        stack->se->car == NULL || \
        symbol_lookup_name(env, symbol_absent) != NULL || \
        sn_from_string == NULL || \
        sn_from_string != sn_from_index || \
        strcmp(sn_from_index->name, symbol_absent) || \
        sn_from_index->type != TYPE_UNDEF) {
        pass = false;
    }
    free(stack->se->car);
    free(stack->se);
    free(stack);
    delete_environment_full(env);
    delete_environment_full(temp_env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_substring(test_env* te) {
    print_test_announce("substring()");
    char big_string[] = "This is an entire sentence.";
    bool pass = true;
    // length zero substring, beginning
    char* out = substring(big_string, 0, 0);
    if (strlen(out) != 0 || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length zero substring, middle
    out = substring(big_string, 5, 5);
    if (strlen(out) != 0 || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length zero substring, end
    out = substring(big_string, strlen(big_string), strlen(big_string));
    if (strlen(out) != 0 || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length one substring, first character
    out = substring(big_string, 0, 1);
    if (strlen(out) != 1 || \
        out[0] != big_string[0] || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length one substring, middle of string
    out = substring(big_string, 5, 6);
    if (strlen(out) != 1 || \
        out[0] != big_string[5] || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length one substring, final character
    out = substring(big_string, strlen(big_string) - 1, strlen(big_string));
    if (strlen(out) != 1 || \
        out[0] != big_string[strlen(big_string) - 1] || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length >1 substring, anchored at beginning
    out = substring(big_string, 0, 4);
    if (strlen(out) != 4 || \
        strcmp(out, "This") || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length >1 substring, middle of string
    out = substring(big_string, 11, 17);
    if (strlen(out) != 6 || \
        strcmp(out, "entire") || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // length >1 substring, anchored at end
    out = substring(big_string, 18, strlen(big_string));
    if (strlen(out) != 9 || \
        strcmp(out, "sentence.") || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    // substring of length == total string length
    out = substring(big_string, 0, strlen(big_string));
    if (strlen(out) != strlen(big_string) || \
        strcmp(out, big_string) || \
        out[strlen(out)] != '\0') {
        pass = false;
    }
    free(out);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

void test_string_is_number(test_env* te) {
    print_test_announce("string_is_number()");
    bool pass = true;
    if (string_is_number("") || \
        string_is_number("-") || \
        string_is_number("a") || \
        string_is_number("a1") || \
        string_is_number("1a") || \
        string_is_number("1a1")) {
        pass = false;
    }
    if (!string_is_number("0") || \
        !string_is_number("1") || \
        !string_is_number("-2") || \
        !string_is_number("1001") || \
        !string_is_number("-9876")) {
        pass = false;
    }
    if (string_is_number("1.2") || \
        string_is_number("1.") || \
        string_is_number("-1.2") || \
        string_is_number(".3") || \
        string_is_number("0.3") || \
        string_is_number("-.4") || \
        string_is_number("-0.5") || \
        string_is_number("123.456")) {
        pass = false;
    }
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

bool test_parse_output(const char cmd[], typed_ptr* expected) {
    Environment* env = create_environment(0, 0);
    typed_ptr* out = parse(cmd, env);
    bool pass = true;
    if (out == NULL || \
        out->type != expected->type || \
        (out->type == TYPE_S_EXPR && \
         !match_s_exprs(out->ptr.se_ptr, expected->ptr.se_ptr)) || \
        (out->type != TYPE_S_EXPR && \
         !match_typed_ptrs(out, expected))) {
        pass = false;
    }
    if (out->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    if (expected->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(expected->ptr.se_ptr, true);
    }
    free(out);
    free(expected);
    delete_environment_full(env);
    return pass;
}

void test_parse(test_env* te) {
    print_test_announce("parse()");
    bool pass = true;
    // errors:
    // "a"
    typed_ptr* expected = create_error_tp(PARSE_ERROR_BARE_SYM);
    pass = test_parse_output("a", expected) && pass;
    // "("
    expected = create_error_tp(PARSE_ERROR_UNBAL_PAREN);
    pass = test_parse_output("(", expected) && pass;
    // ")"
    expected = create_error_tp(PARSE_ERROR_UNBAL_PAREN);
    pass = test_parse_output(")", expected) && pass;
    // "(()"
    expected = create_error_tp(PARSE_ERROR_UNBAL_PAREN);
    pass = test_parse_output("(()", expected) && pass;
    // ")("
    expected = create_error_tp(PARSE_ERROR_UNBAL_PAREN);
    pass = test_parse_output(")(", expected) && pass;
    // "() ()"
    expected = create_error_tp(PARSE_ERROR_TOO_MANY);
    pass = test_parse_output("() ()", expected) && pass;
    // "(a"
    expected = create_error_tp(PARSE_ERROR_UNBAL_PAREN);
    pass = test_parse_output("(a", expected) && pass;
    // valid:
    // ""
    expected = create_s_expr_tp(create_empty_s_expr());
    pass = test_parse_output("", expected) && pass;
    // "()"
    expected = create_s_expr_tp(create_empty_s_expr());
    pass = test_parse_output("()", expected) && pass;
    // "(a)"
    // this is a little fiddly - we have to anticipate the symbol number "a"
    //   gets assigned
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    pass = test_parse_output("(a)", expected) && pass;
    // "(a a a)"
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    pass = test_parse_output("(a a a)", expected) && pass;
    // "(a (a a) a)"
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr* sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    pass = test_parse_output("(a (a a) a)", expected) && pass;
    // "(a (a a) (a a) a)"
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(copy_s_expr(sublist)));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    pass = test_parse_output("(a (a a) (a a) a)", expected) && pass;
    // "(a (a a) a (a a))"
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(copy_s_expr(sublist)));
    pass = test_parse_output("(a (a a) a (a a))", expected) && pass;
    // "((a))"
    sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    pass = test_parse_output("((a))", expected) && pass;
    // "((a) a a)"
    sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    pass = test_parse_output("((a) a a)", expected) && pass;
    // "((a a) a a)"
    sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    pass = test_parse_output("((a a) a a)", expected) && pass;
    // "(((a a) a) a (a a))"
    sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    sublist = create_s_expr(create_s_expr_tp(sublist), \
                            create_s_expr_tp(create_empty_s_expr()));
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    expected = create_s_expr_tp(create_empty_s_expr());
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
    sublist = create_empty_s_expr();
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(sublist, create_atom_tp(TYPE_SYMBOL, 0));
    s_expr_append(expected->ptr.se_ptr, create_s_expr_tp(sublist));
    pass = test_parse_output("(((a a) a) a (a a))", expected) && pass;
    // playing with whitespace
    unsigned int num_ws_inputs = 15;
    char* ws_inputs[] = {"  (a)", \
                         "(a)   ", \
                         "  (a)  ", \
                         "(a  )", \
                         "(  a)", \
                         "(  a  )", \
                         "(a  )  ", \
                         "  (a  )", \
                         "  (a  )  ", \
                         "(  a)  ", \
                         "  (  a)", \
                         "  (  a)  ", \
                         "(  a  )  ", \
                         "  (  a  )", \
                         "  (  a  )  "};
    for (unsigned int idx = 0; idx < num_ws_inputs; idx++) {
        expected = create_s_expr_tp(create_empty_s_expr());
        s_expr_append(expected->ptr.se_ptr, create_atom_tp(TYPE_SYMBOL, 0));
        pass = test_parse_output(ws_inputs[idx], expected) && pass;
    }
    // testing symbol registration
    Environment* env = create_environment(0, 0);
    // "("
    typed_ptr* out = parse("(", env);
    if (env->symbol_table->length != 0 || \
        env->symbol_table->head != NULL) {
        pass = false;
    }
    if (out != NULL && out->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // "()"
    out = parse("()", env);
    if (env->symbol_table->length != 0 || \
        env->symbol_table->head != NULL) {
        pass = false;
    }
    if (out != NULL && out->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // "(a)"
    out = parse("(a)", env);
    if (env->symbol_table->length != 1 || \
        symbol_lookup_name(env, "a") == NULL || \
        symbol_lookup_name(env, "a")->type != TYPE_UNDEF) {
        pass = false;
    }
    if (out != NULL && out->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // "(a b)"
    out = parse("(a b)", env);
    if (env->symbol_table->length != 2 || \
        symbol_lookup_name(env, "a") == NULL || \
        symbol_lookup_name(env, "a")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "b") == NULL || \
        symbol_lookup_name(env, "b")->type != TYPE_UNDEF) {
        pass = false;
    }
    if (out != NULL && out->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    // "(a b c"
    out = parse("(a b c", env);
    if (env->symbol_table->length != 2 || \
        symbol_lookup_name(env, "a") == NULL || \
        symbol_lookup_name(env, "a")->type != TYPE_UNDEF || \
        symbol_lookup_name(env, "b") == NULL || \
        symbol_lookup_name(env, "b")->type != TYPE_UNDEF) {
        pass = false;
    }
    if (out != NULL && out->type == TYPE_S_EXPR) {
        delete_s_expr_recursive(out->ptr.se_ptr, true);
    }
    free(out);
    delete_environment_full(env);
    print_test_result(pass);
    te->passed += pass;
    te->run++;
    return;
}

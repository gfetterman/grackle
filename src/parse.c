#include "parse.h"

// The s-expression storage node is the caller's responsibility to free.
s_expr_storage* create_s_expr_storage(unsigned int list_number, s_expr* se) {
    s_expr_storage* new_ses = malloc(sizeof(s_expr_storage));
    if (new_ses == NULL) {
        fprintf(stderr, "malloc failed in create_s_expr_storage()\n");
        exit(-1);
    }
    new_ses->list_number = list_number;
    new_ses->se = se;
    new_ses->next = NULL;
    return new_ses;
}

// This stack is used to keep track of open s-expressions during parsing.
// It doesn't use the list_number member of s_expr_storage, so this function
//   should not be used to install s-expressions into the list area.
void se_stack_push(s_expr_storage** stack, s_expr* new_se) {
    if (stack == NULL) {
        fprintf(stderr, "stack double pointer NULL in se_stack_push()\n");
        exit(-1);
    }
    s_expr_storage* new_node = create_s_expr_storage(0, new_se);
    new_node->next = *stack;
    *stack = new_node;
    return;
}

// This stack is used to keep track of open s-expressions during parsing.
// This function should not be used to remove s-expressions from the list area.
// While it pops the top item off of the stack and frees the s_expr_storage, it
//   does not free the s-expressions stored therein.
// If the stack is empty, this will fail and exit the interpreter.
void se_stack_pop(s_expr_storage** stack) {
    if (stack == NULL) {
        fprintf(stderr, "stack double pointer NULL in se_stack_pop()\n");
        exit(-1);
    }
    if (*stack == NULL) {
        fprintf(stderr, "cannot pop() from empty se_stack\n");
        exit(-1);
    }
    s_expr_storage* old_head = *stack;
    *stack = old_head->next;
    free(old_head);
    return;
}

// Takes an input string and attempts to parse it into a valid s-expression.
// If it cannot be parsed, returns an s-expression containing an error
//   typed_ptr. In this case, no changes are made to the given symbol table or
//   list area.
// If successful, returns an s-expression. The given symbol table is updated to
//   include any new symbols encountered during the parse; their type is
//   TYPE_UNDEF. The given list area is updated to include the new s-expression
//   command's nodes.
// In either case, the s-expression returned is the caller's responsibility to
//   free; it may be (shallow) freed without harm to the list area, symbol
//   table, or any other object.
s_expr* parse(char str[], environment* env) {
    enum Parse_State {PARSE_START, \
                      PARSE_NEW_SEXPR, \
                      PARSE_READY, \
                      PARSE_READ_SYMBOL, \
                      PARSE_FINISH, \
                      PARSE_ERROR};
    enum Parse_State state = PARSE_START;
    interpreter_error error_code = PARSE_ERROR_NONE;
    s_expr_storage* stack = NULL;
    s_expr* new_s_expr = NULL;
    unsigned int curr = 0;
    unsigned int symbol_start = 0;
    environment* temp_env = create_environment(env->symbol_table->length, \
                                               env->function_table->length);
    s_expr* head = NULL;
    typed_ptr* new_tp = NULL;
    while (str[curr] && state != PARSE_ERROR) {
        switch (state) {
            case PARSE_START:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        head = create_empty_s_expr();
                        se_stack_push(&stack, head);
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        state = PARSE_ERROR;
                        error_code = PARSE_ERROR_UNBAL_PAREN;
                        break;
                    default:
                        state = PARSE_ERROR;
                        error_code = PARSE_ERROR_BARE_SYM;
                        break;
                }
                break;
            case PARSE_NEW_SEXPR:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        new_s_expr = create_empty_s_expr();
                        stack->se->car = create_sexpr_tp(new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        break;
                    case ')':
                        if (stack == NULL) {
                            state = PARSE_ERROR;
                            error_code = PARSE_ERROR_UNBAL_PAREN;
                        } else {
                            // this will necessarily be an empty list
                            // so we needn't terminate it with one
                            se_stack_pop(&stack);
                            while (stack != NULL && stack->se->cdr != NULL) {
                                se_stack_pop(&stack);
                            }
                            state = (stack == NULL) ? PARSE_FINISH : \
                                                      PARSE_READY;
                        }
                        break;
                    default:
                        symbol_start = curr;
                        state = PARSE_READ_SYMBOL;
                        break;
                }
                break;
            case PARSE_READY:
                switch (str[curr]) {
                    case ' ': // lump whitespace together
                        break;
                    case '(':
                        // create the next backbone node
                        new_s_expr = create_empty_s_expr();
                        stack->se->cdr = create_sexpr_tp(new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        // create the new s-expression's initial node
                        new_s_expr = create_empty_s_expr();
                        stack->se->car = create_sexpr_tp(new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        if (stack == NULL || stack->se->cdr != NULL) {
                            state = PARSE_ERROR;
                            error_code = PARSE_ERROR_UNBAL_PAREN;
                        } else {
                            // terminate the stack's top s-expression
                            new_s_expr = create_empty_s_expr();
                            stack->se->cdr = create_sexpr_tp(new_s_expr);
                            se_stack_pop(&stack);
                            while (stack != NULL && stack->se->cdr != NULL) {
                                se_stack_pop(&stack);
                            }
                            state = (stack == NULL) ? PARSE_FINISH : \
                                                      PARSE_READY;
                        }
                        break;
                    default:
                        symbol_start = curr;
                        se_stack_push(&stack, create_empty_s_expr());
                        stack->next->se->cdr = create_sexpr_tp(stack->se);
                        state = PARSE_READ_SYMBOL;
                        break;
                }
                break;
            case PARSE_READ_SYMBOL:
                switch (str[curr]) {
                    case ' ':
                        // update the current backbone node
                        new_tp = install_symbol_substring(env, \
                                                          temp_env, \
                                                          str, \
                                                          symbol_start, \
                                                          curr);
                        stack->se->car = new_tp;
                        state = PARSE_READY;
                        break;
                    case '(':
                        // update the current backbone node
                        new_tp = install_symbol_substring(env, \
                                                          temp_env, \
                                                          str, \
                                                          symbol_start, \
                                                          curr);
                        stack->se->car = new_tp;
                        // create the next backbone node
                        new_s_expr = create_empty_s_expr();
                        stack->se->cdr = create_sexpr_tp(new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        // create the new s-expression's initial node
                        new_s_expr = create_empty_s_expr();
                        stack->se->car = create_sexpr_tp(new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        // update the current backbone node
                        new_tp = install_symbol_substring(env, \
                                                          temp_env, \
                                                          str, \
                                                          symbol_start, \
                                                          curr);
                        stack->se->car = new_tp;
                        if (stack == NULL || stack->se->cdr != NULL) {
                            state = PARSE_ERROR;
                            error_code = PARSE_ERROR_UNBAL_PAREN;
                        } else {
                            // terminate the stack's top s-expression
                            new_s_expr = create_empty_s_expr();
                            stack->se->cdr = create_sexpr_tp(new_s_expr);
                            se_stack_pop(&stack);
                            while (stack != NULL && stack->se->cdr != NULL) {
                                se_stack_pop(&stack);
                            }
                            state = (stack == NULL) ? PARSE_FINISH : \
                                                      PARSE_READY;
                        }
                        break;
                    default: // just keep reading
                        break;
                }
                break;
            case PARSE_FINISH:
                switch (str[curr]) {
                    case ' ': // ignore trailing whitespace
                        break;
                    case '(':
                        state = PARSE_ERROR;
                        error_code = PARSE_ERROR_TOO_MANY;
                        break;
                    case ')':
                        state = PARSE_ERROR;
                        error_code = PARSE_ERROR_UNBAL_PAREN;
                        break;
                    default:
                        state = PARSE_ERROR;
                        error_code = PARSE_ERROR_BARE_SYM;
                        break;
                }
                break;
            default:
                fprintf(stderr, "fatal error: parser in unknown state\n");
                exit(-1);
        }
        curr++;
    }
    if (state != PARSE_ERROR && stack != NULL) {
        state = PARSE_ERROR;
        error_code = PARSE_ERROR_UNBAL_PAREN;
    }
    if (error_code == PARSE_ERROR_NONE) {
        merge_symbol_tables(env->symbol_table, temp_env->symbol_table);
        free(temp_env->symbol_table);
        free(temp_env->function_table);
        free(temp_env);
    } else {
        while (stack != NULL) {
            s_expr_storage* stack_temp = stack;
            stack = stack->next;
            // s-expressions pointed to on the stack are accessible from head
            free(stack_temp);
        }
        sym_tab_node* symbol_curr = temp_env->symbol_table->head;
        while (symbol_curr != NULL) {
            sym_tab_node* symbol_temp = symbol_curr;
            symbol_curr = symbol_curr->next;
            free(symbol_temp);
        }
        free(temp_env->symbol_table);
        free(temp_env->function_table);
        free(temp_env);
        delete_se_recursive(head, true);
        head = create_s_expr(create_error(error_code), \
                             create_sexpr_tp(create_empty_s_expr()));
    }
    return head;
}

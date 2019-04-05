#include "parse.h"

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
typed_ptr* parse(const char str[], Environment* env) {
    Parse_State state = PARSE_START;
    interpreter_error error = PARSE_ERROR_NONE;
    s_expr_stack* stack = NULL;
    unsigned int curr = 0;
    unsigned int symbol_start = 0;
    char* new_symbol = NULL;
    Environment* temp_env = create_environment(env->symbol_table->length, \
                                               env->function_table->length);
    s_expr* head = create_empty_s_expr();
    while (str[curr] && state != PARSE_ERROR) {
        switch (state) {
            case PARSE_START:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        s_expr_stack_push(&stack, head);
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        state = PARSE_ERROR;
                        error = PARSE_ERROR_UNBAL_PAREN;
                        break;
                    default:
                        state = PARSE_ERROR;
                        error = PARSE_ERROR_BARE_SYM;
                        break;
                }
                break;
            case PARSE_NEW_SEXPR:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        init_new_s_expr(&stack);
                        break;
                    case ')':
                        state = terminate_s_expr(&stack, &error);
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
                        extend_s_expr(&stack);
                        init_new_s_expr(&stack);
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        state = terminate_s_expr(&stack, &error);
                        break;
                    default:
                        symbol_start = curr;
                        extend_s_expr(&stack);
                        state = PARSE_READ_SYMBOL;
                        break;
                }
                break;
            case PARSE_READ_SYMBOL:
                if (str[curr] == ' ' || str[curr] == '(' || str[curr] == ')') {
                    new_symbol = substring(str, symbol_start, curr);
                    register_symbol(&stack, env, temp_env, new_symbol);
                    new_symbol = NULL;
                }
                switch (str[curr]) {
                    case ' ':
                        state = PARSE_READY;
                        break;
                    case '(':
                        extend_s_expr(&stack);
                        init_new_s_expr(&stack);
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        state = terminate_s_expr(&stack, &error);
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
                        error = PARSE_ERROR_TOO_MANY;
                        break;
                    case ')':
                        state = PARSE_ERROR;
                        error = PARSE_ERROR_UNBAL_PAREN;
                        break;
                    default:
                        state = PARSE_ERROR;
                        error = PARSE_ERROR_BARE_SYM;
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
        error = PARSE_ERROR_UNBAL_PAREN;
    }
    if (state != PARSE_ERROR) {
        merge_symbol_tables(env->symbol_table, temp_env->symbol_table);
    } else {
        while (stack != NULL) {
            s_expr_stack* stack_temp = stack;
            stack = stack->next;
            // s-expressions pointed to on the stack are accessible from head
            free(stack_temp);
        }
        delete_s_expr_recursive(head, true);
    }
    delete_environment_full(temp_env);
    return (state == PARSE_ERROR) ? create_error_tp(error) : \
                                    create_s_expr_tp(head);
}

void init_new_s_expr(s_expr_stack** stack) {
    s_expr_stack_push(stack, create_empty_s_expr());
    (*stack)->next->se->car = create_s_expr_tp((*stack)->se);
    return;
}

void extend_s_expr(s_expr_stack** stack) {
    s_expr_stack_push(stack, create_empty_s_expr());
    (*stack)->next->se->cdr = create_s_expr_tp((*stack)->se);
    return;
}

Parse_State terminate_s_expr(s_expr_stack** stack, interpreter_error* error) {
    if (*stack == NULL || (*stack)->se->cdr != NULL) {
        *error = PARSE_ERROR_UNBAL_PAREN;
        return PARSE_ERROR;
    } else {
        if (!is_empty_list((*stack)->se)) {
            (*stack)->se->cdr = create_s_expr_tp(create_empty_s_expr());
        }
        s_expr_stack_pop(stack);
        while (*stack != NULL && (*stack)->se->cdr != NULL) {
            s_expr_stack_pop(stack);
        }
        return (*stack == NULL) ? PARSE_FINISH : PARSE_READY;
    }
}

void register_symbol(s_expr_stack** stack, \
                     Environment* env, \
                     Environment* temp, \
                     char* name) {
    typed_ptr* car = NULL;
    if (string_is_number(name)) {
        long value = atol(name);
        free(name);
        car = create_atom_tp(TYPE_NUM, value);
    } else {
        Symbol_Node* found = symbol_lookup_string(env, name);
        found = (found == NULL) ? symbol_lookup_string(temp, name) : found;
        if (found == NULL) {
            car = install_symbol(temp, name, TYPE_UNDEF, (tp_value){.idx=0});
        } else {
            free(name);
            car = create_atom_tp(TYPE_SYMBOL, found->symbol_idx);
        }
    }
    (*stack)->se->car = car;
    return;
}

// The s-expression storage node is the caller's responsibility to free.
s_expr_stack* create_s_expr_stack(s_expr* se) {
    s_expr_stack* new_ses = malloc(sizeof(s_expr_stack));
    if (new_ses == NULL) {
        fprintf(stderr, "malloc failed in create_s_expr_stack()\n");
        exit(-1);
    }
    new_ses->se = se;
    new_ses->next = NULL;
    return new_ses;
}

// This stack is used to keep track of open s-expressions during parsing.
void s_expr_stack_push(s_expr_stack** stack, s_expr* new_se) {
    if (stack == NULL) {
        fprintf(stderr, "stack double pointer NULL in s_expr_stack_push()\n");
        exit(-1);
    }
    s_expr_stack* new_node = create_s_expr_stack(new_se);
    new_node->next = *stack;
    *stack = new_node;
    return;
}

// This stack is used to keep track of open s-expressions during parsing.
// While it pops the top item off of the stack and frees the s_expr_stack, it
//   does not free the s-expressions stored therein.
// If the stack is empty, this will fail and exit the interpreter.
void s_expr_stack_pop(s_expr_stack** stack) {
    if (stack == NULL) {
        fprintf(stderr, "stack double pointer NULL in s_expr_stack_pop()\n");
        exit(-1);
    }
    if (*stack == NULL) {
        fprintf(stderr, "cannot pop() from empty se_stack\n");
        exit(-1);
    }
    s_expr_stack* old_head = *stack;
    *stack = old_head->next;
    free(old_head);
    return;
}

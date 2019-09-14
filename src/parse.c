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
    unsigned int string_start = 0;
    char* new_symbol = NULL;
    char* new_string = NULL;
    Environment* temp_env = create_environment(env->symbol_table->length, \
                                               env->function_table->length, \
                                               NULL);
    s_expr* head = create_empty_s_expr();
    while (str[curr] && state != PARSE_ERROR) {
        switch (state) {
            case PARSE_START:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        s_expr_stack_push(&stack, head);
                        state = PARSE_NEW_S_EXPR;
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
            case PARSE_NEW_S_EXPR:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        init_new_s_expr(&stack);
                        break;
                    case ')':
                        state = terminate_s_expr(&stack, &error);
                        break;
                    case '"':
                        string_start = curr;
                        state = PARSE_READ_STRING;
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
                        state = PARSE_NEW_S_EXPR;
                        break;
                    case ')':
                        state = terminate_s_expr(&stack, &error);
                        break;
                    case '"':
                        string_start = curr;
                        extend_s_expr(&stack);
                        state = PARSE_READ_STRING;
                        break;
                    default:
                        symbol_start = curr;
                        extend_s_expr(&stack);
                        state = PARSE_READ_SYMBOL;
                        break;
                }
                break;
            case PARSE_READ_SYMBOL:
                if (str[curr] == ' ' || \
                    str[curr] == '(' || \
                    str[curr] == ')' || \
                    str[curr] == '"') {
                    new_symbol = substring(str, symbol_start, curr);
                    error = register_symbol(&stack, env, temp_env, new_symbol);
                    free(new_symbol);
                    new_symbol = NULL;
                    if (error != PARSE_ERROR_NONE) {
                        state = PARSE_ERROR;
                        break;
                    }
                }
                switch (str[curr]) {
                    case ' ':
                        state = PARSE_READY;
                        break;
                    case '(':
                        extend_s_expr(&stack);
                        init_new_s_expr(&stack);
                        state = PARSE_NEW_S_EXPR;
                        break;
                    case ')':
                        state = terminate_s_expr(&stack, &error);
                        break;
                    case '"':
                        string_start = curr;
                        extend_s_expr(&stack);
                        state = PARSE_READ_STRING;
                        break;
                    default: // just keep reading
                        break;
                }
                break;
            case PARSE_READ_STRING:
                switch (str[curr]) {
                    case '"':
                        new_string = substring(str, string_start + 1, curr);
                        stack->se->car = create_string_tp(create_string(new_string));
                        free(new_string);
                        new_string = NULL;
                        state = PARSE_READY;
                        break;
                    case '\\':
                        state = PARSE_STRING_ESCAPE;
                        break;
                    default: // just keep reading
                        break;
                }
                break;
            case PARSE_STRING_ESCAPE:
                state = PARSE_READ_STRING;
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
    if (state == PARSE_READ_STRING || state == PARSE_STRING_ESCAPE) {
        state = PARSE_ERROR;
        error = PARSE_ERROR_UNBAL_DOUBLE_QUOTE;
    } else if (state != PARSE_ERROR && stack != NULL) {
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
    delete_environment(temp_env);
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

interpreter_error register_symbol(s_expr_stack** stack, \
                                  Environment* env, \
                                  Environment* temp_env, \
                                  char* name) {
    typed_ptr* tp = NULL;
    if (string_is_number(name)) {
        errno = 0;
        long value = strtol(name, NULL, 10);
        if (value == 0 && errno == EINVAL) {
            return PARSE_ERROR_INT_UNSPEC;
        } else if (value == LONG_MIN && errno == ERANGE) {
            return PARSE_ERROR_INT_TOO_LOW;
        } else if (value == LONG_MAX && errno == ERANGE) {
            return PARSE_ERROR_INT_TOO_HIGH;
        } else {
            tp = create_atom_tp(TYPE_FIXNUM, value);
        }
    } else if (string_is_boolean_literal(name)) {
        tp = create_atom_tp(TYPE_BOOL, (!strcmp(name, "#t")) ? true : false);
    } else {
        Symbol_Node* found = symbol_lookup_name(env, name);
        found = (found == NULL) ? symbol_lookup_name(temp_env, name) : found;
        if (found == NULL) {
            typed_ptr undef_tp = {.type=TYPE_UNDEF, .ptr={.idx=0}};
            tp = install_symbol(temp_env, name, &undef_tp);
        } else {
            tp = create_atom_tp(TYPE_SYMBOL, found->symbol_idx);
        }
    }
    (*stack)->se->car = tp;
    return PARSE_ERROR_NONE;
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

// The string returned is a valid null-terminated C string.
// The string returned is the caller's responsibility to free.
char* substring(const char* str, unsigned int start, unsigned int end) {
    unsigned int len = strlen(str);
    if (str == NULL || len < (end - start)) {
        fprintf(stderr, \
                "fatal error: bad substring from %u to %u, for str len %u\n", \
                start, \
                end, \
                len);
        exit(-1);
    }
    char* ss = malloc(sizeof(char) * (end - start + 1));
    if (ss == NULL) {
        fprintf(stderr, "fatal error: malloc failed in substring()\n");
        exit(-1);
    }
    memcpy(ss, (str + start), (sizeof(char) * (end - start)));
    ss[end - start] = '\0';
    return ss;
}

// Determines whether a string represents a number (rather than a symbol).
// Currently only recognizes integers.
bool string_is_number(const char str[]) {
    char c;
    bool ok = true;
    if (*str == '-') {
        str++;
    }
    if (*str == '\0') {
        ok = false;
    } else {
        while ((c = *str++)) {
            if (c < 48 || c > 57) {
                ok = false;
                break;
            }
        }
    }
    return ok;
}

bool string_is_boolean_literal(const char str[]) {
    return (!strcmp(str, "#t") || !strcmp(str, "#f"));
}

#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

#define PROMPT ">>>"
#define BUF_SIZE 80
#define EMPTY_LIST_IDX 0

typedef enum {TYPE_UNDEF, \
              TYPE_ERROR, \
              TYPE_VOID, \
              TYPE_BUILTIN, \
              TYPE_NUM, \
              TYPE_BOOL, \
              TYPE_SEXPR, \
              TYPE_SYM} type;

typedef struct TYPED_PTR {
    type type;
    unsigned int ptr;
} typed_ptr;

typed_ptr* create_typed_ptr(type type, unsigned int ptr) {
    typed_ptr* new_tp = malloc(sizeof(typed_ptr));
    if (new_tp == NULL) {
        fprintf(stderr, "malloc failed in create_typed_ptr()\n");
        exit(-1);
    }
    new_tp->type = type;
    new_tp->ptr = ptr;
    return new_tp;
}

typedef struct SYMBOL_TABLE_NODE {
    unsigned int symbol_number;
    char* symbol;
    type type;
    unsigned int value;
    struct SYMBOL_TABLE_NODE* next;
} symbol_table_node;

symbol_table_node* create_st_node(unsigned int symbol_number, \
                                  char* symbol, \
                                  type type, \
                                  unsigned int value) {
    symbol_table_node* new_node = malloc(sizeof(symbol_table_node));
    if (new_node == NULL) {
        fprintf(stderr, "fatal error: malloc failed in create_st_node()\n");
        exit(-1);
    }
    new_node->symbol_number = symbol_number;
    new_node->symbol = symbol;
    new_node->type = type;
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

typedef struct SYMBOL_TABLE {
    symbol_table_node* head;
    unsigned int length;
    unsigned int symbol_number_offset;
} Symbol_Table;

Symbol_Table* create_symbol_table(unsigned int offset) {
    Symbol_Table* new_st = malloc(sizeof(Symbol_Table));
    if (new_st == NULL) {
        fprintf(stderr, "malloc failed in create_symbol_table()\n");
        exit(-1);
    }
    new_st->head = NULL;
    new_st->length = 0;
    new_st->symbol_number_offset = offset;
    return new_st;
}

symbol_table_node* symbol_lookup_string(Symbol_Table* st, char* symbol) {
    symbol_table_node* curr = st->head;
    while (curr != NULL) {
        if (!strcmp(curr->symbol, symbol)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

symbol_table_node* symbol_lookup_index(Symbol_Table* st, unsigned int index) {
    symbol_table_node* curr = st->head;
    while (curr != NULL) {
        if (curr->symbol_number == index) {
            break;
        }
        curr = curr->next;
    }
    return curr;
}

typed_ptr* install_symbol(Symbol_Table* st, \
                          char* symbol, \
                          type type, \
                          unsigned int value) {
    unsigned int symbol_number = st->length + st->symbol_number_offset;
    symbol_table_node* found = symbol_lookup_string(st, symbol);
    if (found == NULL) {
        symbol_table_node* new_node = create_st_node(symbol_number, symbol, type, value);
        new_node->next = st->head;
        st->head = new_node;
        st->length++;
    } else {
        free(symbol);
        found->type = type;
        found->value = value;
        symbol_number = found->symbol_number;
    }
    return create_typed_ptr((type != TYPE_BUILTIN) ? TYPE_SYM : type, symbol_number);
}

void blind_install_symbol(Symbol_Table* st, \
                          char* symbol, \
                          type type, \
                          unsigned int value) {
    typed_ptr* tp = install_symbol(st, symbol, type, value);
    free(tp);
    return;
}

char* substring(char* str, unsigned int start, unsigned int end) {
    if (str == NULL || strlen(str) < (end - start)) {
        fprintf(stderr, \
                "fatal error: substring bad address: start %d -> end %d in string of length %d\n", \
                start,
                end,
                (int)strlen(str));
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

void print_symbol_table(Symbol_Table* st) {
    symbol_table_node* curr = st->head;
    printf("current symbol table:\n");
    while (curr != NULL) {
        printf("  symbol #%d, \"%s\", has type \"%d\" and value %u\n", \
               curr->symbol_number, \
               curr->symbol, \
               curr->type, \
               curr->value);
        curr = curr->next;
    }
    return;
}

typedef enum {BUILTIN_ADD, \
              BUILTIN_MUL, \
              BUILTIN_SUB, \
              BUILTIN_DIV, \
              BUILTIN_SETQ, \
              BUILTIN_EXIT, \
              BUILTIN_CONS, \
              BUILTIN_CAR, \
              BUILTIN_CDR, \
              BUILTIN_LIST, \
              BUILTIN_AND, \
              BUILTIN_OR, \
              BUILTIN_NOT, \
              BUILTIN_COND, \
              BUILTIN_PAIRPRED, \
              BUILTIN_LISTPRED, \
              BUILTIN_NUMBERPRED, \
              BUILTIN_BOOLPRED, \
              BUILTIN_VOIDPRED, \
              BUILTIN_NUMBEREQ, \
              BUILTIN_NUMBERGT, \
              BUILTIN_NUMBERLT, \
              BUILTIN_NUMBERGE, \
              BUILTIN_NUMBERLE} builtin_code;

void setup_symbol_table(Symbol_Table* st) {
    blind_install_symbol(st, "NULL_SENTINEL", TYPE_UNDEF, 0);
    blind_install_symbol(st, "+", TYPE_BUILTIN, BUILTIN_ADD);
    blind_install_symbol(st, "*", TYPE_BUILTIN, BUILTIN_MUL);
    blind_install_symbol(st, "-", TYPE_BUILTIN, BUILTIN_SUB);
    blind_install_symbol(st, "/", TYPE_BUILTIN, BUILTIN_DIV);
    blind_install_symbol(st, "setq", TYPE_BUILTIN, BUILTIN_SETQ);
    blind_install_symbol(st, "exit", TYPE_BUILTIN, BUILTIN_EXIT);
    blind_install_symbol(st, "cons", TYPE_BUILTIN, BUILTIN_CONS);
    blind_install_symbol(st, "car", TYPE_BUILTIN, BUILTIN_CAR);
    blind_install_symbol(st, "cdr", TYPE_BUILTIN, BUILTIN_CDR);
    blind_install_symbol(st, "and", TYPE_BUILTIN, BUILTIN_AND);
    blind_install_symbol(st, "or", TYPE_BUILTIN, BUILTIN_OR);
    blind_install_symbol(st, "not", TYPE_BUILTIN, BUILTIN_NOT);
    blind_install_symbol(st, "cond", TYPE_BUILTIN, BUILTIN_COND);
    blind_install_symbol(st, "list", TYPE_BUILTIN, BUILTIN_LIST);
    blind_install_symbol(st, "pair?", TYPE_BUILTIN, BUILTIN_PAIRPRED);
    blind_install_symbol(st, "list?", TYPE_BUILTIN, BUILTIN_LISTPRED);
    blind_install_symbol(st, "number?", TYPE_BUILTIN, BUILTIN_NUMBERPRED);
    blind_install_symbol(st, "boolean?", TYPE_BUILTIN, BUILTIN_BOOLPRED);
    blind_install_symbol(st, "void?", TYPE_BUILTIN, BUILTIN_VOIDPRED);
    blind_install_symbol(st, "=", TYPE_BUILTIN, BUILTIN_NUMBEREQ);
    blind_install_symbol(st, ">", TYPE_BUILTIN, BUILTIN_NUMBERGT);
    blind_install_symbol(st, "<", TYPE_BUILTIN, BUILTIN_NUMBERLT);
    blind_install_symbol(st, ">=", TYPE_BUILTIN, BUILTIN_NUMBERGE);
    blind_install_symbol(st, "<=", TYPE_BUILTIN, BUILTIN_NUMBERLE);
    blind_install_symbol(st, "null", TYPE_SEXPR, EMPTY_LIST_IDX);
    blind_install_symbol(st, "#t", TYPE_BOOL, 1);
    blind_install_symbol(st, "#f", TYPE_BOOL, 0);
    return;
}

void get_input(char* prompt, char buffer[], unsigned int buffer_size) {
    printf("%s ", prompt);
    fgets(buffer, buffer_size, stdin); // yes, this is unsafe
    // drop newline at end of input
    if ((strlen(buffer) > 0) && (buffer[strlen(buffer) - 1] == '\n')) {
        buffer[strlen(buffer) - 1] = '\0';
    }
    return;
}

typedef struct S_EXPR_NODE {
    typed_ptr* car;
    typed_ptr* cdr;
} s_expr;

s_expr* create_s_expr(typed_ptr* car, typed_ptr* cdr) {
    s_expr* new_se = malloc(sizeof(s_expr));
    if (new_se == NULL) {
        fprintf(stderr, "malloc failed in create_s_expr()\n");
        exit(-1);
    }
    new_se->car = car;
    new_se->cdr = cdr;
    return new_se;
}

void delete_s_expr(s_expr* se) {
    if (se != NULL) {
        free(se->car);
        free(se->cdr);
        free(se);
    }
    return;
}

typedef struct S_EXPR_STORAGE_NODE {
    unsigned int list_number;
    s_expr* se;
    struct S_EXPR_STORAGE_NODE* next;
} s_expr_storage;

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

typedef struct LIST_AREA {
    s_expr_storage* head;
    unsigned int length;
    unsigned int offset;
} List_Area;

List_Area* create_list_area(unsigned int offset) {
    List_Area* new_la = malloc(sizeof(List_Area));
    if (new_la == NULL) {
        fprintf(stderr, "malloc failed in create_list_area()\n");
        exit(-1);
    }
    new_la->head = NULL;
    new_la->length = 0;
    new_la->offset = offset;
    return new_la;
}

typed_ptr* install_list(List_Area* la, s_expr* new_se) {
    // we don't check for duplicates - that'd take forever
    unsigned int num = la->length + la->offset;
    s_expr_storage* new_ses = create_s_expr_storage(num, new_se);
    new_ses->next = la->head;
    la->head = new_ses;
    la->length++;
    return create_typed_ptr(TYPE_SEXPR, num);
}

void set_list_area_member(List_Area* la, unsigned int idx, s_expr* new_se) {
    // this is a bit of a kludge
    s_expr_storage* new_ses = create_s_expr_storage(idx, new_se);
    new_ses->next = la->head;
    la->head = new_ses;
    la->length++;
    return;
}

void setup_list_area(List_Area* la) {
    set_list_area_member(la, EMPTY_LIST_IDX, create_s_expr(NULL, NULL));
    return;
}

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
    // note that we don't free() the s_expr contained in this se_stack_node
    // (bug-free) parsing ensures that it is still reachable from the head
    // variable in parse()
    // so it can be cleaned up as needed later
    // and in the meantime, it's still needed
    free(old_head);
    return;
}

bool string_is_number(char str[]) {
    char c;
    bool ok = true;
    while ((c = *str++)) {
        if (c < 48 || c > 57) {
            ok = false;
            break;
        }
    }
    return ok;
}

typed_ptr* install_symbol_substring(Symbol_Table* st, \
                                    Symbol_Table* temp_st, \
                                    char str[], \
                                    unsigned int start, \
                                    unsigned int end) {
    char* symbol = substring(str, start, end);
    if (string_is_number(symbol)) {
        unsigned int value = atoi(symbol);
        free(symbol);
        return create_typed_ptr(TYPE_NUM, value);
    } else {
        symbol_table_node* found = symbol_lookup_string(st, symbol);
        if (found == NULL) {
            found = symbol_lookup_string(temp_st, symbol);
            if (found == NULL) {
                return install_symbol(temp_st, symbol, TYPE_UNDEF, 0);
            }
        }
        free(symbol);
        return create_typed_ptr((found->type == TYPE_BUILTIN) ? found->type : TYPE_SYM, found->symbol_number);
    }
}

s_expr* list_from_index(List_Area* la, unsigned int index) {
    s_expr_storage* curr = la->head;
    while (curr != NULL) {
        if (curr->list_number == index) {
            break;
        }
        curr = curr->next;
    }
    return curr->se;
}

void print_se_recursive(s_expr* se, unsigned int depth, Symbol_Table* st, List_Area* la) {
    for (unsigned int i = 0; i < depth; i++) {
        printf("  ");
    }
    if (se == NULL) {
        printf("null\n");
    } else {
        if (se->car == NULL) {
            printf("car: NULL, ");
        } else {
            switch (se->car->type) {
                case TYPE_NUM:
                    printf("car: number %u, ", se->car->ptr);
                    break;
                case TYPE_SEXPR:
                    printf("car: s-expression #%u, ", se->car->ptr);
                    break;
                case TYPE_SYM:
                    printf("car: symbol #%u, ", se->car->ptr);
                    break;
                case TYPE_BUILTIN:
                    printf("car: built-in operator with symbol #%u, ", se->car->ptr);
                    break;
                case TYPE_BOOL:
                    printf("car: boolean %s, ", (se->car->ptr == 0) ? "#f" : "#t");
                    break;
                case TYPE_VOID:
                    printf("car: #<void>, ");
                default:
                    printf("car: unrecognized type: %d, ", se->car->type);
            }
        }
        if (se->cdr == NULL) {
            printf("cdr: NULL\n");
        } else {
            switch (se->cdr->type) {
                case TYPE_NUM:
                    printf("cdr: number %u, ", se->cdr->ptr);
                    break;
                case TYPE_SEXPR:
                    printf("cdr: s-expression #%u\n", se->cdr->ptr);
                    break;
                case TYPE_SYM:
                    printf("cdr: symbol #%u\n", se->cdr->ptr);
                    break;
                case TYPE_BUILTIN:
                    printf("cdr: built-in operator with symbol #%u, ", se->cdr->ptr);
                    break;
                case TYPE_BOOL:
                    printf("cdr: boolean %s, ", (se->cdr->ptr == 0) ? "#f" : "#t");
                    break;
                case TYPE_VOID:
                    printf("cdr: #<void>\n");
                default:
                    printf("cdr: unrecognized type: %d\n", se->cdr->type);
            }
        }
        if (se->car != NULL && se->car->type == TYPE_SEXPR) {
            print_se_recursive(list_from_index(la, se->car->ptr), depth + 1, st, la);
        }
        if (se->cdr != NULL && se->cdr->type == TYPE_SEXPR) {
            print_se_recursive(list_from_index(la, se->cdr->ptr), depth + 1, st, la);
        }
    }
    return;
}

void print_s_expr(s_expr* se, Symbol_Table* st, List_Area* la) {
    print_se_recursive(se, 0, st, la);
    return;
}

void print_list_area(Symbol_Table* st, List_Area* la) {
    s_expr_storage* curr = la->head;
    printf("current list area:\n");
    while (curr != NULL) {
        printf("  list #%u:\n", curr->list_number);
        print_s_expr(curr->se, st, la);
        curr = curr->next;
    }
    return;
}

void merge_list_areas(List_Area* first, List_Area* second) {
    if (first->head == NULL) {
        first->head = second->head;
    } else {
        s_expr_storage* curr = first->head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = second->head;
    }
    first->length += second->length;
    return;
}

void merge_symbol_tables(Symbol_Table* first, Symbol_Table* second) {
    if (first->head == NULL) {
        first->head = second->head;
    } else {
        symbol_table_node* curr = first->head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = second->head;
    }
    first->length += second->length;
    return;
}

s_expr* parse(char str[], Symbol_Table* st, List_Area* la) {
    enum Parse_State {START, NEW_SE, READY, READ_SYMBOL, FINISH, ERROR};
    enum Parse_State state = START;
    enum Error_Code {NONE, UNBAL_PAREN, BARE_SYM, EMPTY_PAREN, TOO_MANY};
    enum Error_Code error_code = NONE;
    s_expr_storage* stack = NULL;
    s_expr* new_s_expr = NULL;
    unsigned int curr = 0;
    unsigned int symbol_start = 0;
    Symbol_Table* temp_symbol_table = create_symbol_table(st->length);
    List_Area* temp_list_area = create_list_area(la->length);
    s_expr* head = NULL;
    while (str[curr] && state != ERROR) {
        switch (state) {
            case START:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        head = create_s_expr(NULL, NULL);
                        se_stack_push(&stack, head);
                        state = NEW_SE;
                        break;
                    case ')':
                        state = ERROR;
                        error_code = UNBAL_PAREN;
                        break;
                    default:
                        state = ERROR;
                        error_code = BARE_SYM;
                        break;
                }
                break;
            case NEW_SE:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->car = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        break;
                    case ')':
                        state = ERROR;
                        error_code = EMPTY_PAREN;
                        break;
                    default:
                        symbol_start = curr;
                        state = READ_SYMBOL;
                        break;
                }
                break;
            case READY:
                switch (str[curr]) {
                    case ' ': // lump whitespace together
                        break;
                    case '(':
                        // create the next backbone node
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->cdr = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        // create the new s-expression's initial node
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->car = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        state = NEW_SE;
                        break;
                    case ')':
                        if (stack == NULL) {
                            state = ERROR;
                            error_code = UNBAL_PAREN;
                        } else {
                            se_stack_pop(&stack);
                            while (stack != NULL && stack->se->cdr != NULL) {
                                se_stack_pop(&stack);
                            }
                            state = (stack == NULL) ? FINISH : READY;
                        }
                        break;
                    default:
                        symbol_start = curr;
                        se_stack_push(&stack, create_s_expr(NULL, NULL));
                        stack->next->se->cdr = install_list(temp_list_area, \
                                                            stack->se);
                        state = READ_SYMBOL;
                        break;
                }
                break;
            case READ_SYMBOL:
                switch (str[curr]) {
                    case ' ':
                        // update the current backbone node
                        stack->se->car = install_symbol_substring(st, \
                                                                  temp_symbol_table, \
                                                                  str, \
                                                                  symbol_start, \
                                                                  curr);
                        state = READY;
                        break;
                    case '(':
                        // update the current backbone node
                        stack->se->car = install_symbol_substring(st,
                                                                  temp_symbol_table,
                                                                  str,
                                                                  symbol_start,
                                                                  curr);
                        // create the next backbone node
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->cdr = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        // create the new s-expression's initial node
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->car = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        state = NEW_SE;
                        break;
                    case ')':
                        // update the current backbone node
                        stack->se->car = install_symbol_substring(st,
                                                                  temp_symbol_table,
                                                                  str,
                                                                  symbol_start,
                                                                  curr);
                        if (stack == NULL) {
                            state = ERROR;
                            error_code = UNBAL_PAREN;
                        } else {
                            se_stack_pop(&stack);
                            while (stack != NULL && stack->se->cdr != NULL) {
                                se_stack_pop(&stack);
                            }
                            state = (stack == NULL) ? FINISH : READY;
                        }
                        break;
                    default: // just keep reading
                        break;
                }
                break;
            case FINISH:
                switch (str[curr]) {
                    case ' ': // ignore trailing whitespace
                        break;
                    case '(':
                        state = ERROR;
                        error_code = TOO_MANY;
                        break;
                    case ')':
                        state = ERROR;
                        error_code = UNBAL_PAREN;
                        break;
                    default:
                        state = ERROR;
                        error_code = BARE_SYM;
                        break;
                }
                break;
            default:
                fprintf(stderr, "fatal error: parser in unknown state\n");
                exit(-1);
        }
        curr++;
    }
    if (stack != NULL) {
        state = ERROR;
        error_code = UNBAL_PAREN;
    }
    switch (error_code) {
        case NONE: // do nothing
            break;
        case UNBAL_PAREN:
            printf("parse error: unbalanced parentheses\n");
            break;
        case BARE_SYM:
            printf("parse error: symbol outside of parentheses\n");
            break;
        case EMPTY_PAREN:
            printf("parse error: '()' is not allowed\n");
            break;
        case TOO_MANY:
            printf("parse error: too many statements in input\n");
            break;
        default:
            printf("parse error: unrecognized code (%d)\n", error_code);
            break;
    }
    if (error_code == NONE) {
        merge_list_areas(la, temp_list_area);
        free(temp_list_area);
        merge_symbol_tables(st, temp_symbol_table);
        free(temp_symbol_table);
    } else {
        while (stack != NULL) {
            s_expr_storage* stack_temp = stack;
            stack = stack->next;
            // s-expressions pointed to on the stack are accessible from head
            free(stack_temp);
        }
        symbol_table_node* symbol_curr = temp_symbol_table->head;
        while (symbol_curr != NULL) {
            symbol_table_node* symbol_temp = symbol_curr;
            symbol_curr = symbol_curr->next;
            free(symbol_temp);
        }
        free(temp_symbol_table);
        s_expr_storage* list_curr = temp_list_area->head;
        while (list_curr != NULL) {
            s_expr_storage* list_temp = list_curr;
            list_curr = list_curr->next;
            delete_s_expr(list_temp->se);
            free(list_temp);
        }
        free(temp_list_area);
        delete_s_expr(head);
        head = NULL;
    }
    return head;
}

typedef enum {EVAL_ERROR_EXIT, \
              EVAL_ERROR_NULL_SEXPR, \
              EVAL_ERROR_NULL_CAR, \
              EVAL_ERROR_UNDEF_SYM, \
              EVAL_ERROR_UNDEF_TYPE, \
              EVAL_ERROR_UNDEF_BUILTIN, \
              EVAL_ERROR_FEW_ARGS, \
              EVAL_ERROR_MANY_ARGS, \
              EVAL_ERROR_BAD_ARG_TYPE, \
              EVAL_ERROR_NEED_NUM, \
              EVAL_ERROR_DIV_ZERO, \
              EVAL_ERROR_NONTERMINAL_ELSE} eval_error;

s_expr* sexpr_lookup(List_Area* la, typed_ptr* tp) {
    if (tp == NULL) {
        return NULL;
    }
    s_expr_storage* curr = la->head;
    while (curr != NULL) {
        if (curr->list_number == tp->ptr) {
            return curr->se;
        }
        curr = curr->next;
    }
    return NULL;
}

typed_ptr* value_lookup(Symbol_Table* st, typed_ptr* tp) {
    if (tp == NULL) {
        return NULL;
    }
    symbol_table_node* curr = st->head;
    while (curr != NULL) {
        if (curr->symbol_number == tp->ptr) {
            return create_typed_ptr(curr->type, curr->value);
        }
        curr = curr->next;
    }
    return NULL;
}

// forward declaration
typed_ptr* evaluate(s_expr* se, Symbol_Table* st, List_Area* la);

typed_ptr* apply_builtin_arithmetic(s_expr* se, Symbol_Table* st, List_Area* la) {
    builtin_code operation = symbol_lookup_index(st, se->car->ptr)->value;
    unsigned int result = (operation == BUILTIN_ADD || operation == BUILTIN_SUB) ? 0 : 1;
    if (se->cdr == NULL) {
        if (operation == BUILTIN_ADD || operation == BUILTIN_MUL) {
            // (+) or (*) -> additive or multiplicative identity
            return create_typed_ptr(TYPE_NUM, result);
        } else {
            return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
        }
    } else {
        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
        if (cdr_se == NULL) {
            return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NULL_SEXPR);
        }
        typed_ptr* curr_arg = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
        if (curr_arg->type == TYPE_ERROR) { // pass errors through
            return curr_arg;
        } else if (curr_arg->type != TYPE_NUM) { // non-numerics -> error
            free(curr_arg);
            return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NEED_NUM);
        } else if (operation == BUILTIN_DIV && \
                   curr_arg->ptr == 0 && \
                   cdr_se->cdr == NULL) {
            // (/ 0) will fail
            // but (/ 0 1) will succeed
            // as will (+ 0)
            free(curr_arg);
            return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_DIV_ZERO);
        } else {
            switch (operation) {
                case BUILTIN_ADD:
                    result += curr_arg->ptr;
                    break;
                case BUILTIN_MUL:
                    result *= curr_arg->ptr;
                    break;
                case BUILTIN_SUB:
                    if (cdr_se->cdr == NULL) {
                        //  (- 4) -> -4
                        result -= curr_arg->ptr;
                    } else {
                        // (- 4 1) -> 3
                        result = curr_arg->ptr;
                    }
                    break;
                case BUILTIN_DIV:
                    if (cdr_se->cdr == NULL) {
                        //  (/ 1) -> 1
                        result /= curr_arg->ptr;
                    } else {
                        // (/ 4 1) -> 4
                        result = curr_arg->ptr;
                    }
                    break;
                default:
                    free(curr_arg);
                    return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_UNDEF_BUILTIN);
            }
        }
        se = sexpr_lookup(la, se->cdr);
        if (se == NULL) {
            return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NULL_SEXPR);
        }
        while (se->cdr != NULL) {
            s_expr* cdr_se = sexpr_lookup(la, se->cdr);
            if (cdr_se == NULL) {
                return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NULL_SEXPR);
            }
            typed_ptr* curr_arg = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
            if (curr_arg->type == TYPE_ERROR) { // pass errors through
                return curr_arg;
            } else if (curr_arg->type != TYPE_NUM) { // non-numerics -> error
                free(curr_arg);
                return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NEED_NUM);
            }
            switch (operation) {
                case BUILTIN_ADD:
                    result += curr_arg->ptr;
                    break;
                case BUILTIN_MUL:
                    result *= curr_arg->ptr;
                    break;
                case BUILTIN_SUB:
                    result -= curr_arg->ptr;
                    break;
                case BUILTIN_DIV:
                    if (curr_arg->ptr == 0) {
                        free(curr_arg);
                        return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_DIV_ZERO);
                    } else {
                        result /= curr_arg->ptr;
                    }
                    break;
                default:
                    free(curr_arg);
                    return create_typed_ptr(TYPE_ERROR, EVAL_ERROR_UNDEF_BUILTIN);
            }
            se = cdr_se;
            free(curr_arg);
        }
        return create_typed_ptr(TYPE_NUM, result);
    }
}

typed_ptr* apply_number_comparison(s_expr* se, Symbol_Table* st, List_Area* la) {
    builtin_code comparison = symbol_lookup_index(st, se->car->ptr)->value;
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    typed_ptr* result = NULL;
    if (cdr_se == NULL || cdr_se->cdr == NULL) {
        result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
    } else {
        typed_ptr* eval_arg = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
        if (eval_arg->type == TYPE_ERROR) {
            result = eval_arg;
        } else if (eval_arg->type != TYPE_NUM) {
            free(eval_arg);
            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE);
        } else {
            unsigned int last_num = eval_arg->ptr;
            cdr_se = sexpr_lookup(la, cdr_se->cdr);
            result = create_typed_ptr(TYPE_BOOL, 1);
            while (cdr_se != NULL) {
                free(eval_arg);
                eval_arg = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                if (eval_arg->type == TYPE_ERROR) {
                    free(result);
                    result = eval_arg;
                    break;
                } else if (eval_arg->type != TYPE_NUM) {
                    free(eval_arg);
                    free(result);
                    result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE);
                    break;
                } else {
                    bool intermediate_truth;
                    switch (comparison) {
                        case BUILTIN_NUMBEREQ:
                            intermediate_truth = last_num == eval_arg->ptr;
                            break;
                        case BUILTIN_NUMBERGT:
                            intermediate_truth = last_num > eval_arg->ptr;
                            break;
                        case BUILTIN_NUMBERLT:
                            intermediate_truth = last_num < eval_arg->ptr;
                            break;
                        case BUILTIN_NUMBERGE:
                            intermediate_truth = last_num >= eval_arg->ptr;
                            break;
                        case BUILTIN_NUMBERLE:
                            intermediate_truth = last_num <= eval_arg->ptr;
                            break;
                        default:
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_UNDEF_BUILTIN);
                            break;
                    }
                    result->ptr = result->ptr && intermediate_truth;
                }
                cdr_se = sexpr_lookup(la, cdr_se->cdr);
            }
        }
    }
    return result;
}

bool is_false(typed_ptr* tp) {
    return (tp->type == TYPE_BOOL && tp->ptr == 0);
}

typed_ptr* evaluate(s_expr* se, Symbol_Table* st, List_Area* la) {
    typed_ptr* result = NULL;
    if (se == NULL) {
        result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NULL_SEXPR);
    } else if (se->car == NULL) {
        result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NULL_CAR);
    } else {
        switch (se->car->type) {
            case TYPE_UNDEF:
                result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_UNDEF_SYM);
                break;
            case TYPE_ERROR: // future: add info for traceback
                result = se->car;
                break;
            case TYPE_BUILTIN: {
                builtin_code builtin = symbol_lookup_index(st, se->car->ptr)->value;
                switch (builtin) {
                    case BUILTIN_ADD: //    -|
                    case BUILTIN_MUL: //    -|
                    case BUILTIN_SUB: //    -|
                    case BUILTIN_DIV: //     v
                        result = apply_builtin_arithmetic(se, st, la);
                        break;
                    case BUILTIN_SETQ: {
                        // this special form must have exactly 2 arguments
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        s_expr* cddr_se = sexpr_lookup(la, (cdr_se == NULL) ? NULL : cdr_se->cdr);
                        if (cdr_se == NULL || cddr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cddr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            unsigned int symbol_idx = cdr_se->car->ptr;
                            typed_ptr* eval_arg2 = evaluate(create_s_expr(cddr_se->car, NULL), st, la);
                            result = install_symbol(st, \
                                                    strdup(symbol_lookup_index(st, symbol_idx)->symbol), \
                                                    eval_arg2->type, \
                                                    eval_arg2->ptr);
                            free(eval_arg2);
                        }
                        break;
                    }
                    case BUILTIN_EXIT:
                        result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_EXIT);
                        break;
                    case BUILTIN_CONS: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        s_expr* cddr_se = sexpr_lookup(la, cdr_se->cdr);
                        if (cdr_se == NULL || cddr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cddr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* new_car = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            typed_ptr* new_cdr = evaluate(create_s_expr(cddr_se->car, NULL), st, la);
                            result = install_list(la, create_s_expr(new_car, new_cdr));
                        }
                        break;
                    }
                    case BUILTIN_CAR: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* eval_arg1 = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (eval_arg1->type != TYPE_SEXPR || eval_arg1->ptr == EMPTY_LIST_IDX) {
                                result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE);
                            } else {
                                typed_ptr* arg1_car = sexpr_lookup(la, eval_arg1)->car;
                                result = create_typed_ptr(arg1_car->type, arg1_car->ptr);
                            }
                            free(eval_arg1);
                        }
                        break;
                    }
                    case BUILTIN_CDR: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* eval_arg1 = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (eval_arg1->type != TYPE_SEXPR || eval_arg1->ptr == EMPTY_LIST_IDX) {
                                result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_BAD_ARG_TYPE);
                            } else {
                                typed_ptr* arg1_cdr = sexpr_lookup(la, eval_arg1)->cdr; // only diff from BUILTIN_CAR
                                result = create_typed_ptr(arg1_cdr->type, arg1_cdr->ptr);
                            }
                            free(eval_arg1);
                        }
                        break;
                    }
                    case BUILTIN_LIST: {
                        if (se->cdr == NULL) {
                            result = create_typed_ptr(TYPE_SEXPR, EMPTY_LIST_IDX);
                        } else {
                            s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                            typed_ptr* new_car = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            s_expr* last_cons_cell = NULL;
                            s_expr* curr_cons_cell = create_s_expr(new_car, \
                                                                   create_typed_ptr(TYPE_SEXPR, EMPTY_LIST_IDX));
                            result = install_list(la, curr_cons_cell);
                            while (cdr_se->cdr != NULL) {
                                cdr_se = sexpr_lookup(la, cdr_se->cdr);
                                new_car = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                                last_cons_cell = curr_cons_cell;
                                curr_cons_cell = create_s_expr(new_car, last_cons_cell->cdr);
                                last_cons_cell->cdr = install_list(la, curr_cons_cell);
                            }
                        }
                        break;
                    }
                    case BUILTIN_AND: {
                        typed_ptr* last_arg_eval = create_typed_ptr(TYPE_BOOL, 1);
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        while (cdr_se != NULL) {
                            free(last_arg_eval);
                            last_arg_eval = evaluate(cdr_se, st, la);
                            if (is_false(last_arg_eval)) {
                                break;
                            }
                            cdr_se = sexpr_lookup(la, cdr_se->cdr);
                        }
                        result = last_arg_eval;
                        break;
                    }
                    case BUILTIN_OR: {
                        typed_ptr* last_arg_eval = create_typed_ptr(TYPE_BOOL, 0);
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        while (cdr_se != NULL) {
                            free(last_arg_eval);
                            last_arg_eval = evaluate(cdr_se, st, la);
                            if (!is_false(last_arg_eval)) {
                                break;
                            }
                            cdr_se = sexpr_lookup(la, cdr_se->cdr);
                        }
                        result = last_arg_eval;
                        break;
                    }
                    case BUILTIN_NOT: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            result = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (result->type != TYPE_ERROR) {
                                if (is_false(result)) {
                                    result->ptr = 1;
                                } else {
                                    result->type = TYPE_BOOL;
                                    result->ptr = 0;
                                }
                            }
                        }
                        break;
                    }
                    case BUILTIN_COND: {
                        typed_ptr* eval_intermediate = create_typed_ptr(TYPE_VOID, 0);
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        bool pred_true = false;
                        s_expr* then_bodies = NULL;
                        while (cdr_se != NULL && pred_true == false) {
                            s_expr* cond_clause = sexpr_lookup(la, cdr_se->car);
                            typed_ptr* pred = cond_clause->car;
                            if (pred->type == TYPE_SYM && \
                                !strcmp(symbol_lookup_index(st, pred->ptr)->symbol, "else")) {
                                if (cdr_se->cdr != NULL) {
                                    free(eval_intermediate);
                                    eval_intermediate = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_NONTERMINAL_ELSE);
                                    then_bodies = NULL; // strictly unnecessary, but I'm feeling paranoid
                                } else {
                                    then_bodies = sexpr_lookup(la, cond_clause->cdr);
                                }
                                pred_true = true;
                            } else {
                                free(eval_intermediate);
                                eval_intermediate = evaluate(create_s_expr(pred, NULL), st, la);
                                if (eval_intermediate->type == TYPE_ERROR) {
                                    then_bodies = NULL;
                                    pred_true = true;
                                } else if (!is_false(eval_intermediate)) {
                                    then_bodies = sexpr_lookup(la, cond_clause->cdr);
                                    pred_true = true;
                                }
                            }
                            cdr_se = sexpr_lookup(la, cdr_se->cdr);
                        }
                        // then evaluate the then-bodies and return the last one
                        // (or eval_pred if there are none)
                        while (then_bodies != NULL) {
                            free(eval_intermediate);
                            eval_intermediate = evaluate(create_s_expr(then_bodies->car, NULL), st, la);
                            then_bodies = sexpr_lookup(la, then_bodies->cdr);
                        }
                        result = eval_intermediate;
                        break;
                    }
                    case BUILTIN_PAIRPRED: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* eval_arg1 = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (eval_arg1->type == TYPE_ERROR) {
                                result = eval_arg1;
                            } else {
                                result = create_typed_ptr(TYPE_BOOL, eval_arg1->type == TYPE_SEXPR);
                                free(eval_arg1);
                            }
                        }
                        break;
                    }
                    case BUILTIN_LISTPRED: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* eval_arg1 = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (eval_arg1->type == TYPE_ERROR) {
                                result = eval_arg1;
                            } else {
                                result = create_typed_ptr(TYPE_BOOL, 0);
                                if (eval_arg1->type == TYPE_SEXPR) {
                                    if (eval_arg1->ptr == EMPTY_LIST_IDX) {
                                        result->ptr = 1;
                                    } else {
                                        s_expr* curr = sexpr_lookup(la, eval_arg1);
                                        while (curr != NULL) {
                                            if (curr->cdr->type != TYPE_SEXPR) {
                                                break;
                                            }
                                            printf("  node %p ", curr->car);
                                            printf("has car value %u & type %d ", curr->car->ptr, curr->car->type);
                                            printf("and cdr value %u & type %d\n", curr->cdr->ptr, curr->cdr->type);
                                            if (curr->cdr->ptr == EMPTY_LIST_IDX) {
                                                result->ptr = 1;
                                                break;
                                            }
                                            curr = sexpr_lookup(la, curr->cdr);
                                        }
                                    }
                                }
                                free(eval_arg1);
                            }
                        }
                        break;
                    }
                    case BUILTIN_NUMBERPRED: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* eval_arg1 = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (eval_arg1->type == TYPE_ERROR) {
                                result = eval_arg1;
                            } else {
                                result = create_typed_ptr(TYPE_BOOL, eval_arg1->type == TYPE_NUM);
                                free(eval_arg1);
                            }
                        }
                        break;
                    }
                    case BUILTIN_BOOLPRED: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* eval_arg1 = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (eval_arg1->type == TYPE_ERROR) {
                                result = eval_arg1;
                            } else {
                                result = create_typed_ptr(TYPE_BOOL, eval_arg1->type == TYPE_BOOL);
                                free(eval_arg1);
                            }
                        }
                        break;
                    }
                    case BUILTIN_VOIDPRED: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* eval_arg1 = evaluate(create_s_expr(cdr_se->car, NULL), st, la);
                            if (eval_arg1->type == TYPE_ERROR) {
                                result = eval_arg1;
                            } else {
                                result = create_typed_ptr(TYPE_BOOL, eval_arg1->type == TYPE_VOID);
                                free(eval_arg1);
                            }
                        }
                        break;

                    }
                    case BUILTIN_NUMBEREQ: //    -|
                    case BUILTIN_NUMBERGT: //    -|
                    case BUILTIN_NUMBERLT: //    -|
                    case BUILTIN_NUMBERGE: //    -|
                    case BUILTIN_NUMBERLE: //    -V
                        result = apply_number_comparison(se, st, la);
                        break;
                    default:
                        result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_UNDEF_BUILTIN);
                        break;
                }
                break;
            }
            case TYPE_NUM:
                result = create_typed_ptr(se->car->type, se->car->ptr);
                break;
            case TYPE_SEXPR:
                result = evaluate(sexpr_lookup(la, se->car), st, la);
                break;
            case TYPE_SYM:
                result = value_lookup(st, se->car);
                break;
            case TYPE_BOOL:
                result = create_typed_ptr(se->car->type, se->car->ptr);
                break;
            case TYPE_VOID:
                result = create_typed_ptr(TYPE_VOID, 0);
                break;
            default:
                result = create_typed_ptr(TYPE_ERROR, EVAL_ERROR_UNDEF_TYPE);
                break;
        }
    }
    return result;
}

int main() {
    bool exit = 0;
    char input[BUF_SIZE];
    Symbol_Table* symbol_table = create_symbol_table(0);
    setup_symbol_table(symbol_table);
    List_Area* list_area = create_list_area(0);
    setup_list_area(list_area);
    while (!exit) {
        get_input(PROMPT, input, BUF_SIZE);
        s_expr* input_s_expr = parse(input, symbol_table, list_area);
        if (input_s_expr == NULL) { // parse error
            continue;
        }
        typed_ptr* result = evaluate(input_s_expr, symbol_table, list_area);
        printf("result: %u (type %d)\n", result->ptr, result->type);
        if (result->type == TYPE_ERROR && result->ptr == EVAL_ERROR_EXIT) {
            exit = true;
        }
    }
    printf("exiting...\n");
    return 0;
}

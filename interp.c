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

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
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

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_error(unsigned int err_code) {
    return create_typed_ptr(TYPE_ERROR, err_code);
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* copy_typed_ptr(const typed_ptr* tp) {
    return create_typed_ptr(tp->type, tp->ptr);
}

typedef struct SYMBOL_TABLE_NODE {
    unsigned int symbol_number;
    char* symbol;
    type type;
    unsigned int value;
    struct SYMBOL_TABLE_NODE* next;
} sym_tab_node;

// The caller should ensure that the node's symbol number in the symbol table
//   will be unique.
// The string pointed to by name is now the symbol table's responsibility
//   to free. It also must be safe to free (i.e., it must be heap-allocated)
//   and nobody else should free it.
// The returned sym_tab_node is the caller's (i.e., the symbol table's)
//   responsibility to free.
sym_tab_node* create_st_node(unsigned int symbol_number, \
                             char* name, \
                             type type, \
                             unsigned int value) {
    sym_tab_node* new_node = malloc(sizeof(sym_tab_node));
    if (new_node == NULL) {
        fprintf(stderr, "fatal error: malloc failed in create_st_node()\n");
        exit(-1);
    }
    new_node->symbol_number = symbol_number;
    new_node->symbol = name;
    new_node->type = type;
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

typedef struct SYMBOL_TABLE {
    sym_tab_node* head;
    unsigned int length;
    unsigned int symbol_number_offset;
} Symbol_Table;

// The offset allows a temporary symbol table (used while parsing for easy
//   walkback if the parsing fails) to avoid symbol number collisions with the
//   real symbol table. This makes merging the two after a successful parse much
//   easier.
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

// The returned sym_tab_node should (usually) not be freed.
// If the given name does not match any symbol table entry, NULL is returned.
sym_tab_node* symbol_lookup_string(Symbol_Table* st, const char* name) {
    sym_tab_node* curr = st->head;
    while (curr != NULL) {
        if (!strcmp(curr->symbol, name)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// The returned sym_tab_node should (usually) not be freed.
// If the given index does not match any symbol table entry, NULL is returned.
sym_tab_node* symbol_lookup_index(Symbol_Table* st, unsigned int index) {
    sym_tab_node* curr = st->head;
    while (curr != NULL) {
        if (curr->symbol_number == index) {
            break;
        }
        curr = curr->next;
    }
    return curr;
}

// The string pointed to by name is now the symbol table's responsibility to
//   free. It also must be safe to free (i.e., it must be heap-allocated), and
//   nobody else should free it.
// If a symbol already exists in the given symbol table with that name, the type
//   and value associated with it will be updated, and the string pointed to by
//   this function's parameter "name" will be freed. Otherwise, the symbol is
//   installed into the symbol table.
// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to the symbol table or any other
//   object.
typed_ptr* install_symbol(Symbol_Table* st, \
                          char* name, \
                          type type, \
                          unsigned int value) {
    unsigned int sym_num = st->length + st->symbol_number_offset;
    sym_tab_node* found = symbol_lookup_string(st, name);
    if (found == NULL) {
        sym_tab_node* new_node = create_st_node(sym_num, name, type, value);
        new_node->next = st->head;
        st->head = new_node;
        st->length++;
    } else {
        free(name);
        found->type = type;
        found->value = value;
        sym_num = found->symbol_number;
    }
    return create_typed_ptr(TYPE_SYM, sym_num);
}

// All considerations attendant upon the function "install_symbol()" above apply
//   here.
// This is a convenience function for use in initial symbol table setup.
void blind_install_symbol(Symbol_Table* st, \
                          char* symbol, \
                          type type, \
                          unsigned int value) {
    typed_ptr* tp = install_symbol(st, symbol, type, value);
    free(tp);
    return;
}

// The string returned is a valid null-terminated C string.
// The string returned is the caller's responsibility to free.
char* substring(char* str, unsigned int start, unsigned int end) {
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

void print_symbol_table(Symbol_Table* st) {
    sym_tab_node* curr = st->head;
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
              BUILTIN_DEFINE, \
              BUILTIN_SETVAR, \
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
    blind_install_symbol(st, "define", TYPE_BUILTIN, BUILTIN_DEFINE);
    blind_install_symbol(st, "set!", TYPE_BUILTIN, BUILTIN_SETVAR);
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

// The s-expression returned is the caller's responsibility to free.
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

// The s-expression is only shallow-deleted - this function will not follow
// the car or cdr pointers recursively.
// Obviously the s-expression and its car and cdr pointers must be safe to free.
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

typedef struct LIST_AREA {
    s_expr_storage* head;
    unsigned int length;
    unsigned int offset;
} List_Area;

// As for the symbol table, the offset allows the creation of a temporary list
//   area (as in parse(), below) which can be easily merged into the real list
//   area upon a successful parse.
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

// The s-expression to be installed is now the list area's responsibility to
//   free. It must be safe to free.
// No attempt is made to check for duplicate s-expressions already present in
//   the list area.
// The typed_ptr returned is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to the list area or any other objects.
typed_ptr* install_list(List_Area* la, s_expr* new_se) {
    unsigned int num = la->length + la->offset;
    s_expr_storage* new_ses = create_s_expr_storage(num, new_se);
    new_ses->next = la->head;
    la->head = new_ses;
    la->length++;
    return create_typed_ptr(TYPE_SEXPR, num);
}

// This is a bit of kludge to allow the installation of the empty list into the
//   list area during setup.
// No guarantee is made that the index "idx" will be unique in the list area.
void set_list_area_member(List_Area* la, unsigned int idx, s_expr* new_se) {
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

// This stack is used in to keep track of open s-expressions during parsing.
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

// This stack is used in to keep track of open s-expressions during parsing.
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

// Determines whether a string represents a number (rather than a symbol).
// Currently only recognizes nonnegative integers.
bool string_is_number(const char str[]) {
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

// A convenience function for use in parse().
// If there is a symbol whose name matches the specified substring in either
//   the symbol table or the temporary symbol table, a pointer to that symbol is
//   returned. If not, the appropriate symbol is installed in the temporary
//   symbol table, with type TYPE_UNDEF.
// In all cases, the returned typed_ptr is the caller's responsibility to free;
//   it is always safe to free without harm to either symbol table or any other
//   object.
typed_ptr* install_symbol_substring(Symbol_Table* st, \
                                    Symbol_Table* temp_st, \
                                    char str[], \
                                    unsigned int start, \
                                    unsigned int end) {
    char* name = substring(str, start, end);
    if (string_is_number(name)) {
        unsigned int value = atoi(name);
        free(name);
        return create_typed_ptr(TYPE_NUM, value);
    } else {
        sym_tab_node* found = symbol_lookup_string(st, name);
        if (found == NULL) {
            found = symbol_lookup_string(temp_st, name);
            if (found == NULL) {
                return install_symbol(temp_st, name, TYPE_UNDEF, 0);
            }
        }
        free(name);
        return create_typed_ptr(TYPE_SYM, found->symbol_number);
    }
}

// The returned s-expression (usually) shouldn't be freed.
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

sym_tab_node* lookup_builtin(Symbol_Table* st, builtin_code bc) {
    sym_tab_node* curr = st->head;
    while (curr != NULL) {
        if (curr->type == TYPE_BUILTIN && curr->value == bc) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

typedef enum {PARSE_ERROR_NONE, \
              PARSE_ERROR_UNBAL_PAREN, \
              PARSE_ERROR_BARE_SYM, \
              PARSE_ERROR_EMPTY_PAREN, \
              PARSE_ERROR_TOO_MANY, \
              // ^  parsing errors above     ^
              // v  evaluation errors below  v
              EVAL_ERROR_EXIT, \
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
              EVAL_ERROR_NONTERMINAL_ELSE, \
              EVAL_ERROR_CAR_NOT_CALLABLE, \
              EVAL_ERROR_NOT_ID} interpreter_error;

// Primary method to walk nested s-expressions.
// The s-expression returned (usually) shouldn't be freed.
// If the given typed_ptr does not point to a valid list area entry, or if it is
//   NULL, NULL is returned.
s_expr* sexpr_lookup(List_Area* la, const typed_ptr* tp) {
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

// Primary method to look up symbol values.
// The typed_ptr returned is the caller's responsibility to free; it may be
//   (shallow) freed without harm to the symbol table or any other object.
// If the given typed_ptr does not point to a valid symbol table entry, or if
//   it is NULL, NULL is returned.
typed_ptr* value_lookup(Symbol_Table* st, typed_ptr* tp) {
    if (tp == NULL) {
        return NULL;
    }
    sym_tab_node* curr = st->head;
    while (curr != NULL) {
        if (curr->symbol_number == tp->ptr) {
            if (curr->type == TYPE_UNDEF) {
                return create_error(EVAL_ERROR_UNDEF_SYM);
            } else {
                return create_typed_ptr(curr->type, curr->value);
            }
        }
        curr = curr->next;
    }
    return NULL;
}

void print_error(const typed_ptr* tp) {
    if (tp->ptr != EVAL_ERROR_EXIT) {
        printf("error: ");
    }
    switch (tp->ptr) {
        case PARSE_ERROR_NONE:
            printf("none (this indicates an interpreter problem)");
            break;
        case PARSE_ERROR_UNBAL_PAREN:
            printf("parsing: unbalanced parentheses");
            break;
        case PARSE_ERROR_BARE_SYM:
            printf("parsing: bare symbol outside s-expression");
            break;
        case PARSE_ERROR_EMPTY_PAREN:
            printf("parsing: empty parentheses (no procedure expression)");
            break;
        case PARSE_ERROR_TOO_MANY:
            printf("parsing: too many s-expressions (only one allowed)");
            break;
        case EVAL_ERROR_EXIT:
            break; // exit is handled in the REPL
        case EVAL_ERROR_NULL_SEXPR:
            printf("evaluation: an s-expression was NULL");
            break;
        case EVAL_ERROR_NULL_CAR:
            printf("evaluation: the car of an s-expression was NULL");
            break;
        case EVAL_ERROR_UNDEF_SYM:
            printf("evaluation: undefined symbol");
            break;
        case EVAL_ERROR_UNDEF_TYPE:
            printf("evaluation: undefined type");
            break;
        case EVAL_ERROR_UNDEF_BUILTIN:
            printf("evaluation: undefined built-in function or special form");
            break;
        case EVAL_ERROR_FEW_ARGS:
            printf("evaluation: too few arguments to function or special form");
            break;
        case EVAL_ERROR_MANY_ARGS:
            printf("evaluation: too many arguments to function or special form");
            break;
        case EVAL_ERROR_BAD_ARG_TYPE:
            printf("evaluation: argument to function or special form had the wrong type");
            break;
        case EVAL_ERROR_NEED_NUM:
            printf("evaluation: function or special form takes only number arguments");
            break;
        case EVAL_ERROR_DIV_ZERO:
            printf("evaluation: cannot divide by zero");
            break;
        case EVAL_ERROR_NONTERMINAL_ELSE:
            printf("'else' must be terminal clause in 'cond' special form");
            break;
        case EVAL_ERROR_CAR_NOT_CALLABLE:
            printf("evaluation: car of s-expression was not a callable");
            break;
        case EVAL_ERROR_NOT_ID:
            printf("evaluation: first argument to 'set!' must be an identifier");
            break;
        default:
            printf("unknown error: error code %u", tp->ptr);
            break;
    }
    return;
}

// forward declaration
void print_result(const typed_ptr* tp, Symbol_Table* st, List_Area* la);

void print_s_expression(const s_expr* se, Symbol_Table* st, List_Area* la) {
    printf("'(");
    while (se->car != NULL) { // which would indicate the empty list
        print_result(se->car, st, la);
        if (se->cdr != NULL && se->cdr->type == TYPE_SEXPR) { // list
            se = sexpr_lookup(la, se->cdr);
            if (se->car != NULL) {
                printf(" ");
            }
        } else if (se->cdr != NULL) { // pair
            printf(" . ");
            print_result(se->cdr, st, la);
            break;
        } else {
            printf("error: NULL cdr in s-expression");
            break;
        }
    }
    printf(")");
    return;
}

void print_result(const typed_ptr* tp, Symbol_Table* st, List_Area* la) {
    switch (tp->type) {
        case TYPE_UNDEF:
            printf("undefined symbol");
            break;
        case TYPE_ERROR:
            print_error(tp);
            break;
        case TYPE_NUM:
            printf("%u", tp->ptr);
            break;
        case TYPE_SEXPR:
            print_s_expression(sexpr_lookup(la, tp), st, la);
            break;
        case TYPE_SYM:
            printf("'%s", symbol_lookup_index(st, tp->ptr)->symbol);
            break;
        case TYPE_BUILTIN:
            printf("#<procedure:%s>", lookup_builtin(st, tp->ptr)->symbol);
            break;
        case TYPE_BOOL:
            printf("%s", (tp->ptr == 0) ? "#f" : "#t");
            break;
        case TYPE_VOID:
            break; // print nothing
        default:
            printf("unrecognized type: %d", tp->type);
            break;
    }
    return;
}

// Merges the second list area into the first; the second pointer remains valid.
// Makes no attempt to guard against list number collisions.
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

// Merges the second symbol table into the first; the second pointer remains
//   valid.
// Makes no attempt to guard against name or symbol number collisions.
void merge_symbol_tables(Symbol_Table* first, Symbol_Table* second) {
    if (first->head == NULL) {
        first->head = second->head;
    } else {
        sym_tab_node* curr = first->head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = second->head;
    }
    first->length += second->length;
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
s_expr* parse(char str[], Symbol_Table* st, List_Area* la) {
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
    Symbol_Table* temp_symbol_table = create_symbol_table(st->length);
    List_Area* temp_list_area = create_list_area(la->length);
    s_expr* head = NULL;
    typed_ptr* new_tp = NULL;
    while (str[curr] && state != PARSE_ERROR) {
        switch (state) {
            case PARSE_START:
                switch (str[curr]) {
                    case ' ': // ignore leading whitespace
                        break;
                    case '(':
                        head = create_s_expr(NULL, NULL);
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
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->car = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        break;
                    case ')':
                        state = PARSE_ERROR;
                        error_code = PARSE_ERROR_EMPTY_PAREN;
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
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->cdr = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        // create the new s-expression's initial node
                        new_s_expr = create_s_expr(NULL, NULL);
                        stack->se->car = install_list(temp_list_area, \
                                                      new_s_expr);
                        se_stack_push(&stack, new_s_expr);
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        if (stack == NULL) {
                            state = PARSE_ERROR;
                            error_code = PARSE_ERROR_UNBAL_PAREN;
                        } else {
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
                        se_stack_push(&stack, create_s_expr(NULL, NULL));
                        stack->next->se->cdr = install_list(temp_list_area, \
                                                            stack->se);
                        state = PARSE_READ_SYMBOL;
                        break;
                }
                break;
            case PARSE_READ_SYMBOL:
                switch (str[curr]) {
                    case ' ':
                        // update the current backbone node
                        new_tp = install_symbol_substring(st, \
                                                          temp_symbol_table, \
                                                          str, \
                                                          symbol_start, \
                                                          curr);
                        stack->se->car = new_tp;
                        state = PARSE_READY;
                        break;
                    case '(':
                        // update the current backbone node
                        new_tp = install_symbol_substring(st, \
                                                          temp_symbol_table, \
                                                          str, \
                                                          symbol_start, \
                                                          curr);
                        stack->se->car = new_tp;
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
                        state = PARSE_NEW_SEXPR;
                        break;
                    case ')':
                        // update the current backbone node
                        new_tp = install_symbol_substring(st, \
                                                          temp_symbol_table, \
                                                          str, \
                                                          symbol_start, \
                                                          curr);
                        stack->se->car = new_tp;
                        if (stack == NULL) {
                            state = PARSE_ERROR;
                            error_code = PARSE_ERROR_UNBAL_PAREN;
                        } else {
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
    if (stack != NULL) {
        state = PARSE_ERROR;
        error_code = PARSE_ERROR_UNBAL_PAREN;
    }
    if (error_code == PARSE_ERROR_NONE) {
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
        sym_tab_node* symbol_curr = temp_symbol_table->head;
        while (symbol_curr != NULL) {
            sym_tab_node* symbol_temp = symbol_curr;
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
        head = create_s_expr(create_error(error_code), NULL);
    }
    return head;
}

// forward declaration
typed_ptr* evaluate(const s_expr* se, Symbol_Table* st, List_Area* la);

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_xxx | xxx in {ADD, MUL, SUB, DIV}}.
// BUILTIN_ADD and BUILTIN_MUL take any number of arguments.
// BUILTIN_SUB and BUILTIN_DIV take at least 1 argument.
// All arguments are expected to (evaluate to) be numbers.
// Every argument to the built-in function is evaluated.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting number (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_arithmetic(const s_expr* se, Symbol_Table* st, List_Area* la) {
    builtin_code operation = se->car->ptr;
    typed_ptr* result = create_typed_ptr(TYPE_NUM, \
                                         (operation == BUILTIN_ADD || \
                                          operation == BUILTIN_SUB) ? 0 : 1);
    if (se->cdr == NULL) {
        if (operation == BUILTIN_SUB || operation == BUILTIN_DIV) {
            free(result);
            result = create_error(EVAL_ERROR_FEW_ARGS);
        }
        // else (+ or *) -> additive or multiplicative identity
    } else {
        if (se->cdr == NULL) {
            free(result);
            result = create_error(EVAL_ERROR_NULL_SEXPR);
        } else {
            s_expr* cdr_se = sexpr_lookup(la, se->cdr);
            typed_ptr* curr_arg = evaluate(cdr_se, st, la);
            if (curr_arg->type == TYPE_ERROR) { // pass errors through
                free(result);
                result = curr_arg;
            } else if (curr_arg->type != TYPE_NUM) { // non-numerics -> error
                free(curr_arg);
                free(result);
                result = create_error(EVAL_ERROR_NEED_NUM);
            } else if (operation == BUILTIN_DIV && \
                       curr_arg->ptr == 0 && \
                       cdr_se->cdr == NULL) {
                // (/ 0) will fail
                // but (/ 0 1) will succeed
                // as will (+ 0)
                free(curr_arg);
                free(result);
                result = create_error(EVAL_ERROR_DIV_ZERO);
            } else {
                switch (operation) {
                    case BUILTIN_ADD:
                        result->ptr += curr_arg->ptr;
                        break;
                    case BUILTIN_MUL:
                        result->ptr *= curr_arg->ptr;
                        break;
                    case BUILTIN_SUB:
                        if (cdr_se->cdr == NULL) {
                            //  (- 4) -> -4
                            result->ptr -= curr_arg->ptr;
                        } else {
                            // (- 4 1) -> 3
                            result->ptr = curr_arg->ptr;
                        }
                        break;
                    case BUILTIN_DIV:
                        if (cdr_se->cdr == NULL) {
                            //  (/ 1) -> 1
                            result->ptr /= curr_arg->ptr;
                        } else {
                            // (/ 4 1) -> 4
                            result->ptr = curr_arg->ptr;
                        }
                        break;
                    default:
                        free(curr_arg);
                        free(result);
                        result = create_error(EVAL_ERROR_UNDEF_BUILTIN);
                        break;
                }
            }
            if (result->type != TYPE_ERROR) {
                se = sexpr_lookup(la, se->cdr);
                if (se == NULL) {
                    free(result);
                    result = create_error(EVAL_ERROR_NULL_SEXPR);
                }
                while (se->cdr != NULL) {
                    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                    if (cdr_se == NULL) {
                        free(result);
                        result = create_error(EVAL_ERROR_NULL_SEXPR);
                    } else {
                        typed_ptr* curr_arg = evaluate(cdr_se, st, la);
                        if (curr_arg->type == TYPE_ERROR) { // pass errors on
                            free(result);
                            result = curr_arg;
                        } else if (curr_arg->type != TYPE_NUM) {
                            free(curr_arg);
                            free(result);
                            result = create_error(EVAL_ERROR_NEED_NUM);
                        } else {
                            switch (operation) {
                                case BUILTIN_ADD:
                                    result->ptr += curr_arg->ptr;
                                    break;
                                case BUILTIN_MUL:
                                    result->ptr *= curr_arg->ptr;
                                    break;
                                case BUILTIN_SUB:
                                    result->ptr -= curr_arg->ptr;
                                    break;
                                case BUILTIN_DIV:
                                    if (curr_arg->ptr == 0) {
                                        free(curr_arg);
                                        free(result);
                                        result = create_error(EVAL_ERROR_DIV_ZERO);
                                    } else {
                                        result->ptr /= curr_arg->ptr;
                                    }
                                    break;
                                default:
                                    free(curr_arg);
                                    free(result);
                                    result = create_error(EVAL_ERROR_UNDEF_BUILTIN);
                                    break;
                            }
                        }
                        if (result->type == TYPE_ERROR) {
                            break;
                        }
                        se = cdr_se;
                        free(curr_arg);
                    }
                }
            }
        }
    }
    return result;
}

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_NUMBERxx | xx in {EQ, GT, LT, GE, LE}}.
// These functions take at least 1 argument.
// All arguments are expected to (evaluate to) be numbers.
// Every argument to the built-in function is evaluated.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the (boolean) truth value of the expression (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_comparison(const s_expr* se, Symbol_Table* st, List_Area* la) {
    builtin_code comparison = se->car->ptr;
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    typed_ptr* result = NULL;
    if (cdr_se == NULL || cdr_se->cdr == NULL) {
        result = create_error(EVAL_ERROR_FEW_ARGS);
    } else {
        typed_ptr* eval_arg = evaluate(cdr_se, st, la);
        if (eval_arg->type == TYPE_ERROR) {
            result = eval_arg;
        } else if (eval_arg->type != TYPE_NUM) {
            free(eval_arg);
            result = create_error(EVAL_ERROR_BAD_ARG_TYPE);
        } else {
            unsigned int last_num = eval_arg->ptr;
            cdr_se = sexpr_lookup(la, cdr_se->cdr);
            result = create_typed_ptr(TYPE_BOOL, 1);
            while (cdr_se != NULL) {
                free(eval_arg);
                eval_arg = evaluate(cdr_se, st, la);
                if (eval_arg->type == TYPE_ERROR) {
                    free(result);
                    result = eval_arg;
                    break;
                } else if (eval_arg->type != TYPE_NUM) {
                    free(eval_arg);
                    free(result);
                    result = create_error(EVAL_ERROR_BAD_ARG_TYPE);
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
                            result = create_error(EVAL_ERROR_UNDEF_BUILTIN);
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

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_DEFINE.
// This special form takes exactly two arguments.
// The first argument is expected to be a symbol name, and not evaluated.
// There is no restriction on the type of the second argument; it is evaluated.
// The first argument's symbol table entry's value is set to the result of
//   evaluating the second argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting symbol (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_define(const s_expr* se, Symbol_Table* st, List_Area* la) {
    typed_ptr* result = NULL;
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    s_expr* cddr_se = sexpr_lookup(la, cdr_se->cdr);
    if (cdr_se == NULL || cddr_se == NULL) {
        result = create_error(EVAL_ERROR_FEW_ARGS);
    } else if (cddr_se->cdr != NULL) {
        result = create_error(EVAL_ERROR_MANY_ARGS);
    } else if (cdr_se->car->type != TYPE_SYM) {
        result = create_error(EVAL_ERROR_NOT_ID);
    } else {
        sym_tab_node* symbol_entry = symbol_lookup_index(st, cdr_se->car->ptr);
        typed_ptr* eval_arg2 = evaluate(cddr_se, st, la);
        if (eval_arg2->type == TYPE_ERROR) {
            result = eval_arg2;
        } else {
            char* name = strdup(symbol_entry->symbol);
            result = install_symbol(st, name, eval_arg2->type, eval_arg2->ptr);
            free(result);
            result = create_typed_ptr(TYPE_VOID, 0);
            free(eval_arg2);
        }
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_SETVAR.
// This special form takes exactly two arguments.
// The first argument is expected to be a symbol name, and not evaluated. It
//   must not be TYPE_UNDEF. (It must be defined first.)
// There is no restriction on the type of the second argument; it is evaluated.
// The first argument's symbol table entry's value is set to the result of
//   evaluating the second argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting symbol (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_set_variable(const s_expr* se, \
                             Symbol_Table* st, \
                             List_Area* la) {
    typed_ptr* result = NULL;
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    s_expr* cddr_se = sexpr_lookup(la, cdr_se->cdr);
    if (cdr_se == NULL || cddr_se == NULL) {
        result = create_error(EVAL_ERROR_FEW_ARGS);
    } else if (cddr_se->cdr != NULL) {
        result = create_error(EVAL_ERROR_MANY_ARGS);
    } else if (cdr_se->car->type != TYPE_SYM) {
        result = create_error(EVAL_ERROR_NOT_ID);
    } else {
        sym_tab_node* symbol_entry = symbol_lookup_index(st, cdr_se->car->ptr);
        if (symbol_entry->type == TYPE_UNDEF) {
            result = create_error(EVAL_ERROR_UNDEF_SYM);
        } else {
            typed_ptr* eval_arg2 = evaluate(cddr_se, st, la);
            if (eval_arg2->type == TYPE_ERROR) {
                result = eval_arg2;
            } else {
                char* name = strdup(symbol_entry->symbol);
                result = install_symbol(st, name, eval_arg2->type, eval_arg2->ptr);
                free(result);
                result = create_typed_ptr(TYPE_VOID, 0);
                free(eval_arg2);
            }
        }
    }
    return result;
}

// Evaluates an s-expression whose car is in the set
//   {BUILTIN_xxx | xxx in {CAR, CDR}}.
// This function takes exactly one argument, which is evaluated.
// The first argument is expected to be TYPE_SEXPR.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting object (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_car_cdr(const s_expr* se, Symbol_Table* st, List_Area* la) {
    typed_ptr* result = NULL;
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    if (cdr_se == NULL) {
        result = create_error(EVAL_ERROR_FEW_ARGS);
    } else if (cdr_se->cdr != NULL) {
        result = create_error(EVAL_ERROR_MANY_ARGS);
    } else {
        typed_ptr* eval_arg1 = evaluate(cdr_se, st, la);
        if (eval_arg1->type != TYPE_SEXPR || \
            eval_arg1->ptr == EMPTY_LIST_IDX) {
            result = create_error(EVAL_ERROR_BAD_ARG_TYPE);
        } else {
            s_expr* arg1_se = sexpr_lookup(la, eval_arg1);
            if (symbol_lookup_index(st, se->car->ptr)->value == BUILTIN_CAR) {
                result = copy_typed_ptr(arg1_se->car);
            } else {
                result = copy_typed_ptr(arg1_se->cdr);
            }
        }
        free(eval_arg1);
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in function 
//   BUILTIN_LISTPRED.
// This function takes one argument, which is evaluated.
// There is no restriction on the type of the argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the (boolean) truth value of the predicate (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_list_pred(const s_expr* se, Symbol_Table* st, List_Area* la) {
    typed_ptr* result = NULL;
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    if (cdr_se == NULL) {
        result = create_error(EVAL_ERROR_FEW_ARGS);
    } else if (cdr_se->cdr != NULL) {
        result = create_error(EVAL_ERROR_MANY_ARGS);
    } else {
        typed_ptr* eval_arg1 = evaluate(cdr_se, st, la);
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
    return result;
}

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_xxxxPRED | xxxx in {PAIR, BOOL, NUM, VOID}}.
// For convenience, the (C, not Lisp) function takes the type to be tested
//   against as an additional parameter.
// This function takes one argument, which is evaluated.
// There is no restriction on the type of the argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the (boolean) truth value of the predicate (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_atom_pred(const s_expr* se, \
                          Symbol_Table* st, \
                          List_Area* la, \
                          type t) {
    typed_ptr* result = NULL;
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    if (cdr_se == NULL) {
        result = create_error(EVAL_ERROR_FEW_ARGS);
    } else if (cdr_se->cdr != NULL) {
        result = create_error(EVAL_ERROR_MANY_ARGS);
    } else {
        typed_ptr* eval_arg1 = evaluate(cdr_se, st, la);
        if (eval_arg1->type == TYPE_ERROR) {
            result = eval_arg1;
        } else {
            result = create_typed_ptr(TYPE_BOOL, eval_arg1->type == t);
            free(eval_arg1);
        }
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in function BUILTIN_LIST.
// This function takes any number of arguments, which are all evaluated.
// There is no restriction on the type of the arguments.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting s-expression (if it succeeded).
// If no arguments are provided, the resulting typed_ptr points to the empty
//   list.
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_list_construction(const s_expr* se, \
                                  Symbol_Table* st, \
                                  List_Area* la) {
    typed_ptr* result = NULL;
    if (se->cdr == NULL) {
        result = create_typed_ptr(TYPE_SEXPR, EMPTY_LIST_IDX);
    } else {
        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
        typed_ptr* new_car = evaluate(cdr_se, st, la);
        s_expr* last_node = NULL;
        typed_ptr* new_cdr = create_typed_ptr(TYPE_SEXPR, EMPTY_LIST_IDX);
        s_expr* curr_node = create_s_expr(new_car, new_cdr);
        result = install_list(la, curr_node);
        while (cdr_se->cdr != NULL) {
            cdr_se = sexpr_lookup(la, cdr_se->cdr);
            new_car = evaluate(cdr_se, st, la);
            last_node = curr_node;
            curr_node = create_s_expr(new_car, last_node->cdr);
            last_node->cdr = install_list(la, curr_node);
        }
    }
    return result;
}

bool is_false_literal(typed_ptr* tp) {
    return (tp->type == TYPE_BOOL && tp->ptr == 0);
}

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_COND.
// This special form takes any number of arguments.
// The arguments must take the form (predicate [then-body...]).
// The predicates of the arguments are evaluated until one evaluates to anything
//   other than a boolean false value. After this, no predicates are evaluated.
// For an argument whose predicate evaluates to non-false, its then-bodies are
//   evaluated in order. If there are no then-bodies, the return value is the
//   result of the evaluation of the predicate.
// One special predicate, named "else", is allowed, which is a fall-through
//   case. It may only appear as the predicate of the last argument.
// Returns a typed_ptr containing an error code (if any evaluation failed) or
//   the result of the last then-body evaluation.
// If no arguments are provided, or if no argument's predicate evaluates to
//   not-false, the resulting s-expression is a void value.
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_cond(const s_expr* se, Symbol_Table* st, List_Area* la) {
    typed_ptr* eval_interm = create_typed_ptr(TYPE_VOID, 0);
    s_expr* cdr_se = sexpr_lookup(la, se->cdr);
    bool pred_true = false;
    s_expr* then_bodies = NULL;
    while (cdr_se != NULL && pred_true == false) {
        s_expr* cond_clause = sexpr_lookup(la, cdr_se->car);
        typed_ptr* pred = cond_clause->car;
        if (pred->type == TYPE_SYM && \
            !strcmp(symbol_lookup_index(st, pred->ptr)->symbol, "else")) {
            if (cdr_se->cdr != NULL) {
                free(eval_interm);
                eval_interm = create_error(EVAL_ERROR_NONTERMINAL_ELSE);
                then_bodies = NULL;
            } else {
                then_bodies = sexpr_lookup(la, cond_clause->cdr);
            }
            pred_true = true;
        } else {
            free(eval_interm);
            s_expr* pred_se = sexpr_lookup(la, pred);
            eval_interm = evaluate(pred_se, st, la);
            if (eval_interm->type == TYPE_ERROR) {
                then_bodies = NULL;
                pred_true = true;
            } else if (!is_false_literal(eval_interm)) {
                then_bodies = sexpr_lookup(la, cond_clause->cdr);
                pred_true = true;
            }
        }
        cdr_se = sexpr_lookup(la, cdr_se->cdr);
    }
    // then evaluate the then-bodies and return the last one
    // (or eval_pred if there are none)
    while (then_bodies != NULL) {
        free(eval_interm);
        eval_interm = evaluate(then_bodies, st, la);
        then_bodies = sexpr_lookup(la, then_bodies->cdr);
    }
    return eval_interm;
}

// Evaluates an s-expression of any kind within the context of the provided
//   symbol table and list area.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the result (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* evaluate(const s_expr* se, Symbol_Table* st, List_Area* la) {
    typed_ptr* result = NULL;
    if (se == NULL) {
        result = create_error(EVAL_ERROR_NULL_SEXPR);
    } else if (se->car == NULL) {
        result = create_error(EVAL_ERROR_NULL_CAR);
    } else {
        switch (se->car->type) {
            case TYPE_UNDEF:
                result = create_error(EVAL_ERROR_UNDEF_SYM);
                break;
            case TYPE_ERROR: // future: add info for traceback
                result = se->car;
                break;
            case TYPE_BUILTIN: {
                switch (se->car->ptr) {
                    case BUILTIN_ADD: //    -|
                    case BUILTIN_MUL: //    -|
                    case BUILTIN_SUB: //    -|
                    case BUILTIN_DIV: //     v
                        result = eval_arithmetic(se, st, la);
                        break;
                    case BUILTIN_DEFINE:
                        result = eval_define(se, st, la);
                        break;
                    case BUILTIN_SETVAR:
                        result = eval_set_variable(se, st, la);
                        break;
                    case BUILTIN_EXIT:
                        result = create_error(EVAL_ERROR_EXIT);
                        break;
                    case BUILTIN_CONS: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        s_expr* cddr_se = sexpr_lookup(la, cdr_se->cdr);
                        if (cdr_se == NULL || cddr_se == NULL) {
                            result = create_error(EVAL_ERROR_FEW_ARGS);
                        } else if (cddr_se->cdr != NULL) {
                            result = create_error(EVAL_ERROR_MANY_ARGS);
                        } else {
                            typed_ptr* new_car = evaluate(cdr_se, st, la);
                            typed_ptr* new_cdr = evaluate(cddr_se, st, la);
                            result = install_list(la, create_s_expr(new_car, \
                                                                    new_cdr));
                        }
                        break;
                    }
                    case BUILTIN_CAR: //    -|
                    case BUILTIN_CDR: //    -V
                        result = eval_car_cdr(se, st, la);
                        break;
                    case BUILTIN_LIST:
                        result = eval_list_construction(se, st, la);
                        break;
                    case BUILTIN_AND: {
                        typed_ptr* eval_prev = create_typed_ptr(TYPE_BOOL, 1);
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        while (cdr_se != NULL) {
                            free(eval_prev);
                            eval_prev = evaluate(cdr_se, st, la);
                            if (is_false_literal(eval_prev)) {
                                break;
                            }
                            cdr_se = sexpr_lookup(la, cdr_se->cdr);
                        }
                        result = eval_prev;
                        break;
                    }
                    case BUILTIN_OR: {
                        typed_ptr* eval_prev = create_typed_ptr(TYPE_BOOL, 0);
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        while (cdr_se != NULL) {
                            free(eval_prev);
                            eval_prev = evaluate(cdr_se, st, la);
                            if (!is_false_literal(eval_prev)) {
                                break;
                            }
                            cdr_se = sexpr_lookup(la, cdr_se->cdr);
                        }
                        result = eval_prev;
                        break;
                    }
                    case BUILTIN_NOT: {
                        s_expr* cdr_se = sexpr_lookup(la, se->cdr);
                        if (cdr_se == NULL) {
                            result = create_error(EVAL_ERROR_FEW_ARGS);
                        } else if (cdr_se->cdr != NULL) {
                            result = create_error(EVAL_ERROR_MANY_ARGS);
                        } else {
                            result = evaluate(cdr_se, st, la);
                            if (result->type != TYPE_ERROR) {
                                if (is_false_literal(result)) {
                                    result->ptr = 1;
                                } else {
                                    result->type = TYPE_BOOL;
                                    result->ptr = 0;
                                }
                            }
                        }
                        break;
                    }
                    case BUILTIN_COND:
                        result = eval_cond(se, st, la);
                        break;
                    case BUILTIN_LISTPRED:
                        result = eval_list_pred(se, st, la);
                        break;
                    case BUILTIN_PAIRPRED:
                        result = eval_atom_pred(se, st, la, TYPE_SEXPR);
                        break;
                    case BUILTIN_NUMBERPRED:
                        result = eval_atom_pred(se, st, la, TYPE_NUM);
                        break;
                    case BUILTIN_BOOLPRED:
                        result = eval_atom_pred(se, st, la, TYPE_BOOL);
                        break;
                    case BUILTIN_VOIDPRED:
                        result = eval_atom_pred(se, st, la, TYPE_VOID);
                        break;
                    case BUILTIN_NUMBEREQ: //    -|
                    case BUILTIN_NUMBERGT: //    -|
                    case BUILTIN_NUMBERLT: //    -|
                    case BUILTIN_NUMBERGE: //    -|
                    case BUILTIN_NUMBERLE: //    -V
                        result = eval_comparison(se, st, la);
                        break;
                    default:
                        result = create_error(EVAL_ERROR_UNDEF_BUILTIN);
                        break;
                }
                break;
            }
            case TYPE_NUM:
                result = create_typed_ptr(se->car->type, se->car->ptr);
                break;
            case TYPE_SEXPR: {
                s_expr* se_to_eval = sexpr_lookup(la, se->car);
                s_expr* caar_se = create_s_expr(se_to_eval->car, NULL);
                typed_ptr* fn = evaluate(caar_se, st, la);
                free(caar_se);
                if (fn->type == TYPE_ERROR) {
                    result = fn;
                } else {
                    if (fn->type == TYPE_BUILTIN) { // will need to change later
                        s_expr* temp_se = create_s_expr(fn, se_to_eval->cdr);
                        result = evaluate(temp_se, st, la);
                        free(temp_se);
                    } else {
                        result = create_error(EVAL_ERROR_CAR_NOT_CALLABLE);
                    }
                    free(fn);
                }
                break;
            }
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
                result = create_error(EVAL_ERROR_UNDEF_TYPE);
                break;
        }
    }
    return result;
}

// Sets up the environment for execution, and runs the REPL.
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
        if (input_s_expr->car->type == TYPE_ERROR) {
            print_error(input_s_expr->car);
            printf("\n");
            delete_s_expr(input_s_expr);
        } else {
            typed_ptr* input_tp = install_list(list_area, input_s_expr);
            s_expr* super_se = create_s_expr(input_tp, NULL);
            typed_ptr* result = evaluate(super_se, symbol_table, list_area);
            print_result(result, symbol_table, list_area);
            printf("\n");
            if (result->type == TYPE_ERROR && result->ptr == EVAL_ERROR_EXIT) {
                exit = true;
            }
            free(result);
            free(input_tp);
            free(super_se);
        }
    }
    printf("exiting...\n");
    return 0;
}

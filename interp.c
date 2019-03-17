#include "interp.h"

#define EMPTY_LIST_IDX 0

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_typed_ptr(type type, union_idx_se ptr) {
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
typed_ptr* create_atom_tp(type type, unsigned int idx) {
    return create_typed_ptr(type, (union_idx_se){.idx=idx});
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_sexpr_tp(s_expr* se_ptr) {
    return create_typed_ptr(TYPE_SEXPR, (union_idx_se){.se_ptr=se_ptr});
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_error(unsigned int err_code) {
    return create_atom_tp(TYPE_ERROR, err_code);
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* copy_typed_ptr(const typed_ptr* tp) {
    return create_typed_ptr(tp->type, tp->ptr);
}

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
                             union_idx_se value) {
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

// The s-expression returned is the caller's responsibility to free.
s_expr* create_empty_s_expr() {
    return create_s_expr(NULL, NULL);
}

s_expr* copy_s_expr(const s_expr* se) {
    if (se == NULL) {
        return NULL;
    }
    s_expr* new_se = create_empty_s_expr();
    s_expr* curr_se = new_se;
    while (!is_empty_list(se)) {
        curr_se->car = copy_typed_ptr(se->car);
        if (curr_se->car->type == TYPE_SEXPR) {
            curr_se->car->ptr.se_ptr = copy_s_expr(curr_se->car->ptr.se_ptr);
        } // otherwise it's atomic and a copy of the typed_ptr is enough
        curr_se->cdr = copy_typed_ptr(se->cdr);
        if (curr_se->cdr->type == TYPE_SEXPR) {
            curr_se->cdr->ptr.se_ptr = create_empty_s_expr();
            curr_se = sexpr_next(curr_se);
            se = sexpr_next(se);
        } else { // se is a pair, so we're done
            break;
        }
    }
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

void delete_st_node_list(sym_tab_node* stn) {
    while (stn != NULL) {
        sym_tab_node* next = stn->next;
        free(stn->symbol);
        free(stn);
        stn = next;
    }
    return;
}

// The arg list and closure environment are presumed to be unique to this
//   function table node, but the body points to an s-expression that is
//   represented elsewhere (and so cannot be safely freed using this pointer).
fun_tab_node* create_ft_node(unsigned int function_number, \
                             sym_tab_node* arg_list, \
                             environment* closure_env, \
                             typed_ptr* body) {
    fun_tab_node* new_ftn = malloc(sizeof(fun_tab_node));
    if (new_ftn == NULL) {
        fprintf(stderr, "malloc failed in create_ft_node()\n");
        exit(-1);
    }
    new_ftn->function_number = function_number;
    new_ftn->arg_list = arg_list;
    new_ftn->closure_env = closure_env;
    new_ftn->body = body;
    new_ftn->next = NULL;
    return new_ftn;
}

function_table* create_function_table(unsigned int offset) {
    function_table* new_ft = malloc(sizeof(function_table));
    if (new_ft == NULL) {
        fprintf(stderr, "malloc failed in create_function_table()\n");
        exit(-1);
    }
    new_ft->head = NULL;
    new_ft->length = 0;
    new_ft->offset = offset;
    return new_ft;
}

// The fun_tab_node returned shouldn't (usually) be freed.
// tp is assumed to be an atomic typed_ptr.
fun_tab_node* function_lookup_index(environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_USER_FN) {
        return NULL;
    }
    fun_tab_node* curr = env->function_table->head;
    while (curr != NULL) {
        if (curr->function_number == tp->ptr.idx) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

environment* create_environment(unsigned int st_offset, \
                                unsigned int ft_offset) {
    environment* new_env = malloc(sizeof(environment));
    if (new_env == NULL) {
        fprintf(stderr, "malloc failed in create_environment()\n");
        exit(-1);
    }
    new_env->symbol_table = create_symbol_table(st_offset);
    new_env->function_table = create_function_table(ft_offset);
    return new_env;
}

// The new environment contains a deep copy of the old environment's symbol
//   table. Symbol table numbers are preserved (so pointers into it are still
//   valid).
// However, because the function table is currently only added to (not modified
//   or deleted from), its pointer is simply copied over).
// To destroy this new environment, delete_env_shared_ft() below should be used,
//   to ensure it's done safely.
environment* copy_environment(environment* env) {
    environment* new_env = create_environment(0, 0);
    sym_tab_node* curr_stn = env->symbol_table->head;
    while (curr_stn != NULL) {
        sym_tab_node* new_stn = create_st_node(curr_stn->symbol_number, \
                                               strdup(curr_stn->symbol), \
                                               curr_stn->type, \
                                               curr_stn->value);
        if (new_stn->type == TYPE_SEXPR) {
            new_stn->value.se_ptr = copy_s_expr(new_stn->value.se_ptr);
        }
        new_stn->next = new_env->symbol_table->head;
        new_env->symbol_table->head = new_stn;
        curr_stn = curr_stn->next;
    }
    // risky, but currently ok, because the function table is only added to,
    // not modified or deleted from
    free(new_env->function_table);
    new_env->function_table = env->function_table;
    return new_env;
}

// Safely deletes an environment that shares a function table with other
//   environments.
void delete_env_shared_ft(environment* env) {
    sym_tab_node* curr = env->symbol_table->head;
    while (curr != NULL) {
        sym_tab_node* next = curr->next;
        free(curr->symbol);
        if (curr->type == TYPE_SEXPR) {
            delete_se_recursive(curr->value.se_ptr, true);
        }
        free(curr);
        curr = next;
    }
    free(env->symbol_table);
    free(env);
    return;
}

// Fully deletes an environment.
void delete_env_full(environment* env) {
    sym_tab_node* curr_stn = env->symbol_table->head;
    while (curr_stn != NULL) {
        sym_tab_node* next_stn = curr_stn->next;
        free(curr_stn->symbol);
        if (curr_stn->type == TYPE_SEXPR) {
            delete_se_recursive(curr_stn->value.se_ptr, true);
        }
        free(curr_stn);
        curr_stn = next_stn;
    }
    free(env->symbol_table);
    fun_tab_node* curr_ftn = env->function_table->head;
    while (curr_ftn != NULL) {
        fun_tab_node* next_ftn = curr_ftn->next;
        // free argument list
        sym_tab_node* curr_arg_stn = curr_ftn->arg_list;
        while (curr_arg_stn != NULL) {
            sym_tab_node* next_arg_stn = curr_arg_stn->next;
            free(curr_arg_stn->symbol);
            free(curr_arg_stn);
            curr_arg_stn = next_arg_stn;
        }
        // free closure environment
        delete_env_shared_ft(curr_ftn->closure_env);
        // free body s-expression
        if (curr_ftn->body->type == TYPE_SEXPR) {
            delete_se_recursive(curr_ftn->body->ptr.se_ptr, true);
        }
        free(curr_ftn->body);
        free(curr_ftn);
        curr_ftn = next_ftn;
    }
    free(env->function_table);
    free(env);
    return;
}

// The returned sym_tab_node should (usually) not be freed.
// If the given name does not match any symbol table entry, NULL is returned.
sym_tab_node* symbol_lookup_string(environment* env, const char* name) {
    sym_tab_node* curr = env->symbol_table->head;
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
sym_tab_node* symbol_lookup_index(environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_SYM) {
        return NULL;
    }
    sym_tab_node* curr = env->symbol_table->head;
    while (curr != NULL) {
        if (curr->symbol_number == tp->ptr.idx) {
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
typed_ptr* install_symbol(environment* env, \
                          char* name, \
                          type type, \
                          union_idx_se value) {
    unsigned int sym_num = env->symbol_table->length + \
                           env->symbol_table->symbol_number_offset;
    sym_tab_node* found = symbol_lookup_string(env, name);
    if (found == NULL) {
        sym_tab_node* new_node = create_st_node(sym_num, name, type, value);
        new_node->next = env->symbol_table->head;
        env->symbol_table->head = new_node;
        env->symbol_table->length++;
    } else {
        free(name);
        found->type = type;
        found->value = value;
        sym_num = found->symbol_number;
    }
    return create_atom_tp(TYPE_SYM, sym_num);
}

// All considerations attendant upon the function "install_symbol()" above apply
//   here.
// This is a convenience function for use in initial symbol table setup.
void blind_install_symbol_atom(environment* env, \
                               char* symbol, \
                               type type, \
                               unsigned int value) {
    typed_ptr* tp = install_symbol(env, \
                                   symbol, \
                                   type, \
                                   (union_idx_se){.idx=value});
    free(tp);
    return;
}

// All considerations attendant upon the function "install_symbol()" above apply
//   here.
// This is a convenience function for use in initial symbol table setup.
void blind_install_symbol_sexpr(environment* env, \
                                char* symbol, \
                                type type, \
                                s_expr* value) {
    typed_ptr* tp = install_symbol(env, \
                                   symbol, \
                                   type, \
                                   (union_idx_se){.se_ptr=value});
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

void setup_symbol_table(environment* env) {
    blind_install_symbol_atom(env, strdup("NULL_SENTINEL"), TYPE_UNDEF, 0);
    blind_install_symbol_atom(env, strdup("+"), TYPE_BUILTIN, BUILTIN_ADD);
    blind_install_symbol_atom(env, strdup("*"), TYPE_BUILTIN, BUILTIN_MUL);
    blind_install_symbol_atom(env, strdup("-"), TYPE_BUILTIN, BUILTIN_SUB);
    blind_install_symbol_atom(env, strdup("/"), TYPE_BUILTIN, BUILTIN_DIV);
    blind_install_symbol_atom(env, strdup("define"), TYPE_BUILTIN, BUILTIN_DEFINE);
    blind_install_symbol_atom(env, strdup("set!"), TYPE_BUILTIN, BUILTIN_SETVAR);
    blind_install_symbol_atom(env, strdup("exit"), TYPE_BUILTIN, BUILTIN_EXIT);
    blind_install_symbol_atom(env, strdup("cons"), TYPE_BUILTIN, BUILTIN_CONS);
    blind_install_symbol_atom(env, strdup("car"), TYPE_BUILTIN, BUILTIN_CAR);
    blind_install_symbol_atom(env, strdup("cdr"), TYPE_BUILTIN, BUILTIN_CDR);
    blind_install_symbol_atom(env, strdup("and"), TYPE_BUILTIN, BUILTIN_AND);
    blind_install_symbol_atom(env, strdup("or"), TYPE_BUILTIN, BUILTIN_OR);
    blind_install_symbol_atom(env, strdup("not"), TYPE_BUILTIN, BUILTIN_NOT);
    blind_install_symbol_atom(env, strdup("cond"), TYPE_BUILTIN, BUILTIN_COND);
    blind_install_symbol_atom(env, strdup("list"), TYPE_BUILTIN, BUILTIN_LIST);
    blind_install_symbol_atom(env, strdup("pair?"), TYPE_BUILTIN, BUILTIN_PAIRPRED);
    blind_install_symbol_atom(env, strdup("list?"), TYPE_BUILTIN, BUILTIN_LISTPRED);
    blind_install_symbol_atom(env, strdup("number?"), TYPE_BUILTIN, BUILTIN_NUMBERPRED);
    blind_install_symbol_atom(env, strdup("boolean?"), TYPE_BUILTIN, BUILTIN_BOOLPRED);
    blind_install_symbol_atom(env, strdup("void?"), TYPE_BUILTIN, BUILTIN_VOIDPRED);
    blind_install_symbol_atom(env, strdup("="), TYPE_BUILTIN, BUILTIN_NUMBEREQ);
    blind_install_symbol_atom(env, strdup(">"), TYPE_BUILTIN, BUILTIN_NUMBERGT);
    blind_install_symbol_atom(env, strdup("<"), TYPE_BUILTIN, BUILTIN_NUMBERLT);
    blind_install_symbol_atom(env, strdup(">="), TYPE_BUILTIN, BUILTIN_NUMBERGE);
    blind_install_symbol_atom(env, strdup("<="), TYPE_BUILTIN, BUILTIN_NUMBERLE);
    blind_install_symbol_atom(env, strdup("lambda"), TYPE_BUILTIN, BUILTIN_LAMBDA);
    blind_install_symbol_sexpr(env, strdup("null"), TYPE_SEXPR, create_empty_s_expr());
    blind_install_symbol_atom(env, strdup("#t"), TYPE_BOOL, 1);
    blind_install_symbol_atom(env, strdup("#f"), TYPE_BOOL, 0);
    blind_install_symbol_atom(env, strdup("else"), TYPE_UNDEF, 0);
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

void setup_environment(environment* env) {
    setup_symbol_table(env);
    return;
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
typed_ptr* install_symbol_substring(environment* env, \
                                    environment* temp_env, \
                                    char str[], \
                                    unsigned int start, \
                                    unsigned int end) {
    char* name = substring(str, start, end);
    if (string_is_number(name)) {
        unsigned int value = atoi(name);
        free(name);
        return create_atom_tp(TYPE_NUM, value);
    } else {
        sym_tab_node* found = symbol_lookup_string(env, name);
        if (found == NULL) {
            found = symbol_lookup_string(temp_env, name);
            if (found == NULL) {
                return install_symbol(temp_env, \
                                      name, \
                                      TYPE_UNDEF, \
                                      (union_idx_se){.idx=0});
            }
        }
        free(name);
        return create_atom_tp(TYPE_SYM, found->symbol_number);
    }
}

sym_tab_node* builtin_lookup_index(environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_BUILTIN) {
        return NULL;
    }
    sym_tab_node* curr = env->symbol_table->head;
    while (curr != NULL) {
        if (curr->type == TYPE_BUILTIN && curr->value.idx == tp->ptr.idx) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

// Primary method to look up symbol values.
// The typed_ptr returned is the caller's responsibility to free; it may be
//   (shallow) freed without harm to the symbol table or any other object.
// If the given typed_ptr does not point to a valid symbol table entry, or if
//   it is NULL, NULL is returned.
typed_ptr* value_lookup_index(environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_SYM) {
        return NULL;
    }
    sym_tab_node* curr = env->symbol_table->head;
    while (curr != NULL) {
        if (curr->symbol_number == tp->ptr.idx) {
            if (curr->type == TYPE_UNDEF) {
                return create_error(EVAL_ERROR_UNDEF_SYM);
            } else if (curr->type == TYPE_SEXPR) {
                return create_sexpr_tp(copy_s_expr(curr->value.se_ptr));
            } else {
                return create_typed_ptr(curr->type, curr->value);
            }
        }
        curr = curr->next;
    }
    return NULL;
}

void print_error(const typed_ptr* tp) {
    if (tp->ptr.idx != EVAL_ERROR_EXIT) {
        printf("error: ");
    }
    switch (tp->ptr.idx) {
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
        case EVAL_ERROR_MALFORMED_SEXPR:
            printf("evaluation: NULL car or cdr in a non-empty s-expression");
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
        case EVAL_ERROR_MISSING_PROCEDURE:
            printf("evaluation: missing procedure expression; probably originally bare ()");
            break;
        case EVAL_ERROR_BAD_SYNTAX:
            printf("evaluation: bad syntax for a function or special form");
            break;
        case EVAL_ERROR_ILLEGAL_PAIR:
            printf("evaluation: cannot evaluate a pair");
            break;
        case EVAL_ERROR_EMPTY_ELSE:
            printf("evaluation: else-clause in cond must have then-body");
            break;
        case EVAL_ERROR_UNDEF_FUNCTION:
            printf("evaluation: undefined user function");
            break;
        default:
            printf("unknown error: error code %u", tp->ptr.idx);
            break;
    }
    return;
}

void print_s_expression(const s_expr* se, environment* env) {
    if (se == NULL) {
        typed_ptr* err = create_error(EVAL_ERROR_NULL_SEXPR);
        print_error(err);
        free(err);
        return;
    }
    printf("'(");
    while (!is_empty_list(se)) {
        print_result(se->car, env);
        if (se->cdr->type == TYPE_SEXPR) { // list
            se = se->cdr->ptr.se_ptr;
            if (!is_empty_list(se)) {
                printf(" ");
            }
        } else { // pair
            printf(" . ");
            print_result(se->cdr, env);
            break;
        }
    }
    printf(")");
    return;
}

void print_result(const typed_ptr* tp, environment* env) {
    switch (tp->type) {
        case TYPE_UNDEF:
            printf("undefined symbol");
            break;
        case TYPE_ERROR:
            print_error(tp);
            break;
        case TYPE_NUM:
            printf("%u", tp->ptr.idx);
            break;
        case TYPE_SEXPR:
            print_s_expression(tp->ptr.se_ptr, env);
            break;
        case TYPE_SYM:
            printf("'%s", symbol_lookup_index(env, tp)->symbol);
            break;
        case TYPE_BUILTIN:
            printf("#<procedure:%s>", builtin_lookup_index(env, tp)->symbol);
            break;
        case TYPE_BOOL:
            printf("%s", (tp->ptr.idx == 0) ? "#f" : "#t");
            break;
        case TYPE_VOID:
            break; // print nothing
        case TYPE_USER_FN:
            printf("#<procedure>");
            break;
        default:
            printf("unrecognized type: %d", tp->type);
            break;
    }
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
        delete_s_expr(head);
        head = create_s_expr(create_error(error_code), \
                             create_sexpr_tp(create_empty_s_expr()));
    }
    return head;
}

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
typed_ptr* eval_arithmetic(const s_expr* se, environment* env) {
    builtin_code op = se->car->ptr.idx;
    typed_ptr* result = NULL;
    int min_args = (op == BUILTIN_ADD || op == BUILTIN_MUL) ? 0 : 1;
    typed_ptr* args_tp = collect_args(se, env, min_args, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        unsigned int initial = (op == BUILTIN_ADD || op == BUILTIN_SUB) ? 0 : 1;
        result = create_atom_tp(TYPE_NUM, initial);
        s_expr* arg = args_tp->ptr.se_ptr;
        if ((op == BUILTIN_SUB || op == BUILTIN_DIV) && \
            !is_empty_list(sexpr_next(arg))) {
            result->ptr.idx = arg->car->ptr.idx;
            arg = sexpr_next(arg);
        }
        while (!is_empty_list(arg)) {
            if (arg->car->type != TYPE_NUM) {
                free(result);
                result = create_error(EVAL_ERROR_NEED_NUM);
                break;
            }
            switch (op) {
                case BUILTIN_ADD:
                    result->ptr.idx += arg->car->ptr.idx;
                    break;
                case BUILTIN_SUB:
                    result->ptr.idx -= arg->car->ptr.idx;
                    break;
                case BUILTIN_MUL:
                    result->ptr.idx *= arg->car->ptr.idx;
                    break;
                case BUILTIN_DIV:
                    if (arg->car->ptr.idx == 0) {
                        free(result);
                        result = create_error(EVAL_ERROR_DIV_ZERO);
                    } else {
                        result->ptr.idx /= arg->car->ptr.idx;
                    }
                    break;
                default:
                    free(result);
                    result = create_error(EVAL_ERROR_UNDEF_BUILTIN);
                    break;
            }
            if (result->type == TYPE_ERROR) {
                break;
            }
            arg = sexpr_next(arg);
        }
        delete_se_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
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
typed_ptr* eval_comparison(const s_expr* se, environment* env) {
    builtin_code op = se->car->ptr.idx;
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        s_expr* arg_se = args_tp->ptr.se_ptr;
        if (arg_se->car->type != TYPE_NUM) {
            result = create_error(EVAL_ERROR_NEED_NUM);
        } else {
            unsigned int truth = 1;
            unsigned int last_num = arg_se->car->ptr.idx;
            arg_se = sexpr_next(arg_se);
            while (!is_empty_list(arg_se) && truth == 1) {
                if (arg_se->car->type != TYPE_NUM) {
                    result = create_error(EVAL_ERROR_NEED_NUM);
                    break;
                }
                switch (op) {
                    case BUILTIN_NUMBEREQ:
                        truth = last_num == arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERGT:
                        truth = last_num > arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERLT:
                        truth = last_num < arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERGE:
                        truth = last_num >= arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERLE:
                        truth = last_num <= arg_se->car->ptr.idx;
                        break;
                    default:
                        result = create_error(EVAL_ERROR_UNDEF_BUILTIN);
                        break;
                }
                if (result != NULL) {
                    break;
                }
                last_num = arg_se->car->ptr.idx;
                arg_se = sexpr_next(arg_se);
            }
            if (result == NULL) {
                result = create_atom_tp(TYPE_BOOL, truth);
            } // otherwise it threw an error
        }
        delete_se_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
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
typed_ptr* eval_define(const s_expr* se, environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        if (arg->type == TYPE_SYM) {
            sym_tab_node* sym_entry = symbol_lookup_index(env, arg);
            if (sym_entry == NULL) {
                result = create_error(EVAL_ERROR_UNDEF_SYM);
            } else {
                arg = evaluate(sexpr_next(args_tp->ptr.se_ptr), env);
                if (arg->type == TYPE_ERROR) {
                    result = arg;
                } else {
                    char* name = strdup(sym_entry->symbol);
                    result = install_symbol(env, name, arg->type, arg->ptr);
                    free(arg);
                    free(result);
                    result = create_atom_tp(TYPE_VOID, 0);
                }
            }
        } else if (arg->type == TYPE_SEXPR) {
            if (is_empty_list(arg->ptr.se_ptr)) {
                result = create_error(EVAL_ERROR_BAD_SYNTAX);
            } else if (arg->ptr.se_ptr->car->type != TYPE_SYM) {
                result = create_error(EVAL_ERROR_NOT_ID);
            } else {
                typed_ptr* fn_sym = arg->ptr.se_ptr->car;
                sym_tab_node* sym_entry = symbol_lookup_index(env, fn_sym);
                if (sym_entry == NULL) {
                    result = create_error(EVAL_ERROR_UNDEF_SYM);
                } else {
                    // create a dummy (lambda arg-list body) s-expression
                    typed_ptr* arg_list = arg->ptr.se_ptr->cdr;
                    arg->ptr.se_ptr->cdr = create_sexpr_tp(create_empty_s_expr());
                    typed_ptr* fn_body = sexpr_next(args_tp->ptr.se_ptr)->car;
                    sexpr_next(args_tp->ptr.se_ptr)->car = NULL;
                    sym_tab_node* lambda_stn = symbol_lookup_string(env, "lambda");
                    typed_ptr* lambda = create_atom_tp(TYPE_SYM, \
                                                       lambda_stn->symbol_number);
                    s_expr* fn_body_se = create_s_expr(fn_body, create_sexpr_tp(create_empty_s_expr()));
                    s_expr* arg_list_se = create_s_expr(arg_list, \
                                                        create_sexpr_tp(fn_body_se));
                    s_expr* dummy_lambda = create_s_expr(lambda, \
                                                         create_sexpr_tp(arg_list_se));
                    typed_ptr* fn = eval_lambda(dummy_lambda, env);
                    delete_se_recursive(dummy_lambda, false);
                    if (fn->type == TYPE_ERROR) {
                        result = fn;
                    } else {
                        char* name = strdup(sym_entry->symbol);
                        blind_install_symbol_atom(env, name, fn->type, fn->ptr.idx);
                        result = create_typed_ptr(TYPE_VOID, (union_idx_se){.idx=0});
                    }
                }
            }
        } else {
            result = create_error(EVAL_ERROR_NOT_ID);
        }
        delete_se_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
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
typed_ptr* eval_set_variable(const s_expr* se, environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        if (arg->type != TYPE_SYM) {
            result = create_error(EVAL_ERROR_NOT_ID);
        } else {
            sym_tab_node* sym_entry = symbol_lookup_index(env, arg);
            if (sym_entry == NULL || sym_entry->type == TYPE_UNDEF) {
                result = create_error(EVAL_ERROR_UNDEF_SYM);
            } else {
                arg = evaluate(sexpr_next(args_tp->ptr.se_ptr), env);
                if (arg->type == TYPE_ERROR) {
                    result = arg;
                } else {
                    char* name = strdup(sym_entry->symbol);
                    result = install_symbol(env, name, arg->type, arg->ptr);
                    free(arg);
                    free(result);
                    result = create_atom_tp(TYPE_VOID, 0);
                }
            }
        }
        delete_se_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
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
typed_ptr* eval_car_cdr(const s_expr* se, environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        if (arg->type != TYPE_SEXPR || \
            is_empty_list(arg->ptr.se_ptr)) {
            result = create_error(EVAL_ERROR_BAD_ARG_TYPE);
        } else if (se->car->ptr.idx == BUILTIN_CAR) {
            result = arg->ptr.se_ptr->car;
            arg->ptr.se_ptr->car = NULL;
        } else {
            result = arg->ptr.se_ptr->cdr;
            arg->ptr.se_ptr->cdr = NULL;
        }
        delete_se_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
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
typed_ptr* eval_list_pred(const s_expr* se, environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        result = create_atom_tp(TYPE_BOOL, 1);
        if (arg->type != TYPE_SEXPR) {
            result->ptr.idx = 0;
        } else {
            s_expr* arg_se = arg->ptr.se_ptr;
            while (!is_empty_list(arg_se)) {
                if (arg_se->cdr->type != TYPE_SEXPR) {
                    result->ptr.idx = 0;
                    break;
                }
                arg_se = sexpr_next(arg_se);
            }
        }
        delete_se_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
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
typed_ptr* eval_atom_pred(const s_expr* se, environment* env, type t) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        result = create_atom_tp(TYPE_BOOL, (arg->type == t) ? 1 : 0);
        if (arg->type == TYPE_SEXPR && is_empty_list(arg->ptr.se_ptr)) {
            result->ptr.idx = 0;
        }
        delete_se_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
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
typed_ptr* eval_list_construction(const s_expr* se, environment* env) {
    return collect_args(se, env, 0, -1, true);
}

bool is_false_literal(const typed_ptr* tp) {
    return (tp->type == TYPE_BOOL && tp->ptr.idx == 0);
}

bool is_empty_list(const s_expr* se) {
    if (se == NULL) {
        printf("cannot determine if NULL se is empty list\n");
        exit(-1);
    } else {
        return (se->car == NULL && se->cdr == NULL);
    }
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
//   case. It may only appear as the predicate of the last argument. If "else"
//   appears, it must have at least one then-body.
// Returns a typed_ptr containing an error code (if any evaluation failed) or
//   the result of the last then-body evaluation.
// If no arguments are provided, or if no argument's predicate evaluates to
//   not-false, the resulting s-expression is a void value.
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_cond(const s_expr* se, environment* env) {
    typed_ptr* args_tp = collect_args(se, env, 0, -1, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    } else {
        typed_ptr* eval_interm = create_typed_ptr(TYPE_VOID, (union_idx_se){.idx=0});
        s_expr* arg_se = sexpr_next(se);
        if (is_empty_list(arg_se)) {
            delete_se_recursive(args_tp->ptr.se_ptr, false);
            free(args_tp);
            return eval_interm;
        }
        bool pred_true = false;
        s_expr* then_bodies = NULL;
        while (!is_empty_list(arg_se)) {
            if (arg_se->car->type != TYPE_SEXPR) {
                free(eval_interm);
                eval_interm = create_error(EVAL_ERROR_BAD_SYNTAX);
                break;
            }
            s_expr* cond_clause = arg_se->car->ptr.se_ptr;
            if (is_empty_list(cond_clause)) {
                free(eval_interm);
                eval_interm = create_error(EVAL_ERROR_BAD_SYNTAX);
                break;
            }
            if (cond_clause->car->type == TYPE_SYM && \
                cond_clause->car->ptr.idx == symbol_lookup_string(env, "else")->symbol_number) {
                s_expr* next_clause = sexpr_next(arg_se);
                if (!is_empty_list(next_clause)) {
                    free(eval_interm);
                    eval_interm = create_error(EVAL_ERROR_NONTERMINAL_ELSE);
                    break;
                }
                then_bodies = sexpr_next(cond_clause);
                if (is_empty_list(then_bodies)) {
                    free(eval_interm);
                    eval_interm = create_error(EVAL_ERROR_EMPTY_ELSE);
                    break;
                }
                pred_true = true;
            }
            free(eval_interm);
            eval_interm = evaluate(cond_clause, env);
            if (eval_interm->type == TYPE_ERROR) {
                break;
            } else if (!is_false_literal(eval_interm)) {
                pred_true = true;
                then_bodies = sexpr_next(cond_clause);
                break;
            }
            arg_se = sexpr_next(arg_se);
        }
        typed_ptr* result = NULL;
        if (!pred_true) { // no cond-clauses were true or we encountered an error
            if (eval_interm->type == TYPE_ERROR) {
                result = eval_interm;
            } else {
                free(eval_interm);
                result = create_typed_ptr(TYPE_VOID, (union_idx_se){.idx=0});
            }
        } else {
            while (!is_empty_list(then_bodies)) {
                free(eval_interm);
                eval_interm = evaluate(then_bodies, env);
                then_bodies = sexpr_next(then_bodies);
            }
            result = eval_interm;
        }
        delete_se_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
        return result;
    }
}

// At first, we will restrict user-defined functions to have a finite number of
//   parameters.
// tp is expected to be a pointer to an s-expression. If it's empty, NULL is
//   returned.
// If any element of the s-expression pointed to by tp is not a symbol, a
//   single sym_tab_node containing an error code is returned.
// Otherwise, a list of sym_tab_nodes is returned, one for each parameter
//   symbol discovered.
// In either case, these sym_tab_nodes are safe to free (and they are the
//   caller's responsibility to free).
sym_tab_node* collect_parameters(typed_ptr* tp, environment* env) {
    sym_tab_node* params = NULL;
    s_expr* se = tp->ptr.se_ptr;
    if (is_empty_list(se)) {
        return params;
    }
    if (se->car->type != TYPE_SYM) {
        params = create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=EVAL_ERROR_NOT_ID});
    } else {
        char* name = symbol_lookup_index(env, se->car)->symbol;
        params = create_st_node(0, strdup(name), TYPE_UNDEF, (union_idx_se){.idx=0});
        sym_tab_node* curr = params;
        se = sexpr_next(se);
        while (!is_empty_list(se)) {
            if (se->cdr == NULL || se->cdr->type != TYPE_SEXPR) {
                delete_st_node_list(params);
                params = create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=EVAL_ERROR_BAD_ARG_TYPE});
                break;
            }
            if (se->car == NULL || se->car->type != TYPE_SYM) {
                delete_st_node_list(params);
                params = create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=EVAL_ERROR_NOT_ID});
                break;
            }
            name = symbol_lookup_index(env, se->car)->symbol;
            curr->next = create_st_node(0, strdup(name), TYPE_UNDEF, (union_idx_se){.idx=0});
            curr = curr->next;
            se = sexpr_next(se);
        }
    }
    return params;
}

// The arg list and closure environment are now the (general) environment's
//   concern. The body pointed to by the typed pointer remains someone else's
//   problem, and won't be freed by the environment.
// The typed pointer returned is the caller's responsibility to free.
typed_ptr* install_function(environment* env, \
                            sym_tab_node* arg_list, \
                            environment* closure_env, \
                            typed_ptr* body) {
    unsigned int num = env->function_table->length;
    fun_tab_node* new_ftn = create_ft_node(num, arg_list, closure_env, body);
    new_ftn->next = env->function_table->head;
    env->function_table->head = new_ftn;
    env->function_table->length++;
    // and it's now also in closure_env, because the two environments share
    // list areas and function pointers
    return create_atom_tp(TYPE_USER_FN, num);
}

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_LAMBDA.
// This special form takes two arguments.
// The first argument must be a list (nested s-expression) of symbols, which
//   become the parameters of the lambda. Anything else returns an error. This
//   argument is not evaluated.
// The second argument may be anything, and is not evaluated, but stored as the
//   body of the lambda.
// The function installed in the environment, and its associated data, is now
//   the environment's responsibility.
// The typed pointer returned is the caller's responsibility to free, and can
//   safely be (shallow) freed.
typed_ptr* eval_lambda(const s_expr* se, environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* first = args_tp->ptr.se_ptr->car;
        typed_ptr* second = sexpr_next(args_tp->ptr.se_ptr)->car;
        if (first->type != TYPE_SEXPR) {
            result = create_error(EVAL_ERROR_BAD_ARG_TYPE);
        } else {
            sym_tab_node* params = collect_parameters(first, env);
            if (params != NULL && params->type == TYPE_ERROR) {
                result = create_error(params->value.idx);
                delete_st_node_list(params);
            } else {
                environment* closure_env = copy_environment(env);
                typed_ptr* body = copy_typed_ptr(second);
                result = install_function(env, params, closure_env, body);
            }
        }
        delete_se_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
    }
    return result;
}

// If the fun_tab_node's arg list is of different length than the s-expression
//   pointed to by the args typed pointer, an error is returned in the first
//   sym_tab_node.
// Otherwise, the parameters in the arg list are bound to the values produced
//   when evaluating the members of the args s-expression; any error during
//   evaluation aborts this process and is passed back out in the first
//   sym_tab_node.
// If no errors are encountered, the sym_tab_node list contains the bound
//   arguments.
// In all cases, the sym_tab_node list returned is the caller's responsibility
//   to free, and may be safely (shallow) freed.
sym_tab_node* bind_args(environment* env, fun_tab_node* ftn, typed_ptr* args) {
    if (ftn->arg_list == NULL && is_empty_list(args->ptr.se_ptr)) {
        return NULL;
    } else if (is_empty_list(args->ptr.se_ptr)) {
        return create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=EVAL_ERROR_FEW_ARGS});
    } else if (ftn->arg_list == NULL) {
        return create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=EVAL_ERROR_MANY_ARGS});
    } else {
        sym_tab_node* curr_param = ftn->arg_list;
        s_expr* arg_se = args->ptr.se_ptr;
        sym_tab_node* bound_args = NULL;
        bound_args = create_st_node(0, \
                                    strdup(curr_param->symbol), \
                                    arg_se->car->type, \
                                    arg_se->car->ptr);
        curr_param = curr_param->next;
        arg_se = sexpr_next(arg_se);
        while (!is_empty_list(arg_se)) {
            if (curr_param == NULL) {
                delete_st_node_list(bound_args);
                bound_args = create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=EVAL_ERROR_MANY_ARGS});
                break;
            }
            sym_tab_node* new_bound_arg = create_st_node(0, \
                                                         strdup(curr_param->symbol), \
                                                         arg_se->car->type, \
                                                         arg_se->car->ptr);
            new_bound_arg->next = bound_args;
            bound_args = new_bound_arg;
            curr_param = curr_param->next;
            arg_se = sexpr_next(arg_se);
        }
        if (curr_param != NULL) {
            delete_st_node_list(bound_args);
            bound_args = create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=EVAL_ERROR_FEW_ARGS});
        }
        return bound_args;
    }
}

// Reads a list of bound arguments into an environment, returning the result.
// The input environment is not modified.
// The returned environment is the caller's responsibility to delete, using
//   delete_env_shared_ft() below.
environment* make_eval_env(environment* env, sym_tab_node* bound_args) {
    environment* eval_env = copy_environment(env);
    sym_tab_node* curr_arg = bound_args;
    while (curr_arg != NULL) {
        sym_tab_node* found = symbol_lookup_string(eval_env, curr_arg->symbol);
        if (found == NULL) {
            fprintf(stderr, "parameter name not found - something is wrong\n");
            exit(-1);
        } else {
            found->type = curr_arg->type;
            found->value = curr_arg->value;
        }
        curr_arg = curr_arg->next;
    }
    return eval_env;
}

// Evaluates an s-expression whose car has type TYPE_USER_FN.
// The s-expression's cdr must contain the proper number of arguments for the
//   user function being invoked. Any mismatch, or error arising during
//   evaluation of any of the members of the cdr, returns an error.
// Returns a typed_ptr containing an error code (if any argument evaluation
//   failed, or if an error arising during evaluation of the function body) or
//   the result of evaluating the function body using the provided arguments.
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_user_function(const s_expr* se, environment* env) {
    typed_ptr* result = NULL;
    fun_tab_node* ftn = function_lookup_index(env, se->car);
    if (ftn == NULL) {
        return create_error(EVAL_ERROR_UNDEF_FUNCTION);
    }
    typed_ptr* args_tp = collect_args(se, env, 0, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        sym_tab_node* bound_args = bind_args(env, ftn, args_tp);
        if (bound_args != NULL && bound_args->type == TYPE_ERROR) {
            result = create_error(bound_args->value.idx);
        } else {
            environment* bound_env = make_eval_env(ftn->closure_env, bound_args);
            s_expr* super_se = create_s_expr(copy_typed_ptr(ftn->body), \
                                             create_sexpr_tp(create_empty_s_expr()));
            result = evaluate(super_se, bound_env);
            delete_se_recursive(super_se, false);
            delete_env_shared_ft(bound_env);
        }
        delete_st_node_list(bound_args);
        delete_se_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

bool is_pair(const s_expr* se) {
    if (se == NULL) {
        printf("cannot determine pair-ness of NULL s-expression\n");
        exit(-1);
    }
    if (se->cdr == NULL) {
        printf("cannot determine pair-ness if cdr is NULL\n");
        exit(-1);
    }
    return se->cdr->type != TYPE_SEXPR;
}

s_expr* sexpr_next(const s_expr* se) {
    return se->cdr->ptr.se_ptr;
}

void delete_se_recursive(s_expr* se, bool delete_sexpr_cars) {
    s_expr* curr = se;
    while (curr != NULL) {
        if (delete_sexpr_cars && \
            curr->car != NULL && \
            curr->car->type == TYPE_SEXPR) {
            delete_se_recursive(curr->car->ptr.se_ptr, true);
        }
        free(curr->car);
        if (curr->cdr != NULL && curr->cdr->type == TYPE_SEXPR) {
            se = sexpr_next(curr);
            free(curr->cdr);
        } else {
            se = NULL;
            free(curr->cdr);
        }
        free(curr);
        curr = se;
    }
    return;
}

typed_ptr* collect_args(const s_expr* se, \
                        environment* env, \
                        int min_args, \
                        int max_args, \
                        bool evaluate_all_args) {
    int seen = 0;
    if (is_pair(se)) {
        return create_error(EVAL_ERROR_ILLEGAL_PAIR);
    }
    s_expr* curr = sexpr_next(se);
    s_expr* arg_head = create_empty_s_expr();
    s_expr* arg_tail = arg_head;
    typed_ptr* err = NULL;
    while (!is_empty_list(curr)) {
        if (is_pair(curr)) {
            err = create_error(EVAL_ERROR_ILLEGAL_PAIR);
            break;
        }
        seen++;
        if (max_args >= 0 && seen > max_args) {
            err = create_error(EVAL_ERROR_MANY_ARGS);
            break;
        }
        arg_tail->car = copy_typed_ptr(curr->car);
        arg_tail->cdr = create_sexpr_tp(create_empty_s_expr());
        if (evaluate_all_args && \
            arg_tail->car->type != TYPE_BUILTIN && \
            arg_tail->car->type != TYPE_USER_FN) {
            typed_ptr* temp = arg_tail->car;
            arg_tail->car = evaluate(arg_tail, env);
            free(temp);
        }
        if (arg_tail->car->type == TYPE_ERROR) {
            err = copy_typed_ptr(arg_tail->car);
            break;
        }
        arg_tail = sexpr_next(arg_tail);
        curr = sexpr_next(curr);
    }
    if (err == NULL && seen < min_args) {
        err = create_error(EVAL_ERROR_FEW_ARGS);
    }
    if (err != NULL) {
        delete_se_recursive(arg_head, evaluate_all_args);
        return err;
    } else {
        return create_sexpr_tp(arg_head);
    }
}

// Evaluates an s-expression of any kind within the context of the provided
//   symbol table and list area.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the result (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* evaluate(const s_expr* se, environment* env) {
    typed_ptr* result = NULL;
    if (se == NULL) {
        result = create_error(EVAL_ERROR_NULL_SEXPR);
    } else if (is_empty_list(se)) {
        result = create_error(EVAL_ERROR_MISSING_PROCEDURE);
    } else if (se->car == NULL || se->cdr == NULL) {
        result = create_error(EVAL_ERROR_MALFORMED_SEXPR);
    } else {
        switch (se->car->type) {
            case TYPE_UNDEF:
                result = create_error(EVAL_ERROR_UNDEF_SYM);
                break;
            case TYPE_ERROR: // future: add info for traceback
                result = se->car;
                break;
            case TYPE_BUILTIN: {
                switch (se->car->ptr.idx) {
                    case BUILTIN_ADD: //    -|
                    case BUILTIN_MUL: //    -|
                    case BUILTIN_SUB: //    -|
                    case BUILTIN_DIV: //     v
                        result = eval_arithmetic(se, env);
                        break;
                    case BUILTIN_DEFINE:
                        result = eval_define(se, env);
                        break;
                    case BUILTIN_SETVAR:
                        result = eval_set_variable(se, env);
                        break;
                    case BUILTIN_EXIT: {
                        typed_ptr* args_tp = collect_args(se, env, 0, 0, false);
                        if (args_tp->type == TYPE_ERROR) {
                            result = args_tp;
                        } else {
                            result = create_error(EVAL_ERROR_EXIT);
                            delete_se_recursive(args_tp->ptr.se_ptr, false);
                            free(args_tp);
                        }
                        break;
                    }
                    case BUILTIN_CONS: {
                        typed_ptr* args_tp = collect_args(se, env, 2, 2, true);
                        if (args_tp->type == TYPE_ERROR) {
                            result = args_tp;
                        } else {
                            s_expr* arg_list = args_tp->ptr.se_ptr;
                            result = create_sexpr_tp(create_s_expr(arg_list->car, \
                                                                   sexpr_next(arg_list)->car));
                            arg_list->car = NULL;
                            sexpr_next(arg_list)->car = NULL;
                            delete_se_recursive(arg_list, true);
                            free(args_tp);
                        }
                        break;
                    }
                    case BUILTIN_CAR: //    -|
                    case BUILTIN_CDR: //    -V
                        result = eval_car_cdr(se, env);
                        break;
                    case BUILTIN_LIST:
                        result = eval_list_construction(se, env);
                        break;
                    case BUILTIN_AND: {
                        typed_ptr* args_tp = collect_args(se, env, 0, -1, true);
                        if (args_tp->type == TYPE_ERROR) {
                            result = args_tp;
                        } else {
                            s_expr* arg_se = create_s_expr(create_atom_tp(TYPE_BOOL, 1), args_tp);
                            s_expr* curr_se = arg_se;
                            s_expr* last = arg_se;
                            while (!is_empty_list(curr_se)) {
                                last = curr_se;
                                if (is_false_literal(last->car)) {
                                    break;
                                }
                                curr_se = sexpr_next(curr_se);
                            }
                            result = last->car;
                            last->car = NULL;
                            delete_se_recursive(arg_se, true);
                        }
                        break;
                    }
                    case BUILTIN_OR: {
                        typed_ptr* args_tp = collect_args(se, env, 0, -1, true);
                        if (args_tp->type == TYPE_ERROR) {
                            result = args_tp;
                        } else {
                            s_expr* arg_se = create_s_expr(create_atom_tp(TYPE_BOOL, 0), args_tp);
                            s_expr* curr_se = arg_se;
                            s_expr* last = arg_se;
                            while (!is_empty_list(curr_se)) {
                                last = curr_se;
                                if (!is_false_literal(last->car)) {
                                    break;
                                }
                                curr_se = sexpr_next(curr_se);
                            }
                            result = last->car;
                            last->car = NULL;
                            delete_se_recursive(arg_se, true);
                        }
                        break;
                    }
                    case BUILTIN_NOT: {
                        typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
                        if (args_tp->type == TYPE_ERROR) {
                            result = args_tp;
                        } else {
                            if (is_false_literal(args_tp->ptr.se_ptr->car)) {
                                result = create_atom_tp(TYPE_BOOL, 1);
                            } else {
                                result = create_atom_tp(TYPE_BOOL, 0);
                            }
                            delete_se_recursive(args_tp->ptr.se_ptr, true);
                            free(args_tp);
                        }
                        break;
                    }
                    case BUILTIN_COND:
                        result = eval_cond(se, env);
                        break;
                    case BUILTIN_LISTPRED:
                        result = eval_list_pred(se, env);
                        break;
                    case BUILTIN_PAIRPRED:
                        result = eval_atom_pred(se, env, TYPE_SEXPR);
                        break;
                    case BUILTIN_NUMBERPRED:
                        result = eval_atom_pred(se, env, TYPE_NUM);
                        break;
                    case BUILTIN_BOOLPRED:
                        result = eval_atom_pred(se, env, TYPE_BOOL);
                        break;
                    case BUILTIN_VOIDPRED:
                        result = eval_atom_pred(se, env, TYPE_VOID);
                        break;
                    case BUILTIN_NUMBEREQ: //    -|
                    case BUILTIN_NUMBERGT: //    -|
                    case BUILTIN_NUMBERLT: //    -|
                    case BUILTIN_NUMBERGE: //    -|
                    case BUILTIN_NUMBERLE: //    -V
                        result = eval_comparison(se, env);
                        break;
                    case BUILTIN_LAMBDA:
                        result = eval_lambda(se, env);
                        break;
                    default:
                        result = create_error(EVAL_ERROR_UNDEF_BUILTIN);
                        break;
                }
                break;
            }
            case TYPE_SEXPR: {
                s_expr* se_to_eval = se->car->ptr.se_ptr;
                if (is_empty_list(se_to_eval)) {
                    result = create_error(EVAL_ERROR_MISSING_PROCEDURE);
                } else {
                    s_expr* dummy_se = create_s_expr(se_to_eval->car, \
                                                     create_sexpr_tp(create_empty_s_expr()));
                    typed_ptr* fn = evaluate(dummy_se, env);
                    free(sexpr_next(dummy_se));
                    free(dummy_se->cdr);
                    free(dummy_se);
                    if (fn->type == TYPE_ERROR) {
                        result = fn;
                    } else {
                        if (fn->type == TYPE_BUILTIN || fn->type == TYPE_USER_FN) {
                            dummy_se = create_s_expr(fn, se_to_eval->cdr);
                            result = evaluate(dummy_se, env);
                            free(dummy_se);
                        } else {
                            result = create_error(EVAL_ERROR_CAR_NOT_CALLABLE);
                        }
                        free(fn);
                    }
                }
                break;
            }
            case TYPE_USER_FN:
                result = eval_user_function(se, env);
                break;
            case TYPE_SYM:
                result = value_lookup_index(env, se->car);
                break;
            case TYPE_NUM:  //    -| 
            case TYPE_BOOL: //    -|
            case TYPE_VOID: //    -V
                result = copy_typed_ptr(se->car);
                break;
            default:
                result = create_error(EVAL_ERROR_UNDEF_TYPE);
                break;
        }
    }
    return result;
}

#include "environment.h"

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

sym_tab_node* create_error_stn(interpreter_error err_code) {
    return create_st_node(0, NULL, TYPE_ERROR, (union_idx_se){.idx=err_code});
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

// Merges the second symbol table into the first; the second pointer remains
//   valid, but its head is set to NULL.
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
    second->head = NULL;
    return;
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

Function_Table* create_function_table(unsigned int offset) {
    Function_Table* new_ft = malloc(sizeof(Function_Table));
    if (new_ft == NULL) {
        fprintf(stderr, "malloc failed in create_function_table()\n");
        exit(-1);
    }
    new_ft->head = NULL;
    new_ft->length = 0;
    new_ft->offset = offset;
    return new_ft;
}

environment* create_environment(unsigned int st_start, unsigned int ft_start) {
    environment* new_env = malloc(sizeof(environment));
    if (new_env == NULL) {
        fprintf(stderr, "malloc failed in create_environment()\n");
        exit(-1);
    }
    new_env->symbol_table = create_symbol_table(st_start);
    new_env->function_table = create_function_table(ft_start);
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
                               long value) {
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

// A convenience function for use in parse().
// If there is a symbol whose name matches the specified substring in either
//   the symbol table or the temporary symbol table, a pointer to that symbol is
//   returned. If not, the appropriate symbol is installed in the temporary
//   symbol table, with type TYPE_UNDEF.
// In all cases, the returned typed_ptr is the caller's responsibility to free;
//   it is always safe to free without harm to either symbol table or any other
//   object.
typed_ptr* install_symbol_temp(environment* env, environment* temp, char* sym) {
    if (string_is_number(sym)) {
        long value = atol(sym);
        free(sym);
        return create_atom_tp(TYPE_NUM, value);
    } else {
        sym_tab_node* found = symbol_lookup_string(env, sym);
        if (found == NULL) {
            found = symbol_lookup_string(temp, sym);
            if (found == NULL) {
                return install_symbol(temp, \
                                      sym, \
                                      TYPE_UNDEF, \
                                      (union_idx_se){.idx=0});
            }
        }
        free(sym);
        return create_atom_tp(TYPE_SYM, found->symbol_number);
    }
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

void setup_symbol_table(environment* env) {
    builtin_code tbi = TYPE_BUILTIN;
    blind_install_symbol_atom(env, strdup("NULL_SENTINEL"), TYPE_UNDEF, 0);
    blind_install_symbol_atom(env, strdup("+"), tbi, BUILTIN_ADD);
    blind_install_symbol_atom(env, strdup("*"), tbi, BUILTIN_MUL);
    blind_install_symbol_atom(env, strdup("-"), tbi, BUILTIN_SUB);
    blind_install_symbol_atom(env, strdup("/"), tbi, BUILTIN_DIV);
    blind_install_symbol_atom(env, strdup("define"), tbi, BUILTIN_DEFINE);
    blind_install_symbol_atom(env, strdup("set!"), tbi, BUILTIN_SETVAR);
    blind_install_symbol_atom(env, strdup("exit"), tbi, BUILTIN_EXIT);
    blind_install_symbol_atom(env, strdup("cons"), tbi, BUILTIN_CONS);
    blind_install_symbol_atom(env, strdup("car"), tbi, BUILTIN_CAR);
    blind_install_symbol_atom(env, strdup("cdr"), tbi, BUILTIN_CDR);
    blind_install_symbol_atom(env, strdup("and"), tbi, BUILTIN_AND);
    blind_install_symbol_atom(env, strdup("or"), tbi, BUILTIN_OR);
    blind_install_symbol_atom(env, strdup("not"), tbi, BUILTIN_NOT);
    blind_install_symbol_atom(env, strdup("cond"), tbi, BUILTIN_COND);
    blind_install_symbol_atom(env, strdup("list"), tbi, BUILTIN_LIST);
    blind_install_symbol_atom(env, strdup("pair?"), tbi, BUILTIN_PAIRPRED);
    blind_install_symbol_atom(env, strdup("list?"), tbi, BUILTIN_LISTPRED);
    blind_install_symbol_atom(env, strdup("number?"), tbi, BUILTIN_NUMBERPRED);
    blind_install_symbol_atom(env, strdup("boolean?"), tbi, BUILTIN_BOOLPRED);
    blind_install_symbol_atom(env, strdup("void?"), tbi, BUILTIN_VOIDPRED);
    blind_install_symbol_atom(env, strdup("="), tbi, BUILTIN_NUMBEREQ);
    blind_install_symbol_atom(env, strdup(">"), tbi, BUILTIN_NUMBERGT);
    blind_install_symbol_atom(env, strdup("<"), tbi, BUILTIN_NUMBERLT);
    blind_install_symbol_atom(env, strdup(">="), tbi, BUILTIN_NUMBERGE);
    blind_install_symbol_atom(env, strdup("<="), tbi, BUILTIN_NUMBERLE);
    blind_install_symbol_atom(env, strdup("lambda"), tbi, BUILTIN_LAMBDA);
    blind_install_symbol_sexpr(env, \
                               strdup("null"), \
                               TYPE_SEXPR, \
                               create_empty_s_expr());
    blind_install_symbol_atom(env, strdup("#t"), TYPE_BOOL, 1);
    blind_install_symbol_atom(env, strdup("#f"), TYPE_BOOL, 0);
    blind_install_symbol_atom(env, strdup("else"), TYPE_UNDEF, 0);
    return;
}

void setup_environment(environment* env) {
    setup_symbol_table(env);
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

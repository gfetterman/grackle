#include "environment.h"

// The caller should ensure that the node's symbol number in the symbol table
//   will be unique.
// The string pointed to by name is now the symbol table's responsibility
//   to free. It also must be safe to free (i.e., it must be heap-allocated)
//   and nobody else should free it.
// The returned Symbol_Node is the caller's (i.e., the symbol table's)
//   responsibility to free.
Symbol_Node* create_symbol_node(unsigned int symbol_idx, \
                                char* name, \
                                type type, \
                                tp_value value) {
    Symbol_Node* new_node = malloc(sizeof(Symbol_Node));
    if (new_node == NULL) {
        fprintf(stderr, "fatal error: malloc failed in create_symbol_node()\n");
        exit(-1);
    }
    new_node->symbol_idx = symbol_idx;
    new_node->name = name;
    new_node->type = type;
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

Symbol_Node* create_error_symbol_node(interpreter_error err_code) {
    return create_symbol_node(0, NULL, TYPE_ERROR, (tp_value){.idx=err_code});
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
        Symbol_Node* curr = first->head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = second->head;
    }
    first->length += second->length;
    second->head = NULL;
    return;
}

void delete_symbol_node_list(Symbol_Node* sn) {
    while (sn != NULL) {
        Symbol_Node* next = sn->next;
        free(sn->name);
        free(sn);
        sn = next;
    }
    return;
}

// The arg list and closure environment are presumed to be unique to this
//   function table node, but the body points to an s-expression that is
//   represented elsewhere (and so cannot be safely freed using this pointer).
Function_Node* create_function_node(unsigned int function_idx, \
                                    Symbol_Node* arg_list, \
                                    environment* closure_env, \
                                    typed_ptr* body) {
    Function_Node* new_fn = malloc(sizeof(Function_Node));
    if (new_fn == NULL) {
        fprintf(stderr, "malloc failed in create_function_node()\n");
        exit(-1);
    }
    new_fn->function_idx = function_idx;
    new_fn->arg_list = arg_list;
    new_fn->closure_env = closure_env;
    new_fn->body = body;
    new_fn->next = NULL;
    return new_fn;
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
    Symbol_Node* curr_sn = env->symbol_table->head;
    while (curr_sn != NULL) {
        Symbol_Node* new_sn = create_symbol_node(curr_sn->symbol_idx, \
                                                 strdup(curr_sn->name), \
                                                 curr_sn->type, \
                                                 curr_sn->value);
        if (new_sn->type == TYPE_SEXPR) {
            new_sn->value.se_ptr = copy_s_expr(new_sn->value.se_ptr);
        }
        new_sn->next = new_env->symbol_table->head;
        new_env->symbol_table->head = new_sn;
        curr_sn = curr_sn->next;
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
    Symbol_Node* curr = env->symbol_table->head;
    while (curr != NULL) {
        Symbol_Node* next = curr->next;
        free(curr->name);
        if (curr->type == TYPE_SEXPR) {
            delete_s_expr_recursive(curr->value.se_ptr, true);
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
    Symbol_Node* curr_sn = env->symbol_table->head;
    while (curr_sn != NULL) {
        Symbol_Node* next_sn = curr_sn->next;
        free(curr_sn->name);
        if (curr_sn->type == TYPE_SEXPR) {
            delete_s_expr_recursive(curr_sn->value.se_ptr, true);
        }
        free(curr_sn);
        curr_sn = next_sn;
    }
    free(env->symbol_table);
    Function_Node* curr_fn = env->function_table->head;
    while (curr_fn != NULL) {
        Function_Node* next_fn = curr_fn->next;
        // free argument list
        Symbol_Node* curr_arg_sn = curr_fn->arg_list;
        while (curr_arg_sn != NULL) {
            Symbol_Node* next_arg_sn = curr_arg_sn->next;
            free(curr_arg_sn->name);
            free(curr_arg_sn);
            curr_arg_sn = next_arg_sn;
        }
        // free closure environment
        delete_env_shared_ft(curr_fn->closure_env);
        // free body s-expression
        if (curr_fn->body->type == TYPE_SEXPR) {
            delete_s_expr_recursive(curr_fn->body->ptr.se_ptr, true);
        }
        free(curr_fn->body);
        free(curr_fn);
        curr_fn = next_fn;
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
                          tp_value value) {
    unsigned int symbol_idx = env->symbol_table->length + \
                              env->symbol_table->symbol_number_offset;
    Symbol_Node* found = symbol_lookup_string(env, name);
    if (found == NULL) {
        Symbol_Node* new_sn = create_symbol_node(symbol_idx, name, type, value);
        new_sn->next = env->symbol_table->head;
        env->symbol_table->head = new_sn;
        env->symbol_table->length++;
    } else {
        free(name);
        found->type = type;
        found->value = value;
        symbol_idx = found->symbol_idx;
    }
    return create_atom_tp(TYPE_SYMBOL, symbol_idx);
}

// All considerations attendant upon the function "install_symbol()" above apply
//   here.
// This is a convenience function for use in initial symbol table setup.
void blind_install_symbol_atom(environment* env, \
                               char* name, \
                               type type, \
                               long value) {
    typed_ptr* tp = install_symbol(env, \
                                   name, \
                                   type, \
                                   (tp_value){.idx=value});
    free(tp);
    return;
}

// All considerations attendant upon the function "install_symbol()" above apply
//   here.
// This is a convenience function for use in initial symbol table setup.
void blind_install_symbol_sexpr(environment* env, \
                                char* name, \
                                type type, \
                                s_expr* value) {
    typed_ptr* tp = install_symbol(env, \
                                   name, \
                                   type, \
                                   (tp_value){.se_ptr=value});
    free(tp);
    return;
}

// The arg list and closure environment are now the (general) environment's
//   concern. The body pointed to by the typed pointer remains someone else's
//   problem, and won't be freed by the environment.
// The typed pointer returned is the caller's responsibility to free.
typed_ptr* install_function(environment* env, \
                            Symbol_Node* param_list, \
                            environment* closure_env, \
                            typed_ptr* body) {
    unsigned int idx = env->function_table->length;
    Function_Node* new_fn = create_function_node(idx, \
                                                 param_list, \
                                                 closure_env, \
                                                 body);
    new_fn->next = env->function_table->head;
    env->function_table->head = new_fn;
    env->function_table->length++;
    // and it's now also in closure_env, because the two environments share a
    //   function table
    return create_atom_tp(TYPE_FUNCTION, idx);
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

// The returned Symbol_Node should (usually) not be freed.
// If the given name does not match any symbol table entry, NULL is returned.
Symbol_Node* symbol_lookup_string(environment* env, const char* name) {
    Symbol_Node* curr = env->symbol_table->head;
    while (curr != NULL) {
        if (!strcmp(curr->name, name)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

// The returned Symbol_Node should (usually) not be freed.
// If the given index does not match any symbol table entry, NULL is returned.
Symbol_Node* symbol_lookup_index(environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_SYMBOL) {
        return NULL;
    }
    Symbol_Node* curr = env->symbol_table->head;
    while (curr != NULL) {
        if (curr->symbol_idx == tp->ptr.idx) {
            break;
        }
        curr = curr->next;
    }
    return curr;
}

Symbol_Node* builtin_lookup_index(environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_BUILTIN) {
        return NULL;
    }
    Symbol_Node* curr = env->symbol_table->head;
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
    if (tp == NULL || tp->type != TYPE_SYMBOL) {
        return NULL;
    }
    Symbol_Node* curr = env->symbol_table->head;
    while (curr != NULL) {
        if (curr->symbol_idx == tp->ptr.idx) {
            if (curr->type == TYPE_UNDEF) {
                return create_error_tp(EVAL_ERROR_UNDEF_SYM);
            } else if (curr->type == TYPE_SEXPR) {
                return create_s_expr_tp(copy_s_expr(curr->value.se_ptr));
            } else {
                return create_typed_ptr(curr->type, curr->value);
            }
        }
        curr = curr->next;
    }
    return NULL;
}

// The Function_Node returned shouldn't (usually) be freed.
// tp is assumed to be an atomic typed_ptr.
Function_Node* function_lookup_index(environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_FUNCTION) {
        return NULL;
    }
    Function_Node* curr = env->function_table->head;
    while (curr != NULL) {
        if (curr->function_idx == tp->ptr.idx) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

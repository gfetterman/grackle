#include "environment.h"

// The caller should ensure that the node's symbol number in the symbol table
//   will be unique.
// The string name points to is copied. name must not be NULL.
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
    new_node->name = strdup(name);
    new_node->type = type;
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

Symbol_Node* create_error_symbol_node(interpreter_error err_code) {
    return create_symbol_node(0, "", TYPE_ERROR, (tp_value){.idx=err_code});
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
    new_st->offset = offset;
    return new_st;
}

// Merges the second symbol table into the first; the second pointer remains
//   valid, but its head is set to NULL and its length to zero.
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
    second->length = 0;
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
                                    char* name, \
                                    Symbol_Node* param_list, \
                                    Environment* enclosing_env, \
                                    typed_ptr* body) {
    Function_Node* new_fn = malloc(sizeof(Function_Node));
    if (new_fn == NULL) {
        fprintf(stderr, "malloc failed in create_function_node()\n");
        exit(-1);
    }
    new_fn->function_idx = function_idx;
    new_fn->name = strdup(name);
    new_fn->param_list = param_list;
    new_fn->enclosing_env = enclosing_env;
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

Environment* create_environment(unsigned int symbol_start, \
                                unsigned int function_start, \
                                Environment* enclosing_env) {
    Environment* new_env = malloc(sizeof(Environment));
    if (new_env == NULL) {
        fprintf(stderr, "malloc failed in create_environment()\n");
        exit(-1);
    }
    new_env->symbol_table = create_symbol_table(symbol_start);
    new_env->function_table = create_function_table(function_start);
    new_env->enclosing_env = enclosing_env;
    return new_env;
}

// The new environment contains a deep copy of the old environment's symbol
//   table. Symbol table numbers are preserved (so pointers into it are still
//   valid).
// However, because the function table is currently only added to (not modified
//   or deleted from), its pointer is simply copied over).
// To destroy this new environment, delete_env_shared_ft() below should be used,
//   to ensure it's done safely.
Environment* copy_environment(Environment* env) {
    Environment* new_env = create_environment(0, 0, env->enclosing_env);
    Symbol_Node* curr_sn = env->symbol_table->head;
    while (curr_sn != NULL) {
        Symbol_Node* new_sn = create_symbol_node(curr_sn->symbol_idx, \
                                                 curr_sn->name, \
                                                 curr_sn->type, \
                                                 curr_sn->value);
        if (new_sn->type == TYPE_S_EXPR) {
            new_sn->value.se_ptr = copy_s_expr(new_sn->value.se_ptr);
        } else if (new_sn->type == TYPE_STRING) {
            char* contents = new_sn->value.string->contents;
            new_sn->value.string = create_string(contents);
        }
        new_sn->next = new_env->symbol_table->head;
        new_env->symbol_table->head = new_sn;
        curr_sn = curr_sn->next;
    }
    new_env->symbol_table->length = env->symbol_table->length;
    // risky, but currently ok, because the function table is only added to,
    // not modified or deleted from
    free(new_env->function_table);
    new_env->function_table = env->function_table;
    new_env->enclosing_env = env->enclosing_env;
    return new_env;
}

// Safely deletes an environment that shares a function table with other
//   environments.
void delete_environment_shared(Environment* env) {
    Symbol_Node* curr = env->symbol_table->head;
    while (curr != NULL) {
        Symbol_Node* next = curr->next;
        free(curr->name);
        if (curr->type == TYPE_S_EXPR) {
            delete_s_expr_recursive(curr->value.se_ptr, true);
        } else if (curr->type == TYPE_STRING) {
            delete_string(curr->value.string);
        }
        free(curr);
        curr = next;
    }
    free(env->symbol_table);
    free(env);
    return;
}

// Fully deletes an environment.
void delete_environment_full(Environment* env) {
    Symbol_Node* curr_sn = env->symbol_table->head;
    while (curr_sn != NULL) {
        Symbol_Node* next_sn = curr_sn->next;
        free(curr_sn->name);
        if (curr_sn->type == TYPE_S_EXPR) {
            delete_s_expr_recursive(curr_sn->value.se_ptr, true);
        } else if (curr_sn->type == TYPE_STRING) {
            delete_string(curr_sn->value.string);
        }
        free(curr_sn);
        curr_sn = next_sn;
    }
    free(env->symbol_table);
    Function_Node* curr_fn = env->function_table->head;
    while (curr_fn != NULL) {
        Function_Node* next_fn = curr_fn->next;
        free(curr_fn->name);
        // free parameter list
        Symbol_Node* curr_param_sn = curr_fn->param_list;
        while (curr_param_sn != NULL) {
            Symbol_Node* next_param_sn = curr_param_sn->next;
            free(curr_param_sn->name);
            free(curr_param_sn);
            curr_param_sn = next_param_sn;
        }
        // free closure environment
        delete_environment_shared(curr_fn->enclosing_env);
        // free body s-expression
        if (curr_fn->body->type == TYPE_S_EXPR) {
            delete_s_expr_recursive(curr_fn->body->ptr.se_ptr, true);
        } else if (curr_fn->body->type == TYPE_STRING) {
            delete_string(curr_fn->body->ptr.string);
        }
        free(curr_fn->body);
        free(curr_fn);
        curr_fn = next_fn;
    }
    free(env->function_table);
    free(env);
    return;
}

// The string pointed to by name is copied, and must not be NULL.
// If a symbol already exists in the given symbol table with that name, the type
//   and value associated with it will be updated. Otherwise, the symbol is
//   installed into the symbol table.
// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to the symbol table or any other
//   object.
typed_ptr* install_symbol(Environment* env, char* name, typed_ptr* tp) {
    unsigned int idx = env->symbol_table->length + env->symbol_table->offset;
    Symbol_Node* found = symbol_lookup_name(env, name);
    if (found == NULL) {
        Symbol_Node* sn = create_symbol_node(idx, name, tp->type, tp->ptr);
        sn->next = env->symbol_table->head;
        env->symbol_table->head = sn;
        env->symbol_table->length++;
    } else {
        if (found->type == TYPE_S_EXPR) {
            delete_s_expr_recursive(found->value.se_ptr, true);
        } else if (found->type == TYPE_STRING) {
            delete_string(found->value.string);
        }
        found->type = tp->type;
        found->value = tp->ptr;
        idx = found->symbol_idx;
    }
    return create_atom_tp(TYPE_SYMBOL, idx);
}

// All considerations attendant upon the function "install_symbol()" above apply
//   here.
// This is a convenience function for use in initial symbol table setup.
void blind_install_symbol(Environment* env, char* name, typed_ptr* tp) {
    typed_ptr* result = install_symbol(env, name, tp);
    free(result);
    return;
}

// The arg list and closure environment are now the (general) environment's
//   concern. The body pointed to by the typed pointer remains someone else's
//   problem, and won't be freed by the environment.
// The typed pointer returned is the caller's responsibility to free.
typed_ptr* install_function(Environment* env, \
                            char* name, \
                            Symbol_Node* param_list, \
                            Environment* enclosing_env, \
                            typed_ptr* body) {
    unsigned int idx = env->function_table->length;
    Function_Node* new_fn = create_function_node(idx, \
                                                 name, \
                                                 param_list, \
                                                 enclosing_env, \
                                                 body);
    new_fn->next = env->function_table->head;
    env->function_table->head = new_fn;
    env->function_table->length++;
    return create_atom_tp(TYPE_FUNCTION, idx);
}

void setup_symbol_table(Environment* env) {
    #define ATOM_TP(type_, idx_) (typed_ptr){.type=type_, .ptr={.idx=idx_}}
    #define S_EXPR_TP(ptr_) (typed_ptr){.type=TYPE_S_EXPR, .ptr={.se_ptr=ptr_}}
    builtin_code tbi = TYPE_BUILTIN;
    // builtin operators
    blind_install_symbol(env, "+", &ATOM_TP(tbi, BUILTIN_ADD));
    blind_install_symbol(env, "*", &ATOM_TP(tbi, BUILTIN_MUL));
    blind_install_symbol(env, "-", &ATOM_TP(tbi, BUILTIN_SUB));
    blind_install_symbol(env, "/", &ATOM_TP(tbi, BUILTIN_DIV));
    blind_install_symbol(env, "define", &ATOM_TP(tbi, BUILTIN_DEFINE));
    blind_install_symbol(env, "set!", &ATOM_TP(tbi, BUILTIN_SETVAR));
    blind_install_symbol(env, "exit", &ATOM_TP(tbi, BUILTIN_EXIT));
    blind_install_symbol(env, "cons", &ATOM_TP(tbi, BUILTIN_CONS));
    blind_install_symbol(env, "car", &ATOM_TP(tbi, BUILTIN_CAR));
    blind_install_symbol(env, "cdr", &ATOM_TP(tbi, BUILTIN_CDR));
    blind_install_symbol(env, "and", &ATOM_TP(tbi, BUILTIN_AND));
    blind_install_symbol(env, "or", &ATOM_TP(tbi, BUILTIN_OR));
    blind_install_symbol(env, "not", &ATOM_TP(tbi, BUILTIN_NOT));
    blind_install_symbol(env, "cond", &ATOM_TP(tbi, BUILTIN_COND));
    blind_install_symbol(env, "list", &ATOM_TP(tbi, BUILTIN_LIST));
    blind_install_symbol(env, "pair?", &ATOM_TP(tbi, BUILTIN_PAIRPRED));
    blind_install_symbol(env, "list?", &ATOM_TP(tbi, BUILTIN_LISTPRED));
    blind_install_symbol(env, "number?", &ATOM_TP(tbi, BUILTIN_NUMBERPRED));
    blind_install_symbol(env, "boolean?", &ATOM_TP(tbi, BUILTIN_BOOLPRED));
    blind_install_symbol(env, "void?", &ATOM_TP(tbi, BUILTIN_VOIDPRED));
    blind_install_symbol(env, "procedure?", &ATOM_TP(tbi, BUILTIN_PROCPRED));
    blind_install_symbol(env, "null?", &ATOM_TP(tbi, BUILTIN_NULLPRED));
    blind_install_symbol(env, "symbol?", &ATOM_TP(tbi, BUILTIN_SYMBOLPRED));
    blind_install_symbol(env, "string?", &ATOM_TP(tbi, BUILTIN_STRINGPRED));
    blind_install_symbol(env, "=", &ATOM_TP(tbi, BUILTIN_NUMBEREQ));
    blind_install_symbol(env, ">", &ATOM_TP(tbi, BUILTIN_NUMBERGT));
    blind_install_symbol(env, "<", &ATOM_TP(tbi, BUILTIN_NUMBERLT));
    blind_install_symbol(env, ">=", &ATOM_TP(tbi, BUILTIN_NUMBERGE));
    blind_install_symbol(env, "<=", &ATOM_TP(tbi, BUILTIN_NUMBERLE));
    blind_install_symbol(env, "lambda", &ATOM_TP(tbi, BUILTIN_LAMBDA));
    blind_install_symbol(env, "quote", &ATOM_TP(tbi, BUILTIN_QUOTE));
    blind_install_symbol(env, \
                         "string-length", \
                         &ATOM_TP(tbi, BUILTIN_STRINGLEN));
    blind_install_symbol(env, "string=?", &ATOM_TP(tbi, BUILTIN_STRINGEQ));
    blind_install_symbol(env, \
                         "string-append", \
                         &ATOM_TP(tbi, BUILTIN_STRINGAPPEND));
    // special values and keywords
    blind_install_symbol(env, "else", &ATOM_TP(TYPE_UNDEF, 0));
    blind_install_symbol(env, "null", &S_EXPR_TP(create_empty_s_expr()));
    #undef ATOM_TP
    #undef S_EXPR_TP
    return;
}

void setup_environment(Environment* env) {
    setup_symbol_table(env);
    return;
}

// The returned Symbol_Node should (usually) not be freed.
// If the given name does not match any symbol table entry in the immediate
//   environment provided, NULL is returned.
Symbol_Node* symbol_lookup_name(const Environment* env, const char* name) {
    if (name == NULL) {
        return NULL;
    }
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
// If the given index does not match any symbol table entry in the immediate
//   environment provided, NULL is returned.
Symbol_Node* symbol_lookup_index(const Environment* env, const typed_ptr* tp) {
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

Symbol_Node* builtin_lookup_index(const Environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_BUILTIN) {
        return NULL;
    }
    const Environment* global_env = env;
    while (global_env->enclosing_env != NULL) {
        global_env = global_env->enclosing_env;
    }
    Symbol_Node* curr = global_env->symbol_table->head;
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
// Symbol table are searched beginning in the provided environment and marching
//   upwards through its enclosing environments. The first symbol table
//   containing the provided symbol contains the value returned.
// If the given typed_ptr does not point to a valid symbol table entry in any
//   enclosing environment, or if it is NULL, NULL is returned.
typed_ptr* value_lookup_index(const Environment* env, const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_SYMBOL) {
        return NULL;
    }
    const Environment* curr_env = env;
    while (curr_env != NULL) {
        Symbol_Node* curr = symbol_lookup_index(curr_env, tp);
        if (curr != NULL) {
            switch (curr->type) {
                case TYPE_UNDEF:
                    return create_error_tp(EVAL_ERROR_UNDEF_SYM);
                case TYPE_S_EXPR:
                    return create_s_expr_tp(copy_s_expr(curr->value.se_ptr));
                case TYPE_STRING: {
                    char* contents = curr->value.string->contents;
                    return create_string_tp(create_string(contents));
                }
                default:
                    return create_typed_ptr(curr->type, curr->value);
            }
        }
        curr_env = curr_env->enclosing_env;
    }
    return NULL;
}

// The Function_Node returned shouldn't (usually) be freed.
// tp is assumed to be an atomic typed_ptr.
Function_Node* function_lookup_index(const Environment* env, \
                                     const typed_ptr* tp) {
    if (tp == NULL || tp->type != TYPE_FUNCTION) {
        return NULL;
    }
    const Environment* global_env = env;
    while (global_env->enclosing_env != NULL) {
        global_env = global_env->enclosing_env;
    }
    Function_Node* curr = global_env->function_table->head;
    while (curr != NULL) {
        if (curr->function_idx == tp->ptr.idx) {
            return curr;
        }
        curr = curr->next;
    }
    return curr;
}

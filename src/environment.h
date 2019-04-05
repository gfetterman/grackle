#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "fundamentals.h"
#include "grackle_utils.h"

// symbol table and symbol table nodes

typedef struct SYMBOL_NODE {
    unsigned int symbol_idx;
    char* name;
    type type;
    tp_value value;
    struct SYMBOL_NODE* next;
} Symbol_Node;

Symbol_Node* create_symbol_node(unsigned int symbol_idx, \
                                char* name, \
                                type type, \
                                tp_value value);
Symbol_Node* create_error_symbol_node(interpreter_error err_code);

typedef struct SYMBOL_TABLE {
    Symbol_Node* head;
    unsigned int length;
    unsigned int symbol_number_offset;
} Symbol_Table;

Symbol_Table* create_symbol_table(unsigned int offset);
void merge_symbol_tables(Symbol_Table* first, Symbol_Table* second);

void delete_symbol_node_list(Symbol_Node* sn);

// function table and function table nodes

struct ENVIRONMENT; // forward declaration

typedef struct FUNCTION_NODE {
    unsigned int function_idx;
    Symbol_Node* arg_list;
    struct ENVIRONMENT* closure_env;
    typed_ptr* body;
    struct FUNCTION_NODE* next;
} Function_Node;

Function_Node* create_function_node(unsigned int function_idx, \
                                    Symbol_Node* param_list, \
                                    struct ENVIRONMENT* closure_env, \
                                    typed_ptr* body);

typedef struct FUNCTION_TABLE {
    Function_Node* head;
    unsigned int length;
    unsigned int offset;
} Function_Table;

Function_Table* create_function_table(unsigned int offset);

// the Environment structure

typedef struct ENVIRONMENT {
    Symbol_Table* symbol_table;
    Function_Table* function_table;
} Environment;

Environment* create_environment(unsigned int st_start, unsigned int ft_start);
Environment* copy_environment(Environment* env);
void delete_env_shared_ft(Environment* env);
void delete_env_full(Environment* env);

typed_ptr* install_symbol(Environment* env, \
                          char* name, \
                          type type, \
                          tp_value value);
void blind_install_symbol_atom(Environment* env, \
                               char* symbol, \
                               type type, \
                               long value);
void blind_install_symbol_sexpr(Environment* env, \
                                char* symbol, \
                                type type, \
                                s_expr* value);
typed_ptr* install_function(Environment* env, \
                            Symbol_Node* arg_list, \
                            Environment* closure_env, \
                            typed_ptr* body);

void setup_symbol_table(Environment* env);
void setup_environment(Environment* env);

Symbol_Node* symbol_lookup_string(Environment* env, const char* name);
Symbol_Node* symbol_lookup_index(Environment* env, const typed_ptr* tp);
Symbol_Node* builtin_lookup_index(Environment* env, const typed_ptr* tp);
typed_ptr* value_lookup_index(Environment* env, const typed_ptr* tp);
Function_Node* function_lookup_index(Environment* env, const typed_ptr* tp);

#endif

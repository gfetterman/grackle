#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "fundamentals.h"

// symbol node

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
void delete_symbol_node_list(Symbol_Node* sn);

// symbol table

typedef struct SYMBOL_TABLE {
    Symbol_Node* head;
    unsigned int length;
    unsigned int offset;
} Symbol_Table;

Symbol_Table* create_symbol_table(unsigned int offset);
void merge_symbol_tables(Symbol_Table* first, Symbol_Table* second);

// function node and function table

struct ENVIRONMENT;

typedef struct FUNCTION_NODE {
    unsigned int function_idx;
    char* name;
    Symbol_Node* param_list;
    struct ENVIRONMENT* enclosing_env;
    typed_ptr* body;
    struct FUNCTION_NODE* next;
} Function_Node;

Function_Node* create_function_node(unsigned int function_idx, \
                                    char* name, \
                                    Symbol_Node* param_list, \
                                    struct ENVIRONMENT* enclosing_env, \
                                    typed_ptr* body);

typedef struct FUNCTION_TABLE {
    Function_Node* head;
    unsigned int length;
    unsigned int offset;
} Function_Table;

Function_Table* create_function_table(unsigned int offset);

// environment

typedef struct ENVIRONMENT {
    Symbol_Table* symbol_table;
    Function_Table* function_table;
    struct ENVIRONMENT* enclosing_env;
} Environment;

Environment* create_environment(unsigned int symbol_start, \
                                unsigned int function_start, \
                                Environment* enclosing_env);
Environment* copy_environment(Environment* env);
void delete_environment_shared(Environment* env);
void delete_environment_full(Environment* env);

// adding things to an environment

typed_ptr* install_symbol(Environment* env, char* name, typed_ptr* tp);
void blind_install_symbol(Environment* env, char* name, typed_ptr* tp);
typed_ptr* install_function(Environment* env, \
                            char* name, \
                            Symbol_Node* arg_list, \
                            Environment* enclosing_env, \
                            typed_ptr* body);

void setup_environment(Environment* env);
void setup_symbol_table(Environment* env);

// looking things up in an environment

Symbol_Node* symbol_lookup_name(const Environment* env, const char* name);
Symbol_Node* symbol_lookup_index(const Environment* env, const typed_ptr* tp);
Symbol_Node* builtin_lookup_index(const Environment* env, const typed_ptr* tp);
typed_ptr* value_lookup_index(const Environment* env, const typed_ptr* tp);
Function_Node* function_lookup_index(const Environment* env, \
                                     const typed_ptr* tp);

#endif

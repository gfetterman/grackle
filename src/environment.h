#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "fundamentals.h"
#include "grackle_utils.h"

// symbol table and symbol table nodes

typedef struct SYMBOL_TABLE_NODE {
    unsigned int symbol_number;
    char* symbol;
    type type;
    tp_value value;
    struct SYMBOL_TABLE_NODE* next;
} sym_tab_node;

sym_tab_node* create_st_node(unsigned int symbol_number, \
                             char* name, \
                             type type, \
                             tp_value value);
sym_tab_node* create_error_stn(interpreter_error err_code);

typedef struct SYMBOL_TABLE {
    sym_tab_node* head;
    unsigned int length;
    unsigned int symbol_number_offset;
} Symbol_Table;

Symbol_Table* create_symbol_table(unsigned int offset);
void merge_symbol_tables(Symbol_Table* first, Symbol_Table* second);

void delete_st_node_list(sym_tab_node* stn);

// function table and function table nodes

struct ENVIRONMENT; // forward declaration

typedef struct FUN_TAB_NODE {
    unsigned int function_number;
    sym_tab_node* arg_list;
    struct ENVIRONMENT* closure_env;
    typed_ptr* body;
    struct FUN_TAB_NODE* next;
} fun_tab_node;

fun_tab_node* create_ft_node(unsigned int function_number, \
                             sym_tab_node* arg_list, \
                             struct ENVIRONMENT* closure_env, \
                             typed_ptr* body);

typedef struct FUNCTION_TABLE {
    fun_tab_node* head;
    unsigned int length;
    unsigned int offset;
} Function_Table;

Function_Table* create_function_table(unsigned int offset);

// the environment structure

typedef struct ENVIRONMENT {
    Symbol_Table* symbol_table;
    Function_Table* function_table;
} environment;

environment* create_environment(unsigned int st_start, unsigned int ft_start);
environment* copy_environment(environment* env);
void delete_env_shared_ft(environment* env);
void delete_env_full(environment* env);

typed_ptr* install_symbol(environment* env, \
                          char* name, \
                          type type, \
                          tp_value value);
void blind_install_symbol_atom(environment* env, \
                               char* symbol, \
                               type type, \
                               long value);
void blind_install_symbol_sexpr(environment* env, \
                                char* symbol, \
                                type type, \
                                s_expr* value);
typed_ptr* install_function(environment* env, \
                            sym_tab_node* arg_list, \
                            environment* closure_env, \
                            typed_ptr* body);

void setup_symbol_table(environment* env);
void setup_environment(environment* env);

sym_tab_node* symbol_lookup_string(environment* env, const char* name);
sym_tab_node* symbol_lookup_index(environment* env, const typed_ptr* tp);
sym_tab_node* builtin_lookup_index(environment* env, const typed_ptr* tp);
typed_ptr* value_lookup_index(environment* env, const typed_ptr* tp);
fun_tab_node* function_lookup_index(environment* env, const typed_ptr* tp);

#endif

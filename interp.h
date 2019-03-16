#ifndef INTERP_H
#define INTERP_H

#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

// types

typedef enum {TYPE_UNDEF, \
              TYPE_ERROR, \
              TYPE_VOID, \
              TYPE_BUILTIN, \
              TYPE_NUM, \
              TYPE_BOOL, \
              TYPE_SEXPR, \
              TYPE_SYM, \
              TYPE_USER_FN} type;

// built-in functions and special forms

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
              BUILTIN_NUMBERLE, \
              BUILTIN_LAMBDA} builtin_code;

// error codes

typedef enum {PARSE_ERROR_NONE, \
              PARSE_ERROR_UNBAL_PAREN, \
              PARSE_ERROR_BARE_SYM, \
              PARSE_ERROR_EMPTY_PAREN, \
              PARSE_ERROR_TOO_MANY, \
              // ^  parsing errors above     ^
              // v  evaluation errors below  v
              EVAL_ERROR_EXIT, \
              EVAL_ERROR_NULL_SEXPR, \
              EVAL_ERROR_MALFORMED_SEXPR, \
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
              EVAL_ERROR_NOT_ID, \
              EVAL_ERROR_MISSING_PROCEDURE, \
              EVAL_ERROR_BAD_SYNTAX, \
              EVAL_ERROR_EMPTY_ELSE, \
              EVAL_ERROR_ILLEGAL_PAIR} interpreter_error;

// s-expressions & typed pointers

// forward declaration
struct S_EXPR_NODE;

typedef union TP_PTR {
    unsigned int idx;
    struct S_EXPR_NODE* se_ptr;
} union_idx_se;

typedef struct TYPED_PTR {
    type type;
    union_idx_se ptr;
} typed_ptr;

typedef struct S_EXPR_NODE {
    typed_ptr* car;
    typed_ptr* cdr;
} s_expr;

typed_ptr* create_typed_ptr(type type, union_idx_se ptr);
typed_ptr* create_atom_tp(type type, unsigned int idx);
typed_ptr* create_sexpr_tp(s_expr* se_ptr);
typed_ptr* create_error(unsigned int err_code);
typed_ptr* copy_typed_ptr(const typed_ptr* tp);

s_expr* create_s_expr(typed_ptr* car, typed_ptr* cdr);
s_expr* create_empty_s_expr();
void delete_s_expr(s_expr* se);
void delete_se_recursive(s_expr* se);

// symbol table and symbol table nodes

typedef struct SYMBOL_TABLE_NODE {
    unsigned int symbol_number;
    char* symbol;
    type type;
    union_idx_se value;
    struct SYMBOL_TABLE_NODE* next;
} sym_tab_node;

sym_tab_node* create_st_node(unsigned int symbol_number, \
                             char* name, \
                             type type, \
                             union_idx_se value);

typedef struct SYMBOL_TABLE {
    sym_tab_node* head;
    unsigned int length;
    unsigned int symbol_number_offset;
} Symbol_Table;

Symbol_Table* create_symbol_table(unsigned int offset);

// s-expression storage nodes (used during parsing)

typedef struct S_EXPR_STORAGE_NODE {
    unsigned int list_number;
    s_expr* se;
    struct S_EXPR_STORAGE_NODE* next;
} s_expr_storage;

s_expr_storage* create_s_expr_storage(unsigned int list_number, s_expr* se);
void se_stack_push(s_expr_storage** stack, s_expr* new_se);
void se_stack_pop(s_expr_storage** stack);
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
} function_table;

function_table* create_function_table(unsigned int offset);

// the environment structure

typedef struct ENVIRONMENT {
    Symbol_Table* symbol_table;
    function_table* function_table;
} environment;

environment* create_environment(unsigned int st_offset, \
                                unsigned int ft_offset);
environment* copy_environment(environment* env);
void delete_env(environment* env);
sym_tab_node* symbol_lookup_string(environment* env, const char* name);
sym_tab_node* symbol_lookup_index(environment* env, unsigned int index);
typed_ptr* install_symbol(environment* env, \
                          char* name, \
                          type type, \
                          union_idx_se value);
void blind_install_symbol_atom(environment* env, \
                               char* symbol, \
                               type type, \
                               unsigned int value);
void blind_install_symbol_sexpr(environment* env, \
                                char* symbol, \
                                type type, \
                                s_expr* value);
void setup_symbol_table(environment* env);
void setup_environment(environment* env);
typed_ptr* install_symbol_substring(environment* env, \
                                    environment* temp_env, \
                                    char str[], \
                                    unsigned int start, \
                                    unsigned int end);
sym_tab_node* lookup_builtin(environment* env, builtin_code bc);
typed_ptr* value_lookup(environment* env, typed_ptr* tp);
void merge_symbol_tables(Symbol_Table* first, Symbol_Table* second);
typed_ptr* install_function(environment* env, \
                            sym_tab_node* arg_list, \
                            environment* closure_env, \
                            typed_ptr* body);
fun_tab_node* function_lookup(environment* env, typed_ptr* tp);

// I/O

void get_input(char* prompt, char buffer[], unsigned int buffer_size);
void print_error(const typed_ptr* tp);
void print_s_expression(const s_expr* se, environment* env);
void print_result(const typed_ptr* tp, environment* env);

// helper functions
char* substring(char* str, unsigned int start, unsigned int end);
bool string_is_number(const char str[]);
bool is_false_literal(const typed_ptr* tp);
bool is_empty_list(const s_expr* se);
s_expr* sexpr_next(const s_expr* se);

// primary functions

s_expr* parse(char str[], environment* env);
typed_ptr* evaluate(const s_expr* se, environment* env);

// evaluation functions for built-in functions and special forms

typed_ptr* collect_args(const s_expr* se, \
                        environment* env, \
                        int min_args, \
                        int max_args, \
                        bool evaluate_all_args);

typed_ptr* eval_arithmetic(const s_expr* se, environment* env);
typed_ptr* eval_comparison(const s_expr* se, environment* env);
typed_ptr* eval_lambda(const s_expr* se, environment* env);
typed_ptr* eval_define(const s_expr* se, environment* env);
typed_ptr* eval_set_variable(const s_expr* se, environment* env);
typed_ptr* eval_car_cdr(const s_expr* se, environment* env);
typed_ptr* eval_list_pred(const s_expr* se, environment* env);
typed_ptr* eval_atom_pred(const s_expr* se, environment* env, type t);
typed_ptr* eval_list_construction(const s_expr* se, environment* env);
typed_ptr* eval_cond(const s_expr* se, environment* env);

sym_tab_node* collect_parameters(typed_ptr* tp, environment* env);
typed_ptr* eval_lambda(const s_expr* se, environment* env);

sym_tab_node* bind_args(environment* env, fun_tab_node* ftn, typed_ptr* args);
environment* make_eval_env(environment* env, sym_tab_node* bound_args);
typed_ptr* eval_user_function(const s_expr* se, environment* env);

#endif

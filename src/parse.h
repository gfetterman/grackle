#ifndef PARSE_H
#define PARSE_H

#include<stdlib.h>
#include<stdio.h>

#include "fundamentals.h"
#include "environment.h"

typedef enum PARSE_STATE {PARSE_START, \
                          PARSE_NEW_SEXPR, \
                          PARSE_READY, \
                          PARSE_READ_SYMBOL, \
                          PARSE_FINISH, \
                          PARSE_ERROR} Parse_State;

// s-expression storage nodes (used during parsing)

typedef struct S_EXPR_STORAGE_NODE {
    unsigned int list_number;
    s_expr* se;
    struct S_EXPR_STORAGE_NODE* next;
} s_expr_storage;

typed_ptr* parse(char str[], environment* env);

void init_new_s_expr(s_expr_storage** stack);
void extend_s_expr(s_expr_storage** stack);
Parse_State terminate_s_expr(s_expr_storage** stack, interpreter_error* error);
void register_symbol(s_expr_storage** stack, \
                     environment* env, \
                     environment* temp, \
                     char* name);

s_expr_storage* create_s_expr_storage(unsigned int list_number, s_expr* se);
void se_stack_push(s_expr_storage** stack, s_expr* new_se);
void se_stack_pop(s_expr_storage** stack);

#endif

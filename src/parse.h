#ifndef PARSE_H
#define PARSE_H

#include<stdlib.h>
#include<stdio.h>

#include "fundamentals.h"
#include "environment.h"

typedef enum PARSE_STATE {PARSE_START, \
                          PARSE_NEW_S_EXPR, \
                          PARSE_READY, \
                          PARSE_READ_SYMBOL, \
                          PARSE_FINISH, \
                          PARSE_ERROR} Parse_State;

// s-expression storage nodes (used during parsing)

typedef struct S_EXPR_STACK_NODE {
    s_expr* se;
    struct S_EXPR_STACK_NODE* next;
} s_expr_stack;

s_expr_stack* create_s_expr_stack(s_expr* se);
void s_expr_stack_push(s_expr_stack** stack, s_expr* new_se);
void s_expr_stack_pop(s_expr_stack** stack);

typed_ptr* parse(const char str[], Environment* env);

void init_new_s_expr(s_expr_stack** stack);
void extend_s_expr(s_expr_stack** stack);
Parse_State terminate_s_expr(s_expr_stack** stack, interpreter_error* error);
void register_symbol(s_expr_stack** stack, \
                     Environment* env, \
                     Environment* temp_env, \
                     char* name);
char* substring(const char* str, unsigned int start, unsigned int end);
bool string_is_number(const char str[]);

#endif

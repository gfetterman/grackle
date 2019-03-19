#ifndef PARSE_H
#define PARSE_H

#include<stdlib.h>
#include<stdio.h>

#include "fundamentals.h"
#include "environment.h"

// s-expression storage nodes (used during parsing)

typedef struct S_EXPR_STORAGE_NODE {
    unsigned int list_number;
    s_expr* se;
    struct S_EXPR_STORAGE_NODE* next;
} s_expr_storage;

s_expr_storage* create_s_expr_storage(unsigned int list_number, s_expr* se);
void se_stack_push(s_expr_storage** stack, s_expr* new_se);
void se_stack_pop(s_expr_storage** stack);

s_expr* parse(char str[], environment* env);

#endif

#ifndef FUNDAMENTALS_H
#define FUNDAMENTALS_H

#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>

// types

typedef enum {TYPE_UNDEF, \
              TYPE_ERROR, \
              TYPE_VOID, \
              TYPE_BUILTIN, \
              TYPE_NUM, \
              TYPE_BOOL, \
              TYPE_SEXPR, \
              TYPE_SYMBOL, \
              TYPE_FUNCTION} type;

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
              EVAL_ERROR_ILLEGAL_PAIR, \
              EVAL_ERROR_UNDEF_FUNCTION} interpreter_error;

// s-expressions & typed pointers

// forward declaration
struct S_EXPR_NODE;

typedef union TP_PTR {
    long idx;
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
typed_ptr* create_atom_tp(type type, long idx);
typed_ptr* create_sexpr_tp(s_expr* se_ptr);
typed_ptr* create_error(interpreter_error err_code);
typed_ptr* copy_typed_ptr(const typed_ptr* tp);

s_expr* create_s_expr(typed_ptr* car, typed_ptr* cdr);
s_expr* sexpr_next(const s_expr* se);
s_expr* create_empty_s_expr();
s_expr* copy_s_expr(const s_expr* se);
void delete_se_recursive(s_expr* se, bool delete_sexpr_cars);

bool is_empty_list(const s_expr* se);

#endif

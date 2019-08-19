#ifndef FUNDAMENTALS_H
#define FUNDAMENTALS_H

#include<stdlib.h>
#include<stdio.h>
#include<stdbool.h>

// types

typedef enum {TYPE_UNDEF, \
              TYPE_ERROR, \
              TYPE_VOID, \
              TYPE_FIXNUM, \
              TYPE_BOOL, \
              TYPE_BUILTIN, \
              TYPE_S_EXPR, \
              TYPE_SYMBOL, \
              TYPE_FUNCTION} type;

// built-in functions and special forms

typedef enum {BUILTIN_ADD, \
              BUILTIN_MUL, \
              BUILTIN_SUB, \
              BUILTIN_DIV, \
              BUILTIN_NUMBEREQ, \
              BUILTIN_NUMBERGT, \
              BUILTIN_NUMBERLT, \
              BUILTIN_NUMBERGE, \
              BUILTIN_NUMBERLE, \
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
              BUILTIN_LISTPRED, \
              BUILTIN_PAIRPRED, \
              BUILTIN_NUMBERPRED, \
              BUILTIN_BOOLPRED, \
              BUILTIN_VOIDPRED, \
              BUILTIN_LAMBDA} builtin_code;

// error codes

typedef enum {PARSE_ERROR_NONE, \
              PARSE_ERROR_UNBAL_PAREN, \
              PARSE_ERROR_BARE_SYM, \
              PARSE_ERROR_EMPTY_PAREN, \
              PARSE_ERROR_TOO_MANY, \
              PARSE_ERROR_INT_UNSPEC, \
              PARSE_ERROR_INT_TOO_LOW, \
              PARSE_ERROR_INT_TOO_HIGH, \
              // ^  parsing errors above     ^
              // v  evaluation errors below  v
              EVAL_ERROR_EXIT, \
              EVAL_ERROR_NULL_S_EXPR, \
              EVAL_ERROR_MALFORMED_S_EXPR, \
              EVAL_ERROR_UNDEF_SYM, \
              EVAL_ERROR_UNDEF_TYPE, \
              EVAL_ERROR_UNDEF_BUILTIN, \
              EVAL_ERROR_UNDEF_FUNCTION, \
              EVAL_ERROR_ILLEGAL_PAIR, \
              EVAL_ERROR_FEW_ARGS, \
              EVAL_ERROR_MANY_ARGS, \
              EVAL_ERROR_BAD_ARG_TYPE, \
              EVAL_ERROR_NEED_NUM, \
              EVAL_ERROR_NOT_ID, \
              EVAL_ERROR_FIXNUM_UNDER, \
              EVAL_ERROR_FIXNUM_OVER, \
              EVAL_ERROR_DIV_ZERO, \
              EVAL_ERROR_EMPTY_ELSE, \
              EVAL_ERROR_NONTERMINAL_ELSE, \
              EVAL_ERROR_CAR_NOT_CALLABLE, \
              EVAL_ERROR_MISSING_PROCEDURE, \
              EVAL_ERROR_BAD_SYNTAX, \
              EVAL_ERROR_BAD_SYMBOL} interpreter_error;

// s-expressions & typed pointers

struct S_EXPR;

typedef union TP_VALUE {
    long idx;
    struct S_EXPR* se_ptr;
} tp_value;

typedef struct TYPED_PTR {
    type type;
    tp_value ptr;
} typed_ptr;

typedef struct S_EXPR {
    typed_ptr* car;
    typed_ptr* cdr;
} s_expr;

typed_ptr* create_typed_ptr(type type, tp_value ptr);
typed_ptr* create_atom_tp(type type, long idx);
typed_ptr* create_error_tp(interpreter_error err_code);
typed_ptr* create_void_tp();
typed_ptr* create_s_expr_tp(s_expr* se);
typed_ptr* copy_typed_ptr(const typed_ptr* tp);

s_expr* create_s_expr(typed_ptr* car, typed_ptr* cdr);
s_expr* create_empty_s_expr();
s_expr* copy_s_expr(const s_expr* se);
void delete_s_expr_recursive(s_expr* se, bool delete_s_expr_cars);

s_expr* s_expr_next(const s_expr* se);

bool is_empty_list(const s_expr* se);
bool is_false_literal(const typed_ptr* tp);
bool is_pair(const s_expr* se);

#endif

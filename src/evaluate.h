#ifndef EVALUATE_H
#define EVALUATE_H

#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<limits.h>

#include "fundamentals.h"
#include "environment.h"

typed_ptr* evaluate(const typed_ptr* tp, Environment* env);

// evaluating different types

typed_ptr* eval_builtin(const s_expr* se, Environment* env);
typed_ptr* eval_s_expr(const s_expr* se, Environment* env);
typed_ptr* eval_function(const s_expr* se, Environment* env);

// evaluating built-in functions and special forms

// Evaluates an s-expression, whose car is a built-in function in the set
//   {BUILTIN_ADD, BUILTIN_MUL, BUILTIN_SUB, BUILTIN_DIV}, in the context of the
//   given environment.
// BUILTIN_ADD and BUILTIN_MUL take any number of arguments.
// BUILTIN_SUB and BUILTIN_DIV take at least 1 argument.
// All other s-expression contents are evaluated; the results must be numbers.
// Returns a dynamically-allocated typed_ptr containing the resulting number, or
//   an error code (if the evaluation failed).
typed_ptr* eval_arithmetic(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_NUMBEREQ, BUILTIN_NUMBERGT, BUILTIN_NUMBERLT, BUILTIN_NUMBERGE,
//    BUILTIN_NUMBERLE}, in the context of the given environment.
// These functions all take at least 2 arguments.
// All other s-expression contents are evaluated; the results must be numbers.
// Returns a dynamically-allocated typed_ptr containing the boolean truth value
//   of the expression, or an error code (if the evaluation failed).
typed_ptr* eval_comparison(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_DEFINE, in the context of the given environment.
// This special form takes exactly two arguments.
// This special form may be used to assign a value to a symbol:
//   `(define a 1)`
// It also supports a shorthand for assigning a lambda expression to a symbol:
//   Rather than `(define add (lambda (x y) (+ x y)))`, you can write
//   `(define (add x y) (+ x y))`.
// The first argument may thus be either (1) a symbol, or (2) a list of symbols.
// The second argument may be any valid expression. If the first argument is a
//   symbol, the second argument is evaluated. If the first argument is a list
//   of symbols, the second argument is treated as the body of a lambda
//   expression and not evaluated.
// The symbol's value within the given environment is set to be the result of
//   evaluating the second argument, or the equivalent lambda expression if
//   that mode is used.
// Note that the symbol's value is only set within the provided environment, and
//   that it may shadow a variable in an enclosing scope.
// Returns a dynamically-allocated typed_ptr containing a <void> value, or an
//   error code (if the evaluation failed).
typed_ptr* eval_define(const s_expr* se, Environment* env);

typed_ptr* eval_set_variable(const s_expr* se, Environment* env);
typed_ptr* eval_exit(const s_expr* se, Environment* env);
typed_ptr* eval_cons(const s_expr* se, Environment* env);
typed_ptr* eval_car_cdr(const s_expr* se, Environment* env);
typed_ptr* eval_list_construction(const s_expr* se, Environment* env);
typed_ptr* eval_and_or(const s_expr* se, Environment* env);
typed_ptr* eval_not(const s_expr* se, Environment* env);
typed_ptr* eval_cond(const s_expr* se, Environment* env);
typed_ptr* eval_list_pred(const s_expr* se, Environment* env);
typed_ptr* eval_atom_pred(const s_expr* se, Environment* env);
typed_ptr* eval_null_pred(const s_expr* se, Environment* env);
typed_ptr* eval_lambda(const s_expr* se, Environment* env);
typed_ptr* eval_quote(const s_expr* se, Environment* env);
typed_ptr* eval_string_length(const s_expr* se, Environment* env);
typed_ptr* eval_string_equals(const s_expr* se, Environment* env);
typed_ptr* eval_string_append(const s_expr* se, Environment* env);

// helper functions

Symbol_Node* collect_parameters(typed_ptr* tp, Environment* env);
Symbol_Node* bind_args(Function_Node* fn, typed_ptr* args);
Environment* make_eval_env(Environment* env, Symbol_Node* bound_args);
typed_ptr* collect_arguments(const s_expr* se, \
                             Environment* env, \
                             int min_args, \
                             int max_args, \
                             bool evaluate_all_args);

#endif

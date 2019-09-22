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
//   It is not evaluated.
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

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_SETVAR, in the context of the given environment.
// This special form takes exactly two arguments.
// The first argument must be a symbol. It is not evaluated. It must be defined
//   before it can be mutated.
// The second argument may be any valid expression. It is evaluated.
// The symbol's value within the given environment, or (if the symbol is not
//   defined within the given environment) in its nearest enclosing scope, is
//   set to the result of evaluating the second argument.
// Returns a dynamically-allocated typed_ptr containing a <void> value, or an
//   error code (if the evaluation failed).
typed_ptr* eval_set_variable(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in instruction to exit the
//   interpreter, BUILTIN_EXIT.
// This instruction takes exactly zero arguments.
// Returns a dynamically-allocated typed_ptr containing the error
//   EVAL_ERROR_EXIT, or another error code (if evaluation failed).
typed_ptr* eval_exit(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in function BUILTIN_CONS, in
//   the context of the given environment.
// This function takes exactly two arguments.
// The arguments may be any valid expression. Each is evaluated.
// The arguments are made into a cons cell (a pair).
// Returns a dynamically-allocated typed_ptr to the resulting pair, or an error
//   code (if the evaluation failed).
typed_ptr* eval_cons(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is in the set {BUILTIN_CAR, BUILTIN_CDR},
//   in the context of the given environment.
// This function takes exactly one argument.
// The first argument must be a non-empty s-expression. It is evaluated.
// `car` produces the first element in a pair, `cdr` the second.
// Returns a dynamically-allocated typed_ptr containing the resulting object, or
//   an error code (if the evaluation failed).
typed_ptr* eval_car_cdr(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in function BUILTIN_LIST,
//   in the context of the given environment.
// This function takes any number of arguments.
// The arguments may be any valid expression. Each is evaluated.
// The arguments are made into a list. Providing zero arguments produces the
//   empty list.
// Returns a dynamically-allocated typed_ptr containing the resulting
//   s-expression, or an error code (if the evaluation failed).
typed_ptr* eval_list_construction(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is in the set {BUILTIN_AND, BUILTIN_OR},
//   in the context of the given environment.
// These special forms take any number of arguments.
// Their arguments may be any valid expression.
// The arguments are evaluated in order, until a stopping condition is reached
//   or the arguments are exhausted.
// `and` stops once an argument evaluates to boolean false.
// `or` stops once an argument evaluates to anything but boolean false.
// Returns a dynamically-allocated typed_ptr containing the result of the last
//   argument evaluation performed, or an error code (if the evaluation failed).
typed_ptr* eval_and_or(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in function BUILTIN_NOT,
//   in the context of the given environment.
// This function takes exactly one argument.
// The argument may be any valid expression. It is evaluated.
// Returns a dynamically-allocated typed_ptr containing the boolean value of
//   whether the argument is a boolean false, or an error code (if the
//   evaluation failed).
typed_ptr* eval_not(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_COND, in the context of the given environment.
// This special form takes any number of arguments.
// The arguments must be non-empty lists; i.e., they take the form
//   (predicate [then-body ...]).
// The predicates of the arguments are evaluated until one evaluates to anything
//   other than a boolean false value. After this, no predicates are evaluated.
// For an argument whose predicate evaluates to non-false, its then-bodies are
//   evaluated in order, if present.
// One special predicate, named "else", is allowed, which is a fall-through
//   case. It may only appear as the predicate of the last argument. If an else
//   clause appears, it must have at least one then-body.
// Returns a dynamically-allocated typed_ptr containing the result of the last
//   then-body evaluation (or the predicate, if no then-bodies are present), or
//   a <void> value (if no arguments are provided or no argument's predicate
//   evaluates to non-false), or an error code (if any evaluation failed).
typed_ptr* eval_cond(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in function 
//   BUILTIN_LISTPRED, in the context of the given environment.
// This function takes one argument, which may be any valid expression. It is
//   evaluated.
// Returns a dynamically-allocated typed_ptr containing the boolean truth value
//   of the predicate, or an error code (if the evaluation failed).
typed_ptr* eval_list_pred(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_xxPRED | xx in {PAIR, BOOL, NUMBER, VOID, PROC, SYMBOL, STRING}}.
// This function takes one argument, which may be any valid expression. It is
//   evaluated.
// Returns a dynamically-allocated typed_ptr containing the boolean truth value
//   of the predicate, or an error code (if the evaluation failed).
typed_ptr* eval_atom_pred(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is a built-in function BUILTIN_NULLPRED,
//   in the context of the given environment.
// This function takes one argument, which may be any valid expression. It is
//   evaluated.
// Returns a dynamically-allocated typed_ptr containing the boolean truth value
//   of the predicate, or an error code (if the evaluation failed).
typed_ptr* eval_null_pred(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_LAMBDA, in the context of the given environment.
// This special form takes two arguments, neither of which is evaluated.
// The first argument must be a list of symbols.
// The second argument may be any valid expression.
// The first argument is the lambda's parameter list, and the second its body.
// The lambda is installed into the given environment as a function.
// Returns a dynamically-allocated typed_ptr containing a reference to the
//   installed function, or an error code (if the evaluation failed).
typed_ptr* eval_lambda(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_QUOTE, in the context of the given environment.
// This special form takes one argument, which may be any valid expression.
// The argument is returned, unevaluated.
// Returns a dynamically-allocated typed_ptr containing a copy of the input, or
//   an error code (if the evaluation failed).
typed_ptr* eval_quote(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in function
//   BUILTIN_STRINGLEN, in the context of the given environment.
// This function takes one argument, which must be a string. It is evaluated.
// Returns a dynamically-allocated typed_ptr containing the length of the input
//   string, or an error code (if the evaluation failed).
typed_ptr* eval_string_length(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in function
//   BUILTIN_STRINGEQ, in the context of the given environment.
// This function takes at least two arguments, which must all be strings. Each
//   argument is evaluated.
// Returns a dynamically-allocated typed_ptr containing the boolean truth value
//   of the predicate, or an error code (if the evaluation failed).
typed_ptr* eval_string_equals(const s_expr* se, Environment* env);

// Evaluates an s-expression whose car is the built-in function
//   BUILTIN_STRINGAPPEND, in the context of the given environment.
// This function takes any number of arguments, which must all be strings. Each
//   argument is evaluated.
// Returns a dynamically-allocated typed_ptr containing all of the argument
//   strings concatenated, or an error code (if the evaluation failed).
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

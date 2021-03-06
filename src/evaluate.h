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

typed_ptr* eval_arithmetic(const s_expr* se, Environment* env);
typed_ptr* eval_comparison(const s_expr* se, Environment* env);
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

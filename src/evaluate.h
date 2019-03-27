#ifndef EVALUATE_H
#define EVALUATE_H

#include<stdlib.h>
#include<stdbool.h>
#include<string.h>

#include "fundamentals.h"
#include "environment.h"
#include "grackle_utils.h"

typed_ptr* evaluate(const s_expr* se, environment* env);

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

typed_ptr* eval_lambda(const s_expr* se, environment* env);
sym_tab_node* collect_parameters(typed_ptr* tp, environment* env);

typed_ptr* eval_user_function(const s_expr* se, environment* env);
sym_tab_node* bind_args(environment* env, fun_tab_node* ftn, typed_ptr* args);
environment* make_eval_env(environment* env, sym_tab_node* bound_args);

typed_ptr* collect_args(const s_expr* se, \
                        environment* env, \
                        int min_args, \
                        int max_args, \
                        bool evaluate_all_args);

#endif

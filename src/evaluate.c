#include "evaluate.h"

// Evaluates an s-expression of any kind within the context of the provided
//   environment.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the result (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* evaluate(const typed_ptr* tp, Environment* env) {
    typed_ptr* result = NULL;
    if (tp == NULL) {
        result = create_error_tp(EVAL_ERROR_NULL_S_EXPR);
    } else {
        switch (tp->type) {
            case TYPE_UNDEF:
                result = create_error_tp(EVAL_ERROR_UNDEF_SYM);
                break;
            case TYPE_ERROR: // fall-through
            case TYPE_VOID: // fall-through
            case TYPE_FIXNUM: // fall-through
            case TYPE_BOOL: // fall-through
            case TYPE_BUILTIN: // fall-through
            case TYPE_FUNCTION:
                result = copy_typed_ptr(tp);
                break;
            case TYPE_STRING:
                result = copy_typed_ptr(tp);
                result->ptr.string = create_string(tp->ptr.string->contents);
                break;
            case TYPE_S_EXPR:
                result = eval_s_expr(tp->ptr.se_ptr, env);
                break;
            case TYPE_SYMBOL:
                result = value_lookup_index(env, tp);
                if (result == NULL) {
                    result = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
                }
                break;
            default:
                result = create_error_tp(EVAL_ERROR_UNDEF_TYPE);
                break;
        }
    }
    return result;
}

typed_ptr* eval_builtin(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    switch (se->car->ptr.idx) {
        case BUILTIN_ADD: // fall-through
        case BUILTIN_MUL: // fall-through
        case BUILTIN_SUB: // fall-through
        case BUILTIN_DIV:
            result = eval_arithmetic(se, env);
            break;
        case BUILTIN_NUMBEREQ: // fall-through
        case BUILTIN_NUMBERGT: // fall-through
        case BUILTIN_NUMBERLT: // fall-through
        case BUILTIN_NUMBERGE: // fall-through
        case BUILTIN_NUMBERLE:
            result = eval_comparison(se, env);
            break;
        case BUILTIN_DEFINE:
            result = eval_define(se, env);
            break;
        case BUILTIN_SETVAR:
            result = eval_set_variable(se, env);
            break;
        case BUILTIN_EXIT:
            result = eval_exit(se, env);
            break;
        case BUILTIN_CONS:
            result = eval_cons(se, env);
            break;
        case BUILTIN_CAR: // fall-through
        case BUILTIN_CDR:
            result = eval_car_cdr(se, env);
            break;
        case BUILTIN_LIST:
            result = eval_list_construction(se, env);
            break;
        case BUILTIN_AND: // fall-through
        case BUILTIN_OR:
            result = eval_and_or(se, env);
            break;
        case BUILTIN_NOT:
            result = eval_not(se, env);
            break;
        case BUILTIN_COND:
            result = eval_cond(se, env);
            break;
        case BUILTIN_LISTPRED:
            result = eval_list_pred(se, env);
            break;
        case BUILTIN_PAIRPRED: // fall-through
        case BUILTIN_NUMBERPRED: // fall-through
        case BUILTIN_BOOLPRED: // fall-through
        case BUILTIN_VOIDPRED: // fall-through
        case BUILTIN_PROCPRED: // fall-through
        case BUILTIN_SYMBOLPRED: // fall-through
        case BUILTIN_STRINGPRED:
            result = eval_atom_pred(se, env);
            break;
        case BUILTIN_NULLPRED:
            result = eval_null_pred(se, env);
            break;
        case BUILTIN_LAMBDA:
            result = eval_lambda(se, env);
            break;
        case BUILTIN_QUOTE:
            result = eval_quote(se, env);
            break;
        case BUILTIN_STRINGLEN:
            result = eval_string_length(se, env);
            break;
        case BUILTIN_STRINGEQ:
            result = eval_string_equals(se, env);
            break;
        case BUILTIN_STRINGAPPEND:
            result = eval_string_append(se, env);
            break;
        default:
            result = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
            break;
    }
    return result;
}

typed_ptr* eval_s_expr(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    if (se == NULL) {
        return create_error_tp(EVAL_ERROR_NULL_S_EXPR);
    } else if (is_empty_list(se)) {
        return create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    } else if (se->car == NULL || se->cdr == NULL) {
        return create_error_tp(EVAL_ERROR_MALFORMED_S_EXPR);
    } else {
        typed_ptr* evaluated_car = evaluate(se->car, env);
        switch (evaluated_car->type) {
            case TYPE_ERROR:
                result = copy_typed_ptr(evaluated_car);
                break;
            case TYPE_BUILTIN: {
                s_expr* subbed_se = create_s_expr(evaluated_car, se->cdr);
                result = eval_builtin(subbed_se, env);
                free(subbed_se);
                break;
            }
            case TYPE_FUNCTION: {
                s_expr* subbed_se = create_s_expr(evaluated_car, se->cdr);
                result = eval_function(subbed_se, env);
                free(subbed_se);
                break;
            }
            case TYPE_STRING:
                delete_string(evaluated_car->ptr.string);
                result = create_error_tp(EVAL_ERROR_CAR_NOT_CALLABLE);
                break;
            case TYPE_S_EXPR:
                delete_s_expr_recursive(evaluated_car->ptr.se_ptr, true);
                result = create_error_tp(EVAL_ERROR_CAR_NOT_CALLABLE);
                break;
            default:
                result = create_error_tp(EVAL_ERROR_CAR_NOT_CALLABLE);
                break;
        }
        free(evaluated_car);
    }
    return result;
}

// Evaluates an s-expression whose car has type TYPE_FUNCTION.
// The s-expression's cdr must contain the proper number of arguments for the
//   user function being invoked. Any mismatch, or error arising during
//   evaluation of any of the members of the cdr, returns an error.
// Returns a typed_ptr containing an error code (if any argument evaluation
//   failed, or if an error arising during evaluation of the function body) or
//   the result of evaluating the function body using the provided arguments.
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_function(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    Function_Node* fn = function_lookup_index(env, se->car);
    if (fn == NULL) {
        return create_error_tp(EVAL_ERROR_UNDEF_FUNCTION);
    }
    typed_ptr* args_tp = collect_arguments(se, env, 0, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        Symbol_Node* arg_vals = bind_args(fn, args_tp);
        if (arg_vals != NULL && arg_vals->type == TYPE_ERROR) {
            result = create_error_tp(arg_vals->value.idx);
        } else {
            Environment* bound_env = make_eval_env(fn->enclosing_env, arg_vals);
            result = evaluate(fn->body, bound_env);
            bound_env->env_tracker_next = env->global_env->env_tracker_next;
            env->global_env->env_tracker_next = bound_env;
        }
        delete_symbol_node_list(arg_vals);
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

typed_ptr* eval_arithmetic(const s_expr* se, Environment* env) {
    builtin_code op = se->car->ptr.idx;
    int min_args = (op == BUILTIN_ADD || op == BUILTIN_MUL) ? 0 : 1;
    typed_ptr* args_tp = collect_arguments(se, env, min_args, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    long value = (op == BUILTIN_ADD || op == BUILTIN_SUB) ? 0 : 1;
    typed_ptr* result = NULL;
    s_expr* arg = args_tp->ptr.se_ptr;
    if ((op == BUILTIN_SUB || op == BUILTIN_DIV) && \
        !is_empty_list(s_expr_next(arg))) {
        if (arg->car->type != TYPE_FIXNUM) {
            result = create_error_tp(EVAL_ERROR_NEED_NUM);
        } else {
            value = arg->car->ptr.idx;
            arg = s_expr_next(arg);
        }
    }
    for ( ; \
         !is_empty_list(arg) && result == NULL; \
         arg = s_expr_next(arg)) {
        if (arg->car->type != TYPE_FIXNUM) {
            result = create_error_tp(EVAL_ERROR_NEED_NUM);
            break;
        }
        switch (op) {
            case BUILTIN_ADD:
                if (arg->car->ptr.idx < 0 && \
                    value < (LONG_MIN - arg->car->ptr.idx)) {
                    result = create_error_tp(EVAL_ERROR_FIXNUM_UNDER);
                } else if (arg->car->ptr.idx > 0 && \
                           value > (LONG_MAX - arg->car->ptr.idx)) {
                    result = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
                } else {
                    value += arg->car->ptr.idx;
                }
                break;
            case BUILTIN_SUB:
                if (arg->car->ptr.idx > 0 && \
                    value < (LONG_MIN + arg->car->ptr.idx)) {
                    result = create_error_tp(EVAL_ERROR_FIXNUM_UNDER);
                } else if (arg->car->ptr.idx < 0 && \
                           value > (LONG_MAX + arg->car->ptr.idx)) {
                    result = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
                } else {
                    value -= arg->car->ptr.idx;
                }
                break;
            case BUILTIN_MUL:
                if ((value < 0 && \
                     arg->car->ptr.idx > 0 && \
                     value < LONG_MIN / arg->car->ptr.idx) || \
                    (value > 0 && \
                     arg->car->ptr.idx < 0 && \
                     arg->car->ptr.idx < LONG_MIN / value)) {
                    result = create_error_tp(EVAL_ERROR_FIXNUM_UNDER);
                } else if ((value < 0 && \
                            arg->car->ptr.idx < 0 && \
                            arg->car->ptr.idx < LONG_MAX / value) || \
                           (value > 0 && \
                            arg->car->ptr.idx > 0 && \
                            value > LONG_MAX / arg->car->ptr.idx)) {
                    result = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
                } else {
                    value *= arg->car->ptr.idx;
                }
                break;
            case BUILTIN_DIV:
                if (arg->car->ptr.idx == 0) {
                    result = create_error_tp(EVAL_ERROR_DIV_ZERO);
                } else if (value == LONG_MIN && \
                           arg->car->ptr.idx == -1) {
                    result = create_error_tp(EVAL_ERROR_FIXNUM_OVER);
                } else {
                    value /= arg->car->ptr.idx;
                }
                break;
            default:
                result = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
                break;
        }
    }
    if (result == NULL) {
        result = create_atom_tp(TYPE_FIXNUM, value);
    } // otherwise there is an error to return
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_comparison(const s_expr* se, Environment* env) {
    builtin_code op = se->car->ptr.idx;
    typed_ptr* args_tp = collect_arguments(se, env, 2, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = NULL;
    s_expr* arg = args_tp->ptr.se_ptr;
    if (arg->car->type != TYPE_FIXNUM) {
        result = create_error_tp(EVAL_ERROR_NEED_NUM);
    } else {
        bool truth = true;
        long last_num = arg->car->ptr.idx;
        for (arg = s_expr_next(arg) ; \
             !is_empty_list(arg) && truth == true && result == NULL; \
             arg = s_expr_next(arg)) {
            if (arg->car->type != TYPE_FIXNUM) {
                result = create_error_tp(EVAL_ERROR_NEED_NUM);
                break;
            }
            switch (op) {
                case BUILTIN_NUMBEREQ:
                    truth = last_num == arg->car->ptr.idx;
                    break;
                case BUILTIN_NUMBERGT:
                    truth = last_num > arg->car->ptr.idx;
                    break;
                case BUILTIN_NUMBERLT:
                    truth = last_num < arg->car->ptr.idx;
                    break;
                case BUILTIN_NUMBERGE:
                    truth = last_num >= arg->car->ptr.idx;
                    break;
                case BUILTIN_NUMBERLE:
                    truth = last_num <= arg->car->ptr.idx;
                    break;
                default:
                    result = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
                    break;
            }
            last_num = arg->car->ptr.idx;
        }
        if (result == NULL) {
            result = create_atom_tp(TYPE_BOOL, truth);
        } // otherwise there is an error to return
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_define(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = NULL;
    typed_ptr* first_arg = args_tp->ptr.se_ptr->car;
    typed_ptr* second_arg = s_expr_next(args_tp->ptr.se_ptr)->car;
    if (first_arg->type == TYPE_SYMBOL) { // define variable
        Symbol_Node* sym_node = symbol_lookup_index(env->global_env, first_arg);
        if (sym_node == NULL) {
            result = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
        } else {
            typed_ptr* value = evaluate(second_arg, env);
            if (value->type == TYPE_ERROR) {
                result = value;
            } else {
                blind_install_symbol(env, sym_node->name, value);
                free(value);
                result = create_void_tp();
            }
        }
    } else if (first_arg->type == TYPE_S_EXPR) { // define function
        if (is_empty_list(first_arg->ptr.se_ptr)) {
            result = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
        } else if (first_arg->ptr.se_ptr->car->type != TYPE_SYMBOL) {
            result = create_error_tp(EVAL_ERROR_NOT_SYMBOL);
        } else {
            typed_ptr* fn_sym = first_arg->ptr.se_ptr->car;
            Symbol_Node* sym_node = symbol_lookup_index(env->global_env, \
                                                         fn_sym);
            if (sym_node == NULL) {
                result = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
            } else {
                // create a dummy (lambda <params> <body>) s-expression
                typed_ptr* lambda = create_atom_tp(TYPE_BUILTIN, \
                                                   BUILTIN_LAMBDA);
                s_expr* params = s_expr_next(first_arg->ptr.se_ptr);
                typed_ptr* body = copy_typed_ptr(second_arg);
                if (body->type == TYPE_S_EXPR) {
                    body->ptr.se_ptr = copy_s_expr(body->ptr.se_ptr);
                } else if (body->type == TYPE_STRING) {
                    char* contents = body->ptr.string->contents;
                    body->ptr.string = create_string(contents);
                }
                s_expr* lambda_se = unit_list(lambda);
                s_expr_append(lambda_se, create_s_expr_tp(copy_s_expr(params)));
                s_expr_append(lambda_se, body);
                typed_ptr* fn = eval_lambda(lambda_se, env);
                delete_s_expr_recursive(lambda_se, true);
                if (fn->type == TYPE_ERROR) {
                    result = fn;
                } else {
                    blind_install_symbol(env, sym_node->name, fn);
                    Function_Node* fn_fn = function_lookup_index(env, fn);
                    free(fn_fn->name);
                    fn_fn->name = strdup(sym_node->name);
                    free(fn);
                    result = create_void_tp();
                }
            }
        }
    } else {
        result = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
    free(args_tp);
    return result;
}

typed_ptr* eval_set_variable(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = NULL;
    typed_ptr* first_arg = args_tp->ptr.se_ptr->car;
    typed_ptr* second_arg = s_expr_next(args_tp->ptr.se_ptr)->car;
    if (first_arg->type != TYPE_SYMBOL) {
        result = create_error_tp(EVAL_ERROR_NOT_SYMBOL);
    } else {
        Symbol_Node* found = symbol_lookup_index(env, first_arg);
        while (found == NULL && env->enclosing_env != NULL) {
            env = env->enclosing_env;
            found = symbol_lookup_index(env, first_arg);
        }
        if (found == NULL) {
            result = create_error_tp(EVAL_ERROR_BAD_SYMBOL);
        } else if (found->type == TYPE_UNDEF) {
            result = create_error_tp(EVAL_ERROR_UNDEF_SYM);
        } else {
            typed_ptr* value = evaluate(second_arg, env);
            if (value->type == TYPE_ERROR) {
                result = value;
            } else {
                if (found->type == TYPE_S_EXPR) {
                    delete_s_expr_recursive(found->value.se_ptr, true);
                } else if (found->type == TYPE_STRING) {
                    delete_string(found->value.string);
                }
                found->type = value->type;
                found->value = value->ptr;
                free(value);
                result = create_void_tp();
            }
        }
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
    free(args_tp);
    return result;
}

typed_ptr* eval_exit(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 0, 0, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = NULL;
    result = create_error_tp(EVAL_ERROR_EXIT);
    delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
    free(args_tp);
    return result;
}

typed_ptr* eval_cons(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 2, 2, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = NULL;
    typed_ptr* first_arg = args_tp->ptr.se_ptr->car;
    typed_ptr* second_arg = s_expr_next(args_tp->ptr.se_ptr)->car;
    result = create_s_expr_tp(create_s_expr(first_arg, second_arg));
    args_tp->ptr.se_ptr->car = NULL;
    s_expr_next(args_tp->ptr.se_ptr)->car = NULL;
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_car_cdr(const s_expr* se, Environment* env) {
    builtin_code op = se->car->ptr.idx;
    typed_ptr* args_tp = collect_arguments(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = NULL;
    typed_ptr* arg = args_tp->ptr.se_ptr->car;
    if (arg->type != TYPE_S_EXPR || is_empty_list(arg->ptr.se_ptr)) {
        result = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    } else if (op == BUILTIN_CAR) {
        result = arg->ptr.se_ptr->car;
        arg->ptr.se_ptr->car = NULL;
    } else if (op == BUILTIN_CDR) {
        result = arg->ptr.se_ptr->cdr;
        arg->ptr.se_ptr->cdr = NULL;
    } else {
        result = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_list_construction(const s_expr* se, Environment* env) {
    return collect_arguments(se, env, 0, -1, true);
}

typed_ptr* eval_and_or(const s_expr* se, Environment* env) {
    builtin_code op = se->car->ptr.idx;
    if (op != BUILTIN_AND && op != BUILTIN_OR) {
        return create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
    }
    typed_ptr* args_tp = collect_arguments(se, env, 0, -1, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = NULL;
    // the "starting" typed-pointer and the test in the while-loop are the
    // only ways in which "and" and "or" differ
    typed_ptr* start_tp = create_atom_tp(TYPE_BOOL, op == BUILTIN_AND);
    s_expr* arg_se = create_s_expr(start_tp, args_tp);
    typed_ptr* evaluated_arg = copy_typed_ptr(start_tp);
    for (s_expr* curr_se = arg_se; \
         !is_empty_list(curr_se); \
         curr_se = s_expr_next(curr_se)) {
        if (evaluated_arg->type == TYPE_S_EXPR) {
            delete_s_expr_recursive(evaluated_arg->ptr.se_ptr, true);
        } else if (evaluated_arg->type == TYPE_STRING) {
            delete_string(evaluated_arg->ptr.string);
        }
        free(evaluated_arg);
        evaluated_arg = evaluate(curr_se->car, env);
        if (evaluated_arg->type == TYPE_ERROR || \
            is_false_literal(evaluated_arg) == start_tp->ptr.idx) {
            break;
        }
    }
    result = evaluated_arg;
    delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
    free(args_tp);
    free(arg_se->car);
    free(arg_se);
    return result;
}

typed_ptr* eval_not(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* arg = args_tp->ptr.se_ptr->car;
    typed_ptr* result = create_atom_tp(TYPE_BOOL, is_false_literal(arg));
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_cond(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 0, -1, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    for (s_expr* lookahead = args_tp->ptr.se_ptr; \
         !is_empty_list(lookahead); \
         lookahead = s_expr_next(lookahead)) {
        if (lookahead->car->type != TYPE_S_EXPR || \
            is_empty_list(lookahead->car->ptr.se_ptr)) {
            delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
            free(args_tp);
            return create_error_tp(EVAL_ERROR_BAD_SYNTAX);
        }
    }
    typed_ptr* pred_result = create_void_tp();
    bool pred_true = false;
    s_expr* then_bodies = NULL;
    for (s_expr* arg_se = s_expr_next(se); \
         !is_empty_list(arg_se) && !pred_true; \
         arg_se = s_expr_next(arg_se)) {
        s_expr* cond_clause = arg_se->car->ptr.se_ptr;
        typed_ptr* predicate = cond_clause->car;
        then_bodies = s_expr_next(cond_clause);
        Symbol_Node* else_stn = symbol_lookup_name(env->global_env, "else");
        if (predicate->type == TYPE_SYMBOL && \
            predicate->ptr.idx == else_stn->symbol_idx) {
            s_expr* next_cond_clause = s_expr_next(arg_se);
            if (!is_empty_list(next_cond_clause)) {
                free(pred_result);
                pred_result = create_error_tp(EVAL_ERROR_NONTERMINAL_ELSE);
                break;
            }
            if (is_empty_list(then_bodies)) {
                free(pred_result);
                pred_result = create_error_tp(EVAL_ERROR_EMPTY_ELSE);
                break;
            }
            pred_true = true;
        }
        free(pred_result);
        pred_result = evaluate(predicate, env);
        if (pred_result->type == TYPE_ERROR) {
            break;
        } else if (!is_false_literal(pred_result)) {
            pred_true = true;
        }
    }
    typed_ptr* result = NULL;
    if (pred_result->type == TYPE_ERROR) {
        result = pred_result;
    } else if (!pred_true) {
        free(pred_result);
        result = create_void_tp();
    } else {
        typed_ptr* last_then_eval = pred_result;
        while (!is_empty_list(then_bodies)) {
            if (last_then_eval != NULL) {
                if (last_then_eval->type == TYPE_S_EXPR) {
                    delete_s_expr_recursive(last_then_eval->ptr.se_ptr, true);
                } else if (last_then_eval->type == TYPE_STRING) {
                    delete_string(last_then_eval->ptr.string);
                }
            }
            free(last_then_eval);
            last_then_eval = evaluate(then_bodies->car, env);
            if (last_then_eval->type == TYPE_ERROR) {
                break;
            }
            then_bodies = s_expr_next(then_bodies);
        }
        result = last_then_eval;
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
    free(args_tp);
    return result;
}

typed_ptr* eval_list_pred(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* arg = args_tp->ptr.se_ptr->car;
    typed_ptr* result = create_atom_tp(TYPE_BOOL, true);
    if (arg->type != TYPE_S_EXPR) {
        result->ptr.idx = false;
    } else {
        for (s_expr* arg_se = arg->ptr.se_ptr; \
             !is_empty_list(arg_se); \
             arg_se = s_expr_next(arg_se)) {
            if (arg_se->cdr->type != TYPE_S_EXPR) {
                result->ptr.idx = false;
                break;
            }
        }
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_atom_pred(const s_expr* se, Environment* env) {
    type target_type = TYPE_UNDEF;
    switch (se->car->ptr.idx) {
        case BUILTIN_PAIRPRED:
            target_type = TYPE_S_EXPR;
            break;
        case BUILTIN_NUMBERPRED:
            target_type = TYPE_FIXNUM;
            break;
        case BUILTIN_BOOLPRED:
            target_type = TYPE_BOOL;
            break;
        case BUILTIN_VOIDPRED:
            target_type = TYPE_VOID;
            break;
        case BUILTIN_PROCPRED:
            target_type = TYPE_BUILTIN; // or TYPE_FUNCTION - see below
            break;
        case BUILTIN_SYMBOLPRED:
            target_type = TYPE_SYMBOL;
            break;
        case BUILTIN_STRINGPRED:
            target_type = TYPE_STRING;
            break;
        default:
            return create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
    }
    typed_ptr* args_tp = collect_arguments(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* arg = args_tp->ptr.se_ptr->car;
    // general case
    typed_ptr* result = create_atom_tp(TYPE_BOOL, arg->type == target_type);
    // special case: (procedure? +) -> #t AND (procedure? <user-fn>) -> #t
    if (target_type == TYPE_BUILTIN && arg->type == TYPE_FUNCTION) {
        result->ptr.idx = true;
    }
    // special case: (pair? '()) -> #f
    if (arg->type == TYPE_S_EXPR && is_empty_list(arg->ptr.se_ptr)) {
        result->ptr.idx = false;
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_null_pred(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* arg = args_tp->ptr.se_ptr->car;
    typed_ptr* result = create_atom_tp(TYPE_BOOL,
                                       (arg->type == TYPE_S_EXPR && \
                                        is_empty_list(arg->ptr.se_ptr)));
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_lambda(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* first_arg = args_tp->ptr.se_ptr->car;
    typed_ptr* second_arg = s_expr_next(args_tp->ptr.se_ptr)->car;
    typed_ptr* result = NULL;
    if (first_arg->type != TYPE_S_EXPR) {
        result = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
    } else {
        Symbol_Node* params = collect_parameters(first_arg, env);
        if (params != NULL && params->type == TYPE_ERROR) {
            result = create_error_tp(params->value.idx);
            delete_symbol_node_list(params);
        } else {
            typed_ptr* body = copy_typed_ptr(second_arg);
            if (body->type == TYPE_S_EXPR) {
                body->ptr.se_ptr = copy_s_expr(body->ptr.se_ptr);
            } else if (body->type == TYPE_STRING) {
                char* contents = body->ptr.string->contents;
                body->ptr.string = create_string(contents);
            }
            result = install_function(env, "", params, env, body);
        }
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
    free(args_tp);
    return result;
}

typed_ptr* eval_quote(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 1, 1, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* result = copy_typed_ptr(args_tp->ptr.se_ptr->car);
    if (result->type == TYPE_S_EXPR) {
        result->ptr.se_ptr = copy_s_expr(result->ptr.se_ptr);
    } else if (result->type == TYPE_STRING) {
        char* contents = result->ptr.string->contents;
        result->ptr.string = create_string(contents);
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
    free(args_tp);
    return result;
}

typed_ptr* eval_string_length(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    typed_ptr* arg = args_tp->ptr.se_ptr->car;
    typed_ptr* result = NULL;
    if (arg->type != TYPE_STRING) {
        result = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    } else {
        result = create_atom_tp(TYPE_FIXNUM, arg->ptr.string->len);
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_string_equals(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 2, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    s_expr* arg_se = args_tp->ptr.se_ptr;
    typed_ptr* first_arg = arg_se->car;
    typed_ptr* result = NULL;
    if (first_arg->type != TYPE_STRING) {
        result = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    } else {
        result = create_atom_tp(TYPE_BOOL, true);
        for (arg_se = s_expr_next(arg_se); \
             result->ptr.idx == true && !is_empty_list(arg_se); \
             arg_se = s_expr_next(arg_se)) {
            typed_ptr* curr_arg = arg_se->car;
            if (curr_arg->type != TYPE_STRING) {
                free(result);
                result = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
                break;
            }
            if (first_arg->ptr.string->len != curr_arg->ptr.string->len || \
                strcmp(first_arg->ptr.string->contents, \
                        curr_arg->ptr.string->contents)) {
                result->ptr.idx = false;
            }
        }
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

typed_ptr* eval_string_append(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_arguments(se, env, 0, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    }
    bool all_strings = true;
    long total_length = 0;
    s_expr* arg_se = args_tp->ptr.se_ptr;
    for ( ; !is_empty_list(arg_se); arg_se = s_expr_next(arg_se)) {
        if (arg_se->car->type != TYPE_STRING) {
            all_strings = false;
            break;
        }
        total_length += arg_se->car->ptr.string->len;
    }
    typed_ptr* result = NULL;
    if (all_strings) {
        result = create_string_tp(create_string(""));
        if (total_length > 0) {
            char* new_str = malloc(sizeof(char) * (total_length + 1));
            if (new_str == NULL) {
                fprintf(stderr, "malloc failed in eval_string_append()\n");
                exit(-1);
            }
            char* start = new_str;
            arg_se = args_tp->ptr.se_ptr;
            for ( ; !is_empty_list(arg_se); arg_se = s_expr_next(arg_se)) {
                String* curr_string = arg_se->car->ptr.string;
                memcpy(start, curr_string->contents, curr_string->len);
                start += curr_string->len;
            }
            new_str[total_length] = '\0';
            free(result->ptr.string->contents);
            result->ptr.string->contents = new_str;
            result->ptr.string->len = total_length;
        }
    } else {
        result = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
    }
    delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
    free(args_tp);
    return result;
}

// At first, we will restrict user-defined functions to have a finite number of
//   parameters.
// tp is expected to be a pointer to an s-expression. If it's empty, NULL is
//   returned.
// If any element of the s-expression pointed to by tp is not a symbol, a
//   single Symbol_Node containing an error code is returned.
// Otherwise, a list of Symbol_Nodes is returned, one for each parameter
//   symbol discovered.
// In either case, these Symbol_Nodes are safe to free (and they are the
//   caller's responsibility to free).
Symbol_Node* collect_parameters(typed_ptr* tp, Environment* env) {
    Symbol_Node* params = NULL;
    s_expr* se = tp->ptr.se_ptr;
    if (is_empty_list(se)) {
        return params;
    }
    Environment* global_env = env;
    while (global_env->enclosing_env != NULL) {
        global_env = global_env->enclosing_env;
    }
    if (se->car == NULL || se->car->type != TYPE_SYMBOL) {
        params = create_error_symbol_node(EVAL_ERROR_NOT_SYMBOL);
    } else if (se->cdr == NULL || se->cdr->type != TYPE_S_EXPR) {
        params = create_error_symbol_node(EVAL_ERROR_BAD_ARG_TYPE);
    } else {
        Symbol_Node* found = symbol_lookup_index(global_env, se->car);
        if (found == NULL) {
            params = create_error_symbol_node(EVAL_ERROR_BAD_SYMBOL);
        } else {
            params = create_symbol_node(0, \
                                        found->name, \
                                        TYPE_UNDEF, \
                                        (tp_value){.idx=0});
            Symbol_Node* curr = params;
            se = s_expr_next(se);
            while (!is_empty_list(se)) {
                if (se->cdr == NULL || se->cdr->type != TYPE_S_EXPR) {
                    delete_symbol_node_list(params);
                    params = create_error_symbol_node(EVAL_ERROR_BAD_ARG_TYPE);
                    break;
                }
                if (se->car == NULL || se->car->type != TYPE_SYMBOL) {
                    delete_symbol_node_list(params);
                    params = create_error_symbol_node(EVAL_ERROR_NOT_SYMBOL);
                    break;
                }
                found = symbol_lookup_index(global_env, se->car);
                if (found == NULL) {
                    delete_symbol_node_list(params);
                    params = create_error_symbol_node(EVAL_ERROR_BAD_SYMBOL);
                    break;
                }
                curr->next = create_symbol_node(0, \
                                                found->name, \
                                                TYPE_UNDEF, \
                                                (tp_value){.idx=0});
                curr = curr->next;
                se = s_expr_next(se);
            }
        }
    }
    return params;
}

// If the Function_Node's arg list is of different length than the s-expression
//   pointed to by the args typed pointer, an error is returned in the first
//   Symbol_Node.
// Otherwise, the parameters in the arg list are bound to the values stored in
//   the members of the args s-expression. Any s-expressions pointed to by the
//   typed-pointers in the arg list are now the responsibility of the
//   Symbol_Nodes returned.
// If no errors are encountered, the Symbol_Node list contains the bound
//   arguments.
// In all cases, the Symbol_Node list returned is the caller's responsibility
//   to free, and may be safely (shallow) freed.
Symbol_Node* bind_args(Function_Node* fn, typed_ptr* args) {
    if (fn->param_list == NULL && is_empty_list(args->ptr.se_ptr)) {
        return NULL;
    } else if (is_empty_list(args->ptr.se_ptr)) {
        return create_error_symbol_node(EVAL_ERROR_FEW_ARGS);
    } else if (fn->param_list == NULL) {
        return create_error_symbol_node(EVAL_ERROR_MANY_ARGS);
    } else {
        Symbol_Node* curr_param = fn->param_list;
        s_expr* arg_se = args->ptr.se_ptr;
        Symbol_Node* bound_args = NULL;
        bound_args = create_symbol_node(0, \
                                        curr_param->name, \
                                        arg_se->car->type, \
                                        arg_se->car->ptr);
        if (bound_args->type == TYPE_S_EXPR) {
            bound_args->value.se_ptr = copy_s_expr(bound_args->value.se_ptr);
        } else if (bound_args->type == TYPE_STRING) {
            char* contents = bound_args->value.string->contents;
            bound_args->value.string = create_string(contents);
        }
        curr_param = curr_param->next;
        arg_se = s_expr_next(arg_se);
        while (!is_empty_list(arg_se)) {
            if (curr_param == NULL) {
                delete_symbol_node_list(bound_args);
                bound_args = create_error_symbol_node(EVAL_ERROR_MANY_ARGS);
                break;
            }
            Symbol_Node* new_arg = create_symbol_node(0, \
                                                      curr_param->name,\
                                                      arg_se->car->type, \
                                                      arg_se->car->ptr);
            if (new_arg->type == TYPE_S_EXPR) {
                new_arg->value.se_ptr = copy_s_expr(new_arg->value.se_ptr);
            } else if (new_arg->type == TYPE_STRING) {
                char* contents = new_arg->value.string->contents;
                new_arg->value.string = create_string(contents);
            }
            new_arg->next = bound_args;
            bound_args = new_arg;
            curr_param = curr_param->next;
            arg_se = s_expr_next(arg_se);
        }
        if (curr_param != NULL) {
            delete_symbol_node_list(bound_args);
            bound_args = create_error_symbol_node(EVAL_ERROR_FEW_ARGS);
        }
        return bound_args;
    }
}

// Reads a list of bound arguments into an environment, returning the result.
// The input environment is not modified.
// Any s-expressions pointed to in the bound arguments now belong to the
//   returned closure environment.
// The returned environment is the caller's responsibility to delete.
Environment* make_eval_env(Environment* env, Symbol_Node* bound_args) {
    Environment* eval_env = create_environment(0, 0, env);
    Symbol_Node* curr_arg = bound_args;
    while (curr_arg != NULL) {
        typed_ptr curr_value = {.type=curr_arg->type, .ptr=curr_arg->value};
        blind_install_symbol(eval_env, curr_arg->name, &curr_value);
        curr_arg = curr_arg->next;
    }
    return eval_env;
}

typed_ptr* collect_arguments(const s_expr* se, \
                             Environment* env, \
                             int min_args, \
                             int max_args, \
                             bool evaluate_all_args) {
    int seen = 0;
    if (is_pair(se)) {
        return create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
    }
    s_expr* curr = s_expr_next(se);
    s_expr* arg_head = create_empty_s_expr();
    s_expr* arg_tail = arg_head;
    typed_ptr* err = NULL;
    while (!is_empty_list(curr)) {
        if (is_pair(curr)) {
            err = create_error_tp(EVAL_ERROR_ILLEGAL_PAIR);
            break;
        }
        seen++;
        if (max_args >= 0 && seen > max_args) {
            err = create_error_tp(EVAL_ERROR_MANY_ARGS);
            break;
        }
        arg_tail->car = copy_typed_ptr(curr->car);
        arg_tail->cdr = create_s_expr_tp(create_empty_s_expr());
        if (evaluate_all_args && \
            arg_tail->car->type != TYPE_BUILTIN && \
            arg_tail->car->type != TYPE_FUNCTION) {
            typed_ptr* temp = arg_tail->car;
            arg_tail->car = evaluate(arg_tail->car, env);
            free(temp);
        }
        if (!evaluate_all_args && arg_tail->car->type == TYPE_STRING) {
            char* contents = arg_tail->car->ptr.string->contents;
            arg_tail->car->ptr.string = create_string(contents);
        }
        if (arg_tail->car->type == TYPE_ERROR) {
            err = copy_typed_ptr(arg_tail->car);
            break;
        }
        arg_tail = s_expr_next(arg_tail);
        curr = s_expr_next(curr);
    }
    if (err == NULL && seen < min_args) {
        err = create_error_tp(EVAL_ERROR_FEW_ARGS);
    }
    if (err != NULL) {
        delete_s_expr_recursive(arg_head, evaluate_all_args);
        return err;
    } else {
        return create_s_expr_tp(arg_head);
    }
}

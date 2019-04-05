#include "evaluate.h"

// Evaluates an s-expression of any kind within the context of the provided
//   environment.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the result (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* evaluate(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    if (se == NULL) {
        result = create_error_tp(EVAL_ERROR_NULL_SEXPR);
    } else if (is_empty_list(se)) {
        result = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    } else if (se->car == NULL || se->cdr == NULL) {
        result = create_error_tp(EVAL_ERROR_MALFORMED_SEXPR);
    } else {
        switch (se->car->type) {
            case TYPE_UNDEF:
                result = create_error_tp(EVAL_ERROR_UNDEF_SYM);
                break;
            case TYPE_ERROR:
                result = se->car;
                break;
            case TYPE_BUILTIN: 
                result = eval_builtin(se, env);
                break;
            case TYPE_SEXPR: 
                result = eval_sexpr(se, env);
                break;
            case TYPE_FUNCTION:
                result = eval_user_function(se, env);
                break;
            case TYPE_SYMBOL:
                result = value_lookup_index(env, se->car);
                break;
            case TYPE_NUM:  // fall-through
            case TYPE_BOOL: // fall-through
            case TYPE_VOID:
                result = copy_typed_ptr(se->car);
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
        case BUILTIN_PAIRPRED:
            result = eval_atom_pred(se, env, TYPE_SEXPR);
            break;
        case BUILTIN_NUMBERPRED:
            result = eval_atom_pred(se, env, TYPE_NUM);
            break;
        case BUILTIN_BOOLPRED:
            result = eval_atom_pred(se, env, TYPE_BOOL);
            break;
        case BUILTIN_VOIDPRED:
            result = eval_atom_pred(se, env, TYPE_VOID);
            break;
        case BUILTIN_NUMBEREQ: // fall-through
        case BUILTIN_NUMBERGT: // fall-through
        case BUILTIN_NUMBERLT: // fall-through
        case BUILTIN_NUMBERGE: // fall-through
        case BUILTIN_NUMBERLE:
            result = eval_comparison(se, env);
            break;
        case BUILTIN_LAMBDA:
            result = eval_lambda(se, env);
            break;
        default:
            result = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
            break;
    }
    return result;
}

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_xxx | xxx in {ADD, MUL, SUB, DIV}}.
// BUILTIN_ADD and BUILTIN_MUL take any number of arguments.
// BUILTIN_SUB and BUILTIN_DIV take at least 1 argument.
// All arguments are expected to (evaluate to) be numbers.
// Every argument to the built-in function is evaluated.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting number (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_arithmetic(const s_expr* se, Environment* env) {
    builtin_code op = se->car->ptr.idx;
    typed_ptr* result = NULL;
    int min_args = (op == BUILTIN_ADD || op == BUILTIN_MUL) ? 0 : 1;
    typed_ptr* args_tp = collect_args(se, env, min_args, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        long initial = (op == BUILTIN_ADD || op == BUILTIN_SUB) ? 0 : 1;
        result = create_atom_tp(TYPE_NUM, initial);
        s_expr* arg = args_tp->ptr.se_ptr;
        if ((op == BUILTIN_SUB || op == BUILTIN_DIV) && \
            !is_empty_list(s_expr_next(arg))) {
            result->ptr.idx = arg->car->ptr.idx;
            arg = s_expr_next(arg);
        }
        while (!is_empty_list(arg)) {
            if (arg->car->type != TYPE_NUM) {
                free(result);
                result = create_error_tp(EVAL_ERROR_NEED_NUM);
                break;
            }
            switch (op) {
                case BUILTIN_ADD:
                    result->ptr.idx += arg->car->ptr.idx;
                    break;
                case BUILTIN_SUB:
                    result->ptr.idx -= arg->car->ptr.idx;
                    break;
                case BUILTIN_MUL:
                    result->ptr.idx *= arg->car->ptr.idx;
                    break;
                case BUILTIN_DIV:
                    if (arg->car->ptr.idx == 0) {
                        free(result);
                        result = create_error_tp(EVAL_ERROR_DIV_ZERO);
                    } else {
                        result->ptr.idx /= arg->car->ptr.idx;
                    }
                    break;
                default:
                    free(result);
                    result = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
                    break;
            }
            if (result->type == TYPE_ERROR) {
                break;
            }
            arg = s_expr_next(arg);
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_NUMBERxx | xx in {EQ, GT, LT, GE, LE}}.
// These functions take at least 1 argument.
// All arguments are expected to (evaluate to) be numbers.
// Every argument to the built-in function is evaluated.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the (boolean) truth value of the expression (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_comparison(const s_expr* se, Environment* env) {
    builtin_code op = se->car->ptr.idx;
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        s_expr* arg_se = args_tp->ptr.se_ptr;
        if (arg_se->car->type != TYPE_NUM) {
            result = create_error_tp(EVAL_ERROR_NEED_NUM);
        } else {
            long truth = 1;
            long last_num = arg_se->car->ptr.idx;
            arg_se = s_expr_next(arg_se);
            while (!is_empty_list(arg_se) && truth == 1) {
                if (arg_se->car->type != TYPE_NUM) {
                    result = create_error_tp(EVAL_ERROR_NEED_NUM);
                    break;
                }
                switch (op) {
                    case BUILTIN_NUMBEREQ:
                        truth = last_num == arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERGT:
                        truth = last_num > arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERLT:
                        truth = last_num < arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERGE:
                        truth = last_num >= arg_se->car->ptr.idx;
                        break;
                    case BUILTIN_NUMBERLE:
                        truth = last_num <= arg_se->car->ptr.idx;
                        break;
                    default:
                        result = create_error_tp(EVAL_ERROR_UNDEF_BUILTIN);
                        break;
                }
                if (result != NULL) {
                    break;
                }
                last_num = arg_se->car->ptr.idx;
                arg_se = s_expr_next(arg_se);
            }
            if (result == NULL) {
                result = create_atom_tp(TYPE_BOOL, truth);
            } // otherwise it threw an error
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_DEFINE.
// This special form takes exactly two arguments.
// The first argument is expected to be a symbol name, and not evaluated.
// There is no restriction on the type of the second argument; it is evaluated.
// The first argument's symbol table entry's value is set to the result of
//   evaluating the second argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting symbol (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_define(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        if (arg->type == TYPE_SYMBOL) {
            Symbol_Node* sym_entry = symbol_lookup_index(env, arg);
            if (sym_entry == NULL) {
                result = create_error_tp(EVAL_ERROR_UNDEF_SYM);
            } else {
                arg = evaluate(s_expr_next(args_tp->ptr.se_ptr), env);
                if (arg->type == TYPE_ERROR) {
                    result = arg;
                } else {
                    char* name = strdup(sym_entry->name);
                    result = install_symbol(env, name, arg->type, arg->ptr);
                    free(arg);
                    free(result);
                    result = create_atom_tp(TYPE_VOID, 0);
                }
            }
        } else if (arg->type == TYPE_SEXPR) {
            if (is_empty_list(arg->ptr.se_ptr)) {
                result = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
            } else if (arg->ptr.se_ptr->car->type != TYPE_SYMBOL) {
                result = create_error_tp(EVAL_ERROR_NOT_ID);
            } else {
                typed_ptr* fn_sym = arg->ptr.se_ptr->car;
                Symbol_Node* sym_entry = symbol_lookup_index(env, fn_sym);
                if (sym_entry == NULL) {
                    result = create_error_tp(EVAL_ERROR_UNDEF_SYM);
                } else {
                    // create a dummy (lambda arg-list body) s-expression
                    typed_ptr* arg_list = arg->ptr.se_ptr->cdr;
                    s_expr* empty = create_empty_s_expr();
                    arg->ptr.se_ptr->cdr = create_s_expr_tp(empty);
                    typed_ptr* fn_body = s_expr_next(args_tp->ptr.se_ptr)->car;
                    s_expr_next(args_tp->ptr.se_ptr)->car = NULL;
                    Symbol_Node* lam_stn = symbol_lookup_string(env, "lambda");
                    typed_ptr* lam = create_atom_tp(TYPE_SYMBOL, \
                                                    lam_stn->symbol_idx);
                    empty = create_empty_s_expr();
                    s_expr* fn_body_se = create_s_expr(fn_body, \
                                                       create_s_expr_tp(empty));
                    typed_ptr* fn_body_tp = create_s_expr_tp(fn_body_se);
                    s_expr* arg_list_se = create_s_expr(arg_list, fn_body_tp);
                    typed_ptr* arg_list_tp = create_s_expr_tp(arg_list_se);
                    s_expr* dummy_lam = create_s_expr(lam, arg_list_tp);
                    typed_ptr* fn = eval_lambda(dummy_lam, env);
                    if (fn->type == TYPE_ERROR) {
                        delete_s_expr_recursive(dummy_lam, true);
                        result = fn;
                    } else {
                        delete_s_expr_recursive(arg_list->ptr.se_ptr, true);
                        delete_s_expr_recursive(dummy_lam, false);
                        char* name = strdup(sym_entry->name);
                        blind_install_symbol_atom(env, \
                                                  name, \
                                                  fn->type, \
                                                  fn->ptr.idx);
                        free(fn);
                        result = create_typed_ptr(TYPE_VOID, \
                                                  (tp_value){.idx=0});
                    }
                }
            }
        } else {
            result = create_error_tp(EVAL_ERROR_NOT_ID);
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_SETVAR.
// This special form takes exactly two arguments.
// The first argument is expected to be a symbol name, and not evaluated. It
//   must not be TYPE_UNDEF. (It must be defined first.)
// There is no restriction on the type of the second argument; it is evaluated.
// The first argument's symbol table entry's value is set to the result of
//   evaluating the second argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting symbol (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_set_variable(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        if (arg->type != TYPE_SYMBOL) {
            result = create_error_tp(EVAL_ERROR_NOT_ID);
        } else {
            Symbol_Node* sym_entry = symbol_lookup_index(env, arg);
            if (sym_entry == NULL || sym_entry->type == TYPE_UNDEF) {
                result = create_error_tp(EVAL_ERROR_UNDEF_SYM);
            } else {
                arg = evaluate(s_expr_next(args_tp->ptr.se_ptr), env);
                if (arg->type == TYPE_ERROR) {
                    result = arg;
                } else {
                    char* name = strdup(sym_entry->name);
                    result = install_symbol(env, name, arg->type, arg->ptr);
                    free(arg);
                    free(result);
                    result = create_atom_tp(TYPE_VOID, 0);
                }
            }
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
    }
    return result;
}

typed_ptr* eval_exit(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 0, 0, false);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        result = create_error_tp(EVAL_ERROR_EXIT);
        delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
    }
    return result;
}

typed_ptr* eval_cons(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, 2, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        s_expr* args = args_tp->ptr.se_ptr;
        s_expr* rest = s_expr_next(args);
        s_expr* result_se = create_s_expr(args->car, rest->car);
        result = create_s_expr_tp(result_se);
        args->car = NULL;
        rest->car = NULL;
        delete_s_expr_recursive(args, true);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is in the set
//   {BUILTIN_xxx | xxx in {CAR, CDR}}.
// This function takes exactly one argument, which is evaluated.
// The first argument is expected to be TYPE_SEXPR.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting object (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_car_cdr(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        if (arg->type != TYPE_SEXPR || is_empty_list(arg->ptr.se_ptr)) {
            result = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
        } else if (se->car->ptr.idx == BUILTIN_CAR) {
            result = arg->ptr.se_ptr->car;
            arg->ptr.se_ptr->car = NULL;
        } else {
            result = arg->ptr.se_ptr->cdr;
            arg->ptr.se_ptr->cdr = NULL;
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in function 
//   BUILTIN_LISTPRED.
// This function takes one argument, which is evaluated.
// There is no restriction on the type of the argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the (boolean) truth value of the predicate (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_list_pred(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        result = create_atom_tp(TYPE_BOOL, 1);
        if (arg->type != TYPE_SEXPR) {
            result->ptr.idx = 0;
        } else {
            s_expr* arg_se = arg->ptr.se_ptr;
            while (!is_empty_list(arg_se)) {
                if (arg_se->cdr->type != TYPE_SEXPR) {
                    result->ptr.idx = 0;
                    break;
                }
                arg_se = s_expr_next(arg_se);
            }
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is a built-in function in the set
//   {BUILTIN_xxxxPRED | xxxx in {PAIR, BOOL, NUM, VOID}}.
// For convenience, the (C, not Lisp) function takes the type to be tested
//   against as an additional parameter.
// This function takes one argument, which is evaluated.
// There is no restriction on the type of the argument.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the (boolean) truth value of the predicate (if it succeeded).
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_atom_pred(const s_expr* se, Environment* env, type t) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* arg = args_tp->ptr.se_ptr->car;
        result = create_atom_tp(TYPE_BOOL, (arg->type == t) ? 1 : 0);
        if (arg->type == TYPE_SEXPR && is_empty_list(arg->ptr.se_ptr)) {
            result->ptr.idx = 0;
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in function BUILTIN_LIST.
// This function takes any number of arguments, which are all evaluated.
// There is no restriction on the type of the arguments.
// Returns a typed_ptr containing an error code (if the evaluation failed) or
//   the resulting s-expression (if it succeeded).
// If no arguments are provided, the resulting typed_ptr points to the empty
//   list.
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_list_construction(const s_expr* se, Environment* env) {
    return collect_args(se, env, 0, -1, true);
}

typed_ptr* eval_and_or(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 0, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        // the "starting" typed-pointer and the test in the while-loop are the
        // only ways in which "and" and "or" differ
        typed_ptr* start_tp = create_atom_tp(TYPE_BOOL, \
                                             se->car->ptr.idx == BUILTIN_AND);
        s_expr* arg_se = create_s_expr(start_tp, args_tp);
        s_expr* curr_se = arg_se;
        s_expr* last = arg_se;
        while (!is_empty_list(curr_se)) {
            last = curr_se;
            if (is_false_literal(last->car) == start_tp->ptr.idx) {
                break;
            }
            curr_se = s_expr_next(curr_se);
        }
        result = last->car;
        last->car = NULL;
        delete_s_expr_recursive(arg_se, true);
    }
    return result;
}

typed_ptr* eval_not(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 1, 1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        if (is_false_literal(args_tp->ptr.se_ptr->car)) {
            result = create_atom_tp(TYPE_BOOL, 1);
        } else {
            result = create_atom_tp(TYPE_BOOL, 0);
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_COND.
// This special form takes any number of arguments.
// The arguments must take the form (predicate [then-body...]).
// The predicates of the arguments are evaluated until one evaluates to anything
//   other than a boolean false value. After this, no predicates are evaluated.
// For an argument whose predicate evaluates to non-false, its then-bodies are
//   evaluated in order. If there are no then-bodies, the return value is the
//   result of the evaluation of the predicate.
// One special predicate, named "else", is allowed, which is a fall-through
//   case. It may only appear as the predicate of the last argument. If "else"
//   appears, it must have at least one then-body.
// Returns a typed_ptr containing an error code (if any evaluation failed) or
//   the result of the last then-body evaluation.
// If no arguments are provided, or if no argument's predicate evaluates to
//   not-false, the resulting s-expression is a void value.
// In either case, the returned typed_ptr is the caller's responsibility to
//   free, and is safe to (shallow) free without harm to the symbol table, list
//   area, or any other object.
typed_ptr* eval_cond(const s_expr* se, Environment* env) {
    typed_ptr* args_tp = collect_args(se, env, 0, -1, false);
    if (args_tp->type == TYPE_ERROR) {
        return args_tp;
    } else {
        typed_ptr* eval_interm = create_typed_ptr(TYPE_VOID, \
                                                  (tp_value){.idx=0});
        s_expr* arg_se = s_expr_next(se);
        if (is_empty_list(arg_se)) {
            delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
            free(args_tp);
            return eval_interm;
        }
        bool pred_true = false;
        s_expr* then_bodies = NULL;
        while (!is_empty_list(arg_se)) {
            if (arg_se->car->type != TYPE_SEXPR) {
                free(eval_interm);
                eval_interm = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
                break;
            }
            s_expr* cond_clause = arg_se->car->ptr.se_ptr;
            if (is_empty_list(cond_clause)) {
                free(eval_interm);
                eval_interm = create_error_tp(EVAL_ERROR_BAD_SYNTAX);
                break;
            }
            Symbol_Node* else_stn = symbol_lookup_string(env, "else");
            if (cond_clause->car->type == TYPE_SYMBOL && \
                cond_clause->car->ptr.idx == else_stn->symbol_idx) {
                s_expr* next_clause = s_expr_next(arg_se);
                if (!is_empty_list(next_clause)) {
                    free(eval_interm);
                    eval_interm = create_error_tp(EVAL_ERROR_NONTERMINAL_ELSE);
                    break;
                }
                then_bodies = s_expr_next(cond_clause);
                if (is_empty_list(then_bodies)) {
                    free(eval_interm);
                    eval_interm = create_error_tp(EVAL_ERROR_EMPTY_ELSE);
                    break;
                }
                pred_true = true;
            }
            free(eval_interm);
            eval_interm = evaluate(cond_clause, env);
            if (eval_interm->type == TYPE_ERROR) {
                break;
            } else if (!is_false_literal(eval_interm)) {
                pred_true = true;
                then_bodies = s_expr_next(cond_clause);
                break;
            }
            arg_se = s_expr_next(arg_se);
        }
        typed_ptr* result = NULL;
        if (!pred_true) { // no cond-clauses were true, or there was an error
            if (eval_interm->type == TYPE_ERROR) {
                result = eval_interm;
            } else {
                free(eval_interm);
                result = create_typed_ptr(TYPE_VOID, (tp_value){.idx=0});
            }
        } else {
            while (!is_empty_list(then_bodies)) {
                free(eval_interm);
                eval_interm = evaluate(then_bodies, env);
                then_bodies = s_expr_next(then_bodies);
            }
            result = eval_interm;
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
        return result;
    }
}

// Evaluates an s-expression whose car is the built-in special form
//   BUILTIN_LAMBDA.
// This special form takes two arguments.
// The first argument must be a list (nested s-expression) of symbols, which
//   become the parameters of the lambda. Anything else returns an error. This
//   argument is not evaluated.
// The second argument may be anything, and is not evaluated, but stored as the
//   body of the lambda.
// The function installed in the environment, and its associated data, is now
//   the environment's responsibility.
// The typed pointer returned is the caller's responsibility to free, and can
//   safely be (shallow) freed.
typed_ptr* eval_lambda(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    typed_ptr* args_tp = collect_args(se, env, 2, 2, false);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        typed_ptr* first = args_tp->ptr.se_ptr->car;
        typed_ptr* second = s_expr_next(args_tp->ptr.se_ptr)->car;
        if (first->type != TYPE_SEXPR) {
            result = create_error_tp(EVAL_ERROR_BAD_ARG_TYPE);
        } else {
            Symbol_Node* params = collect_parameters(first, env);
            if (params != NULL && params->type == TYPE_ERROR) {
                result = create_error_tp(params->value.idx);
                delete_symbol_node_list(params);
            } else {
                Environment* closure_env = copy_environment(env);
                typed_ptr* body = copy_typed_ptr(second);
                if (body->type == TYPE_SEXPR) {
                    body->ptr.se_ptr = copy_s_expr(body->ptr.se_ptr);
                }
                result = install_function(env, params, closure_env, body);
            }
        }
        delete_s_expr_recursive(args_tp->ptr.se_ptr, false);
        free(args_tp);
    }
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
    if (se->car->type != TYPE_SYMBOL) {
        params = create_error_symbol_node(EVAL_ERROR_NOT_ID);
    } else {
        char* name = symbol_lookup_index(env, se->car)->name;
        params = create_symbol_node(0, \
                                    strdup(name), \
                                    TYPE_UNDEF, \
                                    (tp_value){.idx=0});
        Symbol_Node* curr = params;
        se = s_expr_next(se);
        while (!is_empty_list(se)) {
            if (se->cdr == NULL || se->cdr->type != TYPE_SEXPR) {
                delete_symbol_node_list(params);
                params = create_error_symbol_node(EVAL_ERROR_BAD_ARG_TYPE);
                break;
            }
            if (se->car == NULL || se->car->type != TYPE_SYMBOL) {
                delete_symbol_node_list(params);
                params = create_error_symbol_node(EVAL_ERROR_NOT_ID);
                break;
            }
            name = symbol_lookup_index(env, se->car)->name;
            curr->next = create_symbol_node(0, \
                                            strdup(name), \
                                            TYPE_UNDEF, \
                                            (tp_value){.idx=0});
            curr = curr->next;
            se = s_expr_next(se);
        }
    }
    return params;
}

typed_ptr* eval_sexpr(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    s_expr* se_to_eval = se->car->ptr.se_ptr;
    if (is_empty_list(se_to_eval)) {
        result = create_error_tp(EVAL_ERROR_MISSING_PROCEDURE);
    } else {
        s_expr* empty = create_empty_s_expr();
        s_expr* dummy_se = create_s_expr(se_to_eval->car, \
                                         create_s_expr_tp(empty));
        typed_ptr* fn = evaluate(dummy_se, env);
        free(s_expr_next(dummy_se));
        free(dummy_se->cdr);
        free(dummy_se);
        if (fn->type == TYPE_ERROR) {
            result = fn;
        } else {
            if (fn->type == TYPE_BUILTIN || \
                fn->type == TYPE_FUNCTION) {
                dummy_se = create_s_expr(fn, se_to_eval->cdr);
                result = evaluate(dummy_se, env);
                free(dummy_se);
            } else {
                result = create_error_tp(EVAL_ERROR_CAR_NOT_CALLABLE);
            }
            free(fn);
        }
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
typed_ptr* eval_user_function(const s_expr* se, Environment* env) {
    typed_ptr* result = NULL;
    Function_Node* fn = function_lookup_index(env, se->car);
    if (fn == NULL) {
        return create_error_tp(EVAL_ERROR_UNDEF_FUNCTION);
    }
    typed_ptr* args_tp = collect_args(se, env, 0, -1, true);
    if (args_tp->type == TYPE_ERROR) {
        result = args_tp;
    } else {
        Symbol_Node* arg_vals = bind_args(env, fn, args_tp);
        if (arg_vals != NULL && arg_vals->type == TYPE_ERROR) {
            result = create_error_tp(arg_vals->value.idx);
        } else {
            Environment* bound_env = make_eval_env(fn->closure_env, arg_vals);
            s_expr* empty = create_empty_s_expr();
            s_expr* super_se = create_s_expr(copy_typed_ptr(fn->body), \
                                             create_s_expr_tp(empty));
            result = evaluate(super_se, bound_env);
            delete_s_expr_recursive(super_se, false);
            delete_env_shared_ft(bound_env);
        }
        delete_symbol_node_list(arg_vals);
        delete_s_expr_recursive(args_tp->ptr.se_ptr, true);
        free(args_tp);
    }
    return result;
}

// If the Function_Node's arg list is of different length than the s-expression
//   pointed to by the args typed pointer, an error is returned in the first
//   Symbol_Node.
// Otherwise, the parameters in the arg list are bound to the values produced
//   when evaluating the members of the args s-expression; any error during
//   evaluation aborts this process and is passed back out in the first
//   Symbol_Node.
// If no errors are encountered, the Symbol_Node list contains the bound
//   arguments.
// In all cases, the Symbol_Node list returned is the caller's responsibility
//   to free, and may be safely (shallow) freed.
Symbol_Node* bind_args(Environment* env, Function_Node* fn, typed_ptr* args) {
    if (fn->arg_list == NULL && is_empty_list(args->ptr.se_ptr)) {
        return NULL;
    } else if (is_empty_list(args->ptr.se_ptr)) {
        return create_error_symbol_node(EVAL_ERROR_FEW_ARGS);
    } else if (fn->arg_list == NULL) {
        return create_error_symbol_node(EVAL_ERROR_MANY_ARGS);
    } else {
        Symbol_Node* curr_param = fn->arg_list;
        s_expr* arg_se = args->ptr.se_ptr;
        Symbol_Node* bound_args = NULL;
        bound_args = create_symbol_node(0, \
                                        strdup(curr_param->name), \
                                        arg_se->car->type, \
                                        arg_se->car->ptr);
        curr_param = curr_param->next;
        arg_se = s_expr_next(arg_se);
        while (!is_empty_list(arg_se)) {
            if (curr_param == NULL) {
                delete_symbol_node_list(bound_args);
                bound_args = create_error_symbol_node(EVAL_ERROR_MANY_ARGS);
                break;
            }
            Symbol_Node* new_arg = create_symbol_node(0, \
                                                      strdup(curr_param->name),\
                                                      arg_se->car->type, \
                                                      arg_se->car->ptr);
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
// The returned environment is the caller's responsibility to delete, using
//   delete_env_shared_ft() below.
Environment* make_eval_env(Environment* env, Symbol_Node* bound_args) {
    Environment* eval_env = copy_environment(env);
    Symbol_Node* curr_arg = bound_args;
    while (curr_arg != NULL) {
        Symbol_Node* found = symbol_lookup_string(eval_env, curr_arg->name);
        if (found == NULL) {
            fprintf(stderr, "parameter name not found - something is wrong\n");
            exit(-1);
        } else {
            found->type = curr_arg->type;
            found->value = curr_arg->value;
        }
        curr_arg = curr_arg->next;
    }
    return eval_env;
}

typed_ptr* collect_args(const s_expr* se, \
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
            arg_tail->car = evaluate(arg_tail, env);
            free(temp);
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

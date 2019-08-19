#include "test_utils.h"

typed_ptr* parse_and_evaluate(char command[], Environment* env) {
    typed_ptr* parse_output = parse(command, env);
    if (parse_output->type == TYPE_ERROR) {
        return parse_output;
    } else {
        typed_ptr* eval_output = evaluate(parse_output, env);
        delete_s_expr_recursive(parse_output->ptr.se_ptr, true);
        free(parse_output);
        return eval_output;
    }
}

bool check_typed_ptr(typed_ptr* tp, type t, tp_value ptr) {
    if (tp == NULL) {
        return false;
    } else if (tp->type == TYPE_S_EXPR) {
        return tp->type == t && tp->ptr.se_ptr == ptr.se_ptr;
    } else {
        return tp->type == t && tp->ptr.idx == ptr.idx;
    }
}

bool match_error(const typed_ptr* tp, interpreter_error err) {
    return (tp != NULL && tp->type == TYPE_ERROR && tp->ptr.idx == err);
}

bool check_pair(typed_ptr* tp, \
                typed_ptr** tp_list, \
                unsigned int tp_list_len, \
                Environment* env) {
    if (tp == NULL || tp->type != TYPE_S_EXPR || tp_list_len != 2) {
        return false;
    }
    s_expr* se = tp->ptr.se_ptr;
    if (se->car->type == tp_list[0]->type && \
        se->car->ptr.idx == tp_list[0]->ptr.idx && \
        se->cdr->type == tp_list[1]->type && \
        se->cdr->ptr.idx == tp_list[1]->ptr.idx) {
        return true;
    } else {
        return false;
    }
}

bool check_s_expr(typed_ptr* tp, \
                  typed_ptr** tp_list, \
                  unsigned int tp_list_len, \
                  Environment* env) {
    // doesn't currently handle nested lists, but that's ok for now
    if (tp == NULL || tp->type != TYPE_S_EXPR) {
        return false;
    }
    s_expr* curr_se = tp->ptr.se_ptr;
    if (tp_list_len == 0) {
        if (is_empty_list(curr_se)) {
            return true;
        } else {
            return false;
        }
    } else {
        unsigned int curr_check_idx = 0;
        while (!is_empty_list(curr_se)) {
            if (curr_check_idx >= tp_list_len) {
                // the output we're checking is too long
                return false;
            }
            typed_ptr* curr_check = tp_list[curr_check_idx];
            if (curr_se->car->type != curr_check->type || \
                (curr_check->type == TYPE_S_EXPR && \
                 curr_se->car->ptr.se_ptr != curr_check->ptr.se_ptr) || \
                (curr_check->type != TYPE_S_EXPR && \
                 curr_se->car->ptr.idx != curr_check->ptr.idx)) {
                    return false;
            }
            curr_check_idx++;
            curr_se = s_expr_next(curr_se);
        }
        if (curr_check_idx < tp_list_len) {
            // the output we're checking is too short
            return false;
        } else {
            return true;
        }
    }
}

bool match_typed_ptrs(typed_ptr* first, typed_ptr* second) {
    if (first == NULL || second == NULL) {
        return false;
    } else if (first->type != second->type) {
        return false;
    } else if (first->type == TYPE_S_EXPR) {
        return first->ptr.se_ptr == second->ptr.se_ptr;
    } else {
        return first->ptr.idx == second->ptr.idx;
    }
}

bool match_s_exprs(const s_expr* first, const s_expr* second) {
    if (first == NULL && second == NULL) {
        return true;
    } else if (first == NULL || second == NULL) {
        return false;
    } else {
        if (is_empty_list(first) && is_empty_list(second)) {
            return true;
        } else if (is_empty_list(first) || is_empty_list(second)) {
            return false;
        } else if (first->car == NULL || second->car == NULL) {
            return false;
        } else if (first->cdr == NULL || second->cdr == NULL) {
            return false;
        } else {
            bool ok = true;
            if (first->car->type != second->car->type) {
                return false;
            } else if (first->car->type == TYPE_S_EXPR) {
                ok = ok && match_s_exprs(first->car->ptr.se_ptr, \
                                         second->car->ptr.se_ptr);
            } else {
                ok = ok && match_typed_ptrs(first->car, second->car);
            }
            if (first->cdr->type != second->cdr->type) {
                return false;
            } else if (first->cdr->type == TYPE_S_EXPR) {
                return ok && match_s_exprs(first->cdr->ptr.se_ptr, \
                                           second->cdr->ptr.se_ptr);
            } else {
                return ok && match_typed_ptrs(first->cdr, second->cdr);
            }
        }
    }
}

bool deep_match_typed_ptrs(typed_ptr* first, typed_ptr* second) {
    if (first == NULL && second == NULL) {
        return true;
    } else if (first == NULL || second == NULL) {
        return false;
    } else if (first->type != second->type) {
        return false;
    } else {
        switch (first->type) {
            case TYPE_S_EXPR:
                return match_s_exprs(first->ptr.se_ptr, second->ptr.se_ptr);
            default:
                return first->ptr.idx == second->ptr.idx;
        }
    }
}

void print_test_announce(char function[]) {
    printf("unit test: %-40s", function);
    return;
}

void print_test_result(bool pass) {
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    return;
}

s_expr* unit_list(typed_ptr* tp) {
    return create_s_expr(tp, create_s_expr_tp(create_empty_s_expr()));
}

void s_expr_append(s_expr* se, typed_ptr* tp) {
    // assume: se is a valid s-expression
    while (!is_empty_list(se)) {
        se = se->cdr->ptr.se_ptr;
    }
    se->car = tp;
    se->cdr = create_s_expr_tp(create_empty_s_expr());
    return;
}

typed_ptr* builtin_tp_from_name(Environment* env, const char name[]) {
    Symbol_Node* found = symbol_lookup_name(env, name);
    if (found == NULL || found->type != TYPE_BUILTIN) {
        return NULL;
    } else {
        return create_atom_tp(TYPE_BUILTIN, found->value.idx);
    }
}

typed_ptr* symbol_tp_from_name(Environment* env, const char name[]) {
    Symbol_Node* found = symbol_lookup_name(env, name);
    return (found == NULL) ? NULL : \
                             create_atom_tp(TYPE_SYMBOL, found->symbol_idx);
}

typed_ptr* create_number_tp(long value) {
    return create_typed_ptr(TYPE_FIXNUM, (tp_value){.idx=value});
}

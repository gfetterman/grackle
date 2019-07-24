#include "test_utils.h"

typed_ptr* parse_and_evaluate(char command[], Environment* env) {
    typed_ptr* output = parse(command, env);
    if (output->type != TYPE_ERROR) {
        s_expr* empty = create_empty_s_expr();
        s_expr* super_se = create_s_expr(output, create_s_expr_tp(empty));
        output = evaluate(super_se, env);
        delete_s_expr_recursive(super_se, true);
    }
    return output;
}

bool check_typed_ptr(typed_ptr* tp, type t, tp_value ptr) {
    if (tp == NULL) {
        return false;
    } else if (tp->type == TYPE_SEXPR) {
        return tp->type == t && tp->ptr.se_ptr == ptr.se_ptr;
    } else {
        return tp->type == t && tp->ptr.idx == ptr.idx;
    }
}

bool check_pair(typed_ptr* tp, \
                typed_ptr** tp_list, \
                unsigned int tp_list_len, \
                Environment* env) {
    if (tp == NULL || tp->type != TYPE_SEXPR || tp_list_len != 2) {
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

bool check_sexpr(typed_ptr* tp, \
                 typed_ptr** tp_list, \
                 unsigned int tp_list_len, \
                 Environment* env) {
    // doesn't currently handle nested lists, but that's ok for now
    if (tp == NULL || tp->type != TYPE_SEXPR) {
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
                (curr_check->type == TYPE_SEXPR && \
                 curr_se->car->ptr.se_ptr != curr_check->ptr.se_ptr) || \
                (curr_check->type != TYPE_SEXPR && \
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
    } else if (first->type == TYPE_SEXPR) {
        return first->ptr.se_ptr == second->ptr.se_ptr;
    } else {
        return first->ptr.idx == second->ptr.idx;
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

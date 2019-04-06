#include "test_functions.h"

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

void e2e_autofail_test(char cmd[], test_env* te) {
    // for tests that segfault, so I know to come back to them later
    printf("test command: %-40s", cmd);
    printf("FAILED <= (auto)\n");
    te->run++;
    return;
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

void e2e_atom_test(char cmd[], type t, long val, test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    bool pass = check_typed_ptr(output, t, (tp_value){.idx=val});
    if (output->type == TYPE_SEXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    }
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
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

void e2e_pair_test(char cmd[], \
                   typed_ptr** tp_list, \
                   unsigned int tp_list_len, \
                   test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    bool pass = check_pair(output, tp_list, tp_list_len, te->env);
    if (output->type == TYPE_SEXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    }
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
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

void e2e_sexpr_test(char cmd[], \
                    typed_ptr** tp_list, \
                    unsigned int tp_list_len, \
                    test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_evaluate(cmd, te->env);
    bool pass = check_sexpr(output, tp_list, tp_list_len, te->env);
    if (output->type == TYPE_SEXPR) {
        delete_s_expr_recursive(output->ptr.se_ptr, true);
    }
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void e2e_multiline_atom_test(char* cmds[], \
                             unsigned int num_cmds, \
                             type t, \
                             long val, \
                             test_env* te) {
    printf("test command: %-40s", cmds[0]);
    typed_ptr* output = parse_and_evaluate(cmds[0], te->env);
    for (unsigned int i = 1; i < num_cmds; i++) {
        printf("\n");
        free(output);
        printf("              %-40s", cmds[i]);
        output = parse_and_evaluate(cmds[i], te->env);
    }
    bool pass = check_typed_ptr(output, t, (tp_value){.idx=val});
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

#include "test_functions.h"

typed_ptr* parse_and_eval(char command[], environment* env) {
    typed_ptr* output = NULL;
    s_expr* parse_output = parse(command, env);
    if (!is_empty_list(parse_output) && \
        parse_output->car->type == TYPE_ERROR) {
        output = parse_output->car;
    } else {
        s_expr* super_se = create_s_expr(create_sexpr_tp(parse_output), \
                                         create_sexpr_tp(create_empty_s_expr()));
        output = evaluate(super_se, env);
        delete_se_recursive(super_se, true);
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

bool check_tp(typed_ptr* tp, type t, union_idx_se ptr) {
    if (tp == NULL) {
        return false;
    } else if (tp->type == TYPE_SEXPR) {
        return tp->type == t && tp->ptr.se_ptr == ptr.se_ptr;
    } else {
        return tp->type == t && tp->ptr.idx == ptr.idx;
    }
}

void e2e_atom_test(char cmd[], type t, unsigned int val, test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_eval(cmd, te->env);
    bool pass = check_tp(output, t, (union_idx_se){.idx=val});
    if (output->type == TYPE_SEXPR) {
        delete_se_recursive(output->ptr.se_ptr, true);
    }
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

bool check_pair(typed_ptr* tp, typed_ptr** tplist, unsigned int len, environment* env) {
    if (tp == NULL || tp->type != TYPE_SEXPR || len != 2) {
        return false;
    }
    s_expr* se = tp->ptr.se_ptr;
    if (se->car->type == tplist[0]->type && \
        se->car->ptr.idx == tplist[0]->ptr.idx && \
        se->cdr->type == tplist[1]->type && \
        se->cdr->ptr.idx == tplist[1]->ptr.idx) {
        return true;
    } else {
        return false;
    }
}

void e2e_pair_test(char cmd[], typed_ptr** tplist, unsigned int len, test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_eval(cmd, te->env);
    bool pass = check_pair(output, tplist, len, te->env);
    if (output->type == TYPE_SEXPR) {
        delete_se_recursive(output->ptr.se_ptr, true);
    }
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

bool check_sexpr(typed_ptr* tp, typed_ptr** tplist, unsigned int len, environment* env) {
    // this doesn't currently handle nested lists, but that's alright for the moment
    if (tp == NULL || tp->type != TYPE_SEXPR) {
        return false;
    }
    s_expr* curr_se = tp->ptr.se_ptr;
    if (len == 0) {
        if (is_empty_list(curr_se)) {
            return true;
        } else {
            return false;
        }
    } else {
        unsigned int curr_check_idx = 0;
        while (!is_empty_list(curr_se)) {
            if (curr_check_idx >= len) {
                // the output we're checking is too long
                return false;
            }
            typed_ptr* curr_check = tplist[curr_check_idx];
            if (curr_se->car->type != curr_check->type || \
                (curr_check->type == TYPE_SEXPR && \
                 curr_se->car->ptr.se_ptr != curr_check->ptr.se_ptr) || \
                (curr_check->type != TYPE_SEXPR && \
                 curr_se->car->ptr.idx != curr_check->ptr.idx)) {
                    return false;
            }
            curr_check_idx++;
            curr_se = sexpr_next(curr_se);
        }
        if (curr_check_idx < len) {
            // the output we're checking is too short
            return false;
        } else {
            return true;
        }
    }
}

void e2e_sexpr_test(char cmd[], typed_ptr** tplist, unsigned int len, test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_eval(cmd, te->env);
    bool pass = check_sexpr(output, tplist, len, te->env);
    if (output->type == TYPE_SEXPR) {
        delete_se_recursive(output->ptr.se_ptr, true);
    }
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

void e2e_multiline_atom_test(char* cmds[], unsigned int cmd_num, type t, unsigned int val, test_env* te) {
    printf("test command: %-40s", cmds[0]);
    typed_ptr* output = parse_and_eval(cmds[0], te->env);
    for (unsigned int i = 1; i < cmd_num; i++) {
        printf("\n");
        free(output);
        printf("              %-40s", cmds[i]);
        output = parse_and_eval(cmds[i], te->env);
    }
    bool pass = check_tp(output, t, (union_idx_se){.idx=val});
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

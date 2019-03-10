#include "test_functions.h"

typed_ptr* parse_and_eval(char command[], environment* env) {
    typed_ptr* output = NULL;
    s_expr* parse_output = parse(command, env);
    if (parse_output->car != NULL && \
        parse_output->car->type == TYPE_ERROR) {
        output = parse_output->car;
        free(parse_output);
    } else {
        typed_ptr* input_tp = install_list(env, parse_output);
        s_expr* super_se = create_s_expr(input_tp, NULL);
        output = evaluate(super_se, env);
        free(input_tp);
        free(super_se);
        free(parse_output);
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

bool check_tp(typed_ptr* tp, type t, unsigned int ptr) {
    if (tp == NULL) {
        return false;
    } else {
        return tp->type == t && tp->ptr == ptr;
    }
}

void e2e_atom_test(char cmd[], type t, unsigned int val, test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_eval(cmd, te->env);
    bool pass = check_tp(output, t, val);
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
    s_expr* se = sexpr_lookup(env, tp);
    if (se->car->type == tplist[0]->type && \
        se->car->ptr == tplist[0]->ptr && \
        se->cdr->type == tplist[1]->type && \
        se->cdr->ptr == tplist[1]->ptr) {
        return true;
    } else {
        return false;
    }
}

void e2e_pair_test(char cmd[], typed_ptr** tplist, unsigned int len, test_env* te) {
    printf("test command: %-40s", cmd);
    typed_ptr* output = parse_and_eval(cmd, te->env);
    bool pass = check_pair(output, tplist, len, te->env);
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

bool check_sexpr(typed_ptr* tp, typed_ptr** tplist, unsigned int len, environment* env) {
    if (tp == NULL || tp->type != TYPE_SEXPR) {
        return false;
    }
    s_expr* curr_se = sexpr_lookup(env, tp);
    if (len == 0) {
        if (curr_se->car == NULL && curr_se->cdr == NULL) { // curr is empty s-expr
            return true;
        } else {
            return false;
        }
    } else {
        unsigned int curr_check_idx = 0;
        while (curr_se->car != NULL) {
            if (curr_check_idx >= len) {
                // the output we're checking is too long
                return false;
            }
            typed_ptr* curr_check = tplist[curr_check_idx];
            if (curr_se->car->type != curr_check->type || \
                curr_se->car->ptr != curr_check->ptr) {
                    return false;
            }
            curr_check_idx++;
            curr_se = sexpr_lookup(env, curr_se->cdr);
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
    bool pass = check_tp(output, t, val);
    free(output);
    printf("%s\n", (pass) ? "PASSED" : "FAILED <=");
    te->passed += (pass) ? 1 : 0;
    te->run++;
    return;
}

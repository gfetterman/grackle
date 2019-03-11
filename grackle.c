#include "interp.h"

#define PROMPT ">>>"
#define BUF_SIZE 80

// Sets up the environment for execution, and runs the REPL.
int main() {
    bool exit = 0;
    char input[BUF_SIZE];
    environment* env = create_environment(0, 0, 0);
    setup_environment(env);
    while (!exit) {
        get_input(PROMPT, input, BUF_SIZE);
        s_expr* input_s_expr = parse(input, env);
        if (input_s_expr->car != NULL && \
            input_s_expr->car->type == TYPE_ERROR) {
            print_error(input_s_expr->car);
            printf("\n");
            delete_s_expr(input_s_expr);
        } else {
            typed_ptr* input_tp = install_list(env, input_s_expr);
            s_expr* super_se = create_s_expr(input_tp, NULL);
            typed_ptr* result = evaluate(super_se, env);
            print_result(result, env);
            printf("\n");
            if (result->type == TYPE_ERROR && result->ptr == EVAL_ERROR_EXIT) {
                exit = true;
            }
            free(result);
            free(input_tp);
            free(super_se);
        }
    }
    printf("exiting...\n");
    return 0;
}

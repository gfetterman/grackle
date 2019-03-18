#include "interp.h"

#define PROMPT ">>>"
#define BUF_SIZE 80

int main() {
    bool exit = 0;
    char input[BUF_SIZE];
    environment* env = create_environment(0, 0);
    setup_environment(env);
    while (!exit) {
        get_input(PROMPT, input, BUF_SIZE);
        s_expr* input_s_expr = parse(input, env);
        if (!is_empty_list(input_s_expr) && \
            input_s_expr->car->type == TYPE_ERROR) {
            print_error(input_s_expr->car);
            printf("\n");
            delete_se_recursive(input_s_expr, true);
        } else {
            s_expr* empty = create_empty_s_expr();
            s_expr* super_se = create_s_expr(create_sexpr_tp(input_s_expr), \
                                             create_sexpr_tp(empty));
            typed_ptr* result = evaluate(super_se, env);
            print_result(result, env);
            printf("\n");
            if (result->type == TYPE_ERROR && \
                result->ptr.idx == EVAL_ERROR_EXIT) {
                exit = true;
            }
            free(result);
            delete_se_recursive(super_se, true);
        }
    }
    delete_env_full(env);
    printf("exiting...\n");
    return 0;
}

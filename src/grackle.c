#include "fundamentals.h"
#include "environment.h"
#include "parse.h"
#include "evaluate.h"
#include "grackle_io.h"
#include "grackle_utils.h"

#define PROMPT ">>>"
#define BUF_SIZE 80

int main() {
    bool exit = 0;
    char input[BUF_SIZE];
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    while (!exit) {
        get_input(PROMPT, input, BUF_SIZE);
        typed_ptr* output = parse(input, env);
        if (output->type == TYPE_ERROR) {
            print_error(output);
            printf("\n");
        } else {
            s_expr* empty = create_empty_s_expr();
            s_expr* super_se = create_s_expr(output, create_s_expr_tp(empty));
            output = evaluate(super_se, env);
            print_result(output, env);
            printf("\n");
            if (output->type == TYPE_ERROR && \
                output->ptr.idx == EVAL_ERROR_EXIT) {
                exit = true;
            }
            delete_s_expr_recursive(super_se, true);
        }
        free(output);
    }
    delete_environment_full(env);
    printf("exiting...\n");
    return 0;
}

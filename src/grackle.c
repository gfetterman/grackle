#include "fundamentals.h"
#include "environment.h"
#include "parse.h"
#include "evaluate.h"
#include "grackle_io.h"

#define PROMPT ">>>"

int main() {
    bool exit = false;
    char* input = NULL;
    Environment* env = create_environment(0, 0, NULL);
    setup_environment(env);
    while (!exit) {
        input = get_input(PROMPT);
        typed_ptr* parse_output = parse(input, env);
        if (parse_output->type == TYPE_ERROR) {
            print_error(parse_output);
            printf("\n");
        } else if (parse_output->type == TYPE_S_EXPR && \
                   !is_empty_list(parse_output->ptr.se_ptr)) {
            for (s_expr* term = parse_output->ptr.se_ptr; \
                 !is_empty_list(term); \
                 term = term->cdr->ptr.se_ptr) {
                typed_ptr* eval_output = evaluate(term->car, env);
                print_typed_ptr(eval_output, env);
                printf("\n");
                if (eval_output->type == TYPE_ERROR && \
                    eval_output->ptr.idx == EVAL_ERROR_EXIT) {
                    exit = true;
                }
                if (eval_output->type == TYPE_S_EXPR) {
                    delete_s_expr_recursive(eval_output->ptr.se_ptr, true);
                } else if (eval_output->type == TYPE_STRING) {
                    delete_string(eval_output->ptr.string);
                }
                if (eval_output->type == TYPE_ERROR) {
                    free(eval_output);
                    break;
                }
                free(eval_output);
            }
        }
        if (parse_output->type == TYPE_S_EXPR) {
            delete_s_expr_recursive(parse_output->ptr.se_ptr, true);
        }
        free(parse_output);
        free(input);
    }
    delete_environment(env);
    return 0;
}

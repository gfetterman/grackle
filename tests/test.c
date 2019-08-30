#include "fundamentals.h"
#include "environment.h"
#include "test_utils.h"
#include "unit_tests.h"
#include "end_to_end_tests.h"

test_env* unit_tests() {
    printf("unit tests\n");
    printf("----------------\n");
    // setup
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    test_env* t_env = malloc(sizeof(test_env));
    if (t_env == NULL) {
        printf("malloc failed - aborting\n");
        exit(-1);
    }
    t_env->env = env;
    t_env->passed = 0;
    t_env->run = 0;
    // tests
    unit_tests_test_utils(t_env);
    unit_tests_fundamentals(t_env);
    unit_tests_environment(t_env);
    unit_tests_parse(t_env);
    unit_tests_evaluate(t_env);
    // cleanup
    delete_environment_full(t_env->env);
    t_env->env = NULL;
    return t_env;
}

test_env* end_to_end_tests() {
    printf("end-to-end tests\n");
    printf("----------------\n");
    // setup
    Environment* env = create_environment(0, 0);
    setup_environment(env);
    test_env* t_env = malloc(sizeof(test_env));
    if (t_env == NULL) {
        printf("malloc failed - aborting\n");
        exit(-1);
    }
    t_env->env = env;
    t_env->passed = 0;
    t_env->run = 0;
    // tests
    end_to_end_parse_tests(t_env);
    end_to_end_arithmetic_tests(t_env);
    end_to_end_numerical_comparison_tests(t_env);
    end_to_end_exit_tests(t_env);
    end_to_end_predicate_tests(t_env);
    end_to_end_boolean_operation_tests(t_env);
    end_to_end_car_cdr_tests(t_env);
    end_to_end_cond_tests(t_env);
    end_to_end_cons_tests(t_env);
    end_to_end_list_tests(t_env);
    end_to_end_lambda_tests(t_env);
    end_to_end_setvar_tests(t_env);
    end_to_end_define_tests(t_env);
    end_to_end_string_length_tests(t_env);
    // cleanup
    delete_environment_full(t_env->env);
    t_env->env = NULL;
    return t_env;
}

int main() {
    printf("\n----------------\n");
    test_env* unit_test_results = unit_tests();
    printf("\n----------------\n");
    test_env* e2e_test_results = end_to_end_tests();
    printf("\n----------------\n");
    printf("\ntest results");
    printf("\n------------\n");
    printf("unit tests passed/run: %u/%u\n", \
           unit_test_results->passed, \
           unit_test_results->run);
    printf("end-to-end tests passed/run: %u/%u\n", \
           e2e_test_results->passed, \
           e2e_test_results->run);
    printf("\n----------------\n");
    printf("testing complete\n");
    int PASS = 0;
    int FAIL = 64; // 64 is a generic "failure" exit code in sysexits.h
    bool ok = (unit_test_results->passed == unit_test_results->run && \
               e2e_test_results->passed == e2e_test_results->run);
    free(unit_test_results);
    free(e2e_test_results);
    return (ok) ? PASS : FAIL;
}

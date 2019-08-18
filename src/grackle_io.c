#include "grackle_io.h"

void get_input(const char* prompt, char buffer[], unsigned int buffer_size) {
    printf("%s ", prompt);
    fgets(buffer, buffer_size, stdin); // yes, this is unsafe
    // drop newline at end of input
    if ((strlen(buffer) > 0) && (buffer[strlen(buffer) - 1] == '\n')) {
        buffer[strlen(buffer) - 1] = '\0';
    }
    return;
}

void print_typed_ptr(const typed_ptr* tp, const Environment* env) {
    switch (tp->type) {
        case TYPE_UNDEF:
            printf("undefined symbol");
            break;
        case TYPE_ERROR:
            print_error(tp);
            break;
        case TYPE_FIXNUM:
            printf("%ld", tp->ptr.idx);
            break;
        case TYPE_S_EXPR:
            print_s_expr(tp->ptr.se_ptr, env);
            break;
        case TYPE_SYMBOL:
            printf("'%s", symbol_lookup_index(env, tp)->name);
            break;
        case TYPE_BUILTIN:
            printf("#<procedure:%s>", builtin_lookup_index(env, tp)->name);
            break;
        case TYPE_BOOL:
            printf("%s", (tp->ptr.idx == false) ? "#f" : "#t");
            break;
        case TYPE_VOID:
            break; // print nothing
        case TYPE_FUNCTION:
            printf("#<procedure>");
            break;
        default:
            printf("unrecognized type: %d", tp->type);
            break;
    }
    return;
}

void print_error(const typed_ptr* tp) {
    if (tp->ptr.idx != EVAL_ERROR_EXIT) {
        printf("error: ");
    }
    switch (tp->ptr.idx) {
        case PARSE_ERROR_NONE:
            printf("none (this indicates an interpreter problem)");
            break;
        case PARSE_ERROR_UNBAL_PAREN:
            printf("parsing: unbalanced parentheses");
            break;
        case PARSE_ERROR_BARE_SYM:
            printf("parsing: bare symbol outside s-expression");
            break;
        case PARSE_ERROR_EMPTY_PAREN:
            printf("parsing: empty parentheses (no procedure expression)");
            break;
        case PARSE_ERROR_TOO_MANY:
            printf("parsing: too many s-expressions (only one allowed)");
            break;
        case PARSE_ERROR_INT_UNSPEC:
            printf("parsing: unspecified error while parsing an integer");
            break;
        case PARSE_ERROR_INT_TOO_LOW:
            printf("parsing: integer literal too small to fit in fixnum");
            break;
        case PARSE_ERROR_INT_TOO_HIGH:
            printf("parsing: integer literal too large to fit in fixnum");
            break;
        case EVAL_ERROR_EXIT:
            break; // exit is handled in the REPL
        case EVAL_ERROR_NULL_S_EXPR:
            printf("evaluation: an s-expression was NULL");
            break;
        case EVAL_ERROR_MALFORMED_S_EXPR:
            printf("evaluation: NULL car or cdr in a non-empty s-expression");
            break;
        case EVAL_ERROR_UNDEF_SYM:
            printf("evaluation: undefined symbol");
            break;
        case EVAL_ERROR_UNDEF_TYPE:
            printf("evaluation: undefined type");
            break;
        case EVAL_ERROR_UNDEF_BUILTIN:
            printf("evaluation: undefined built-in function or special form");
            break;
        case EVAL_ERROR_FEW_ARGS:
            printf("evaluation: too few arguments to function or special form");
            break;
        case EVAL_ERROR_MANY_ARGS:
            printf("evaluation: too many arguments to function or special ");
            printf("form");
            break;
        case EVAL_ERROR_BAD_ARG_TYPE:
            printf("evaluation: argument to function or special form had the ");
            printf("wrong type");
            break;
        case EVAL_ERROR_NEED_NUM:
            printf("evaluation: function or special form takes only number ");
            printf("arguments");
            break;
        case EVAL_ERROR_FIXNUM_UNDER:
            printf("evaluation: fixnum integer underflow");
            break;
        case EVAL_ERROR_FIXNUM_OVER:
            printf("evaluation: fixnum integer overflow");
            break;
        case EVAL_ERROR_DIV_ZERO:
            printf("evaluation: cannot divide by zero");
            break;
        case EVAL_ERROR_NONTERMINAL_ELSE:
            printf("'else' must be terminal clause in 'cond' special form");
            break;
        case EVAL_ERROR_CAR_NOT_CALLABLE:
            printf("evaluation: car of s-expression was not a callable");
            break;
        case EVAL_ERROR_NOT_ID:
            printf("evaluation: first argument to 'set!' must be an ");
            printf("identifier");
            break;
        case EVAL_ERROR_MISSING_PROCEDURE:
            printf("evaluation: missing procedure expression; probably ");
            printf("originally bare ()");
            break;
        case EVAL_ERROR_BAD_SYNTAX:
            printf("evaluation: bad syntax for a function or special form");
            break;
        case EVAL_ERROR_ILLEGAL_PAIR:
            printf("evaluation: cannot evaluate a pair");
            break;
        case EVAL_ERROR_EMPTY_ELSE:
            printf("evaluation: else-clause in cond must have then-body");
            break;
        case EVAL_ERROR_UNDEF_FUNCTION:
            printf("evaluation: undefined user function");
            break;
        case EVAL_ERROR_BAD_SYMBOL:
            printf("evaluation: symbol not in symbol table");
            break;
        default:
            printf("unknown error: error code %ld", tp->ptr.idx);
            break;
    }
    return;
}

void print_s_expr(const s_expr* se, const Environment* env) {
    if (se == NULL) {
        typed_ptr* err = create_error_tp(EVAL_ERROR_NULL_S_EXPR);
        print_error(err);
        free(err);
        return;
    }
    printf("'(");
    while (!is_empty_list(se)) {
        print_typed_ptr(se->car, env);
        if (se->cdr->type == TYPE_S_EXPR) { // list
            se = se->cdr->ptr.se_ptr;
            if (!is_empty_list(se)) {
                printf(" ");
            }
        } else { // pair
            printf(" . ");
            print_typed_ptr(se->cdr, env);
            break;
        }
    }
    printf(")");
    return;
}

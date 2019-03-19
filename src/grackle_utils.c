#include "grackle_utils.h"

// The string returned is a valid null-terminated C string.
// The string returned is the caller's responsibility to free.
char* substring(char* str, unsigned int start, unsigned int end) {
    unsigned int len = strlen(str);
    if (str == NULL || len < (end - start)) {
        fprintf(stderr, \
                "fatal error: bad substring from %u to %u, for str len %u\n", \
                start, \
                end, \
                len);
        exit(-1);
    }
    char* ss = malloc(sizeof(char) * (end - start + 1));
    if (ss == NULL) {
        fprintf(stderr, "fatal error: malloc failed in substring()\n");
        exit(-1);
    }
    memcpy(ss, (str + start), (sizeof(char) * (end - start)));
    ss[end - start] = '\0';
    return ss;
}

// Determines whether a string represents a number (rather than a symbol).
// Currently only recognizes nonnegative integers.
bool string_is_number(const char str[]) {
    char c;
    bool ok = true;
    if (*str == '-') {
        str++;
    }
    if (*str == '\0') {
        ok = false;
    } else {
        while ((c = *str++)) {
            if (c < 48 || c > 57) {
                ok = false;
                break;
            }
        }
    }
    return ok;
}

bool is_false_literal(const typed_ptr* tp) {
    return (tp->type == TYPE_BOOL && tp->ptr.idx == 0);
}

bool is_pair(const s_expr* se) {
    if (se == NULL) {
        printf("cannot determine pair-ness of NULL s-expression\n");
        exit(-1);
    }
    if (se->cdr == NULL) {
        printf("cannot determine pair-ness if cdr is NULL\n");
        exit(-1);
    }
    return se->cdr->type != TYPE_SEXPR;
}

#include "fundamentals.h"

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_typed_ptr(type type, union_idx_se ptr) {
    typed_ptr* new_tp = malloc(sizeof(typed_ptr));
    if (new_tp == NULL) {
        fprintf(stderr, "malloc failed in create_typed_ptr()\n");
        exit(-1);
    }
    new_tp->type = type;
    new_tp->ptr = ptr;
    return new_tp;
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_atom_tp(type type, unsigned int idx) {
    return create_typed_ptr(type, (union_idx_se){.idx=idx});
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_sexpr_tp(s_expr* se_ptr) {
    return create_typed_ptr(TYPE_SEXPR, (union_idx_se){.se_ptr=se_ptr});
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* create_error(unsigned int err_code) {
    return create_atom_tp(TYPE_ERROR, err_code);
}

// The returned typed_ptr is the caller's responsibility to free; it can be
//   safely (shallow) freed without harm to any other object.
typed_ptr* copy_typed_ptr(const typed_ptr* tp) {
    return create_typed_ptr(tp->type, tp->ptr);
}

// The s-expression returned is the caller's responsibility to free.
s_expr* create_s_expr(typed_ptr* car, typed_ptr* cdr) {
    s_expr* new_se = malloc(sizeof(s_expr));
    if (new_se == NULL) {
        fprintf(stderr, "malloc failed in create_s_expr()\n");
        exit(-1);
    }
    new_se->car = car;
    new_se->cdr = cdr;
    return new_se;
}

s_expr* sexpr_next(const s_expr* se) {
    return se->cdr->ptr.se_ptr;
}

// The s-expression returned is the caller's responsibility to free.
s_expr* create_empty_s_expr() {
    return create_s_expr(NULL, NULL);
}

s_expr* copy_s_expr(const s_expr* se) {
    if (se == NULL) {
        return NULL;
    }
    s_expr* new_se = create_empty_s_expr();
    s_expr* curr_se = new_se;
    while (!is_empty_list(se)) {
        curr_se->car = copy_typed_ptr(se->car);
        if (curr_se->car->type == TYPE_SEXPR) {
            curr_se->car->ptr.se_ptr = copy_s_expr(curr_se->car->ptr.se_ptr);
        } // otherwise it's atomic and a copy of the typed_ptr is enough
        curr_se->cdr = copy_typed_ptr(se->cdr);
        if (curr_se->cdr->type == TYPE_SEXPR) {
            curr_se->cdr->ptr.se_ptr = create_empty_s_expr();
            curr_se = sexpr_next(curr_se);
            se = sexpr_next(se);
        } else { // se is a pair, so we're done
            break;
        }
    }
    return new_se;
}

void delete_se_recursive(s_expr* se, bool delete_sexpr_cars) {
    s_expr* curr = se;
    while (curr != NULL) {
        if (delete_sexpr_cars && \
            curr->car != NULL && \
            curr->car->type == TYPE_SEXPR) {
            delete_se_recursive(curr->car->ptr.se_ptr, true);
        }
        free(curr->car);
        if (curr->cdr != NULL && curr->cdr->type == TYPE_SEXPR) {
            se = sexpr_next(curr);
            free(curr->cdr);
        } else {
            se = NULL;
            free(curr->cdr);
        }
        free(curr);
        curr = se;
    }
    return;
}

bool is_empty_list(const s_expr* se) {
    if (se == NULL) {
        printf("cannot determine if NULL se is empty list\n");
        exit(-1);
    } else {
        return (se->car == NULL && se->cdr == NULL);
    }
}

#ifndef GRACKLE_IO_H
#define GRACKLE_IO_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "fundamentals.h"
#include "environment.h"

char* get_input(const char* prompt);

void print_typed_ptr(const typed_ptr* tp, const Environment* env);

void print_error(const typed_ptr* tp);
void print_s_expr(const s_expr* se, const Environment* env);

#endif

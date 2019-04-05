#ifndef GRACKLE_IO_H
#define GRACKLE_IO_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "fundamentals.h"
#include "environment.h"
#include "grackle_utils.h"

void get_input(char* prompt, char buffer[], unsigned int buffer_size);
void print_error(const typed_ptr* tp);
void print_s_expression(const s_expr* se, Environment* env);
void print_result(const typed_ptr* tp, Environment* env);

#endif

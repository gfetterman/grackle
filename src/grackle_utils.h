#ifndef GRACKLE_UTILS_H
#define GRACKLE_UTILS_H

#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

#include "fundamentals.h"

char* substring(char* str, unsigned int start, unsigned int end);
bool string_is_number(const char str[]);
bool is_false_literal(const typed_ptr* tp);
bool is_pair(const s_expr* se);

#endif

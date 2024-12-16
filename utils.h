#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stddef.h>

#include "com.h"

bool Is_equal(double a, double b);
size_t Get_file_size(FILE *file);
bool Is_operator(char simbol);
bool Is_parity(double a);
bool Is_inf_or_nan(double a);

//#define USE_DEBUG_PRINTF

#ifdef USE_DEBUG_PRINTF
    #define DPRINTF(str, ...) fprintf(stderr, str, __VA_ARGS__)
#else
    #define DPRINTF(str, ...) ;
#endif

#endif //UTILS_H
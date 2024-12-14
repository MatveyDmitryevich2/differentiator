#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>

#include "com.h"

bool Is_equal(double a, double b)
{
    if (fabs(a - b) < 0.0001) { return true; }

    return false;
}

bool Is_operator(char simbol)
{
    return (simbol == '+' || simbol == '-' || simbol == '/' || simbol == '*' || simbol == '^');
}

size_t Get_file_size(FILE *file)
{
    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    size_t size_file = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    return size_file;
}
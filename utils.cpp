#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

int Sravnenie(double a, double b)
{
    int resultat = 0;
    if (fabs(a - b) < 0.0001) {resultat = 1; }

    return resultat;
}

size_t Get_file_size(FILE *file)
{
    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    size_t size_file = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    return size_file;
}
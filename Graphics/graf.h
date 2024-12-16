#ifndef GRAF_H
#define GRAF_H

#include <stdio.h>
#include <stdbool.h>

#include "../com.h"
#include "../utils.h"
#include "../tree_utils.h"
#include "../tree.h"

struct Array_coordinates_points
{
    double x;
    double y;
};

static const size_t SIZE_ARRAY_COORDINATES_POINTS = 61;
static const double MIN_X = -30;
static const double MAX_X = 30;

Array_coordinates_points* Calculat_value_function_at_point(Node* root, Pool_allocator* pool_allocator);

#endif //GRAF_H
#ifndef TAYLOR_H
#define TAYLOR_H

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>

#include "../com.h"
#include "../utils.h"
#include "../tree_utils.h"
#include "../tree.h"

Node* Calculating_the_n_term(Node* node, int64_t number_member_in_decomposition,
                             int64_t function_point, Pool_allocator* pool_allocator);
Node* Assembling_formula(Node* node, int64_t order_of_decomposition,
                             int64_t function_point, Pool_allocator* pool_allocator);

#endif //TAYLOR_H
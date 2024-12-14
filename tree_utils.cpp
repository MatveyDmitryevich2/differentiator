#include "tree_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include "com.h"

bool Is_empty(Node* node)
{
    return (node == NULL);
}

bool Is_operation(Node* node)
{
    return ((node != NULL) && (node->elem.type == Types_OPERATION));
}

bool Is_number(Node* node)
{
    return ((node != NULL) && (node->elem.type == Types_NUMBER));
}

bool Is_veriable(Node* node)
{
    return ((node != NULL) && (node->elem.type == Types_VARIABLE));
}

bool Is_op_mul(Node* node)
{
   return ((node != NULL) && (node->elem.argument.operation == Operation_MUL));
}
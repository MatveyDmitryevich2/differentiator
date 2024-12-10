#include "tree_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include "tree.h"

bool Is_empty(Node* node)
{
    if (node == NULL) { return true;  }
    else              { return false; }
}

bool Is_operation(Node* node)
{
    if ((node != NULL) && (node->elem.type == Types_OPERATION)) { return true;  }
    else                                                        { return false; }
}

bool Is_number(Node* node)
{
    if ((node != NULL) && (node->elem.type == Types_NUMBER)) { return true;  }
    else                                                     { return false; }
}

bool Is_veriable(Node* node)
{
    if ((node != NULL) && (node->elem.type == Types_VARIABLE)) { return true;  }
    else                                                       { return false; }
}

bool Is_op_mul(Node* node)
{
    if ((node != NULL) && (node->elem.argument.operation == Operation_MUL)) { return true;  }
    else                                                { return false; }
}
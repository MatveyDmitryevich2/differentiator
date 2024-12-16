#include "graf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include "../utils.h"
#include "../com.h"
#include "../tree_utils.h"
#include "../tree.h"

static double Counting_leaflet_at_point(Node* node, double x, Pool_allocator* pool_allocator);

Array_coordinates_points* Calculat_value_function_at_point(Node* root, Pool_allocator* pool_allocator)
{
    assert(root != NULL);

    FILE* file = fopen("coord.txt", "w");

    Array_coordinates_points* array_coordinates_points 
    = (Array_coordinates_points*)calloc(SIZE_ARRAY_COORDINATES_POINTS, sizeof(Array_coordinates_points));

    double range = MIN_X;

    for (size_t i = 0; range <= MAX_X; range = range + 0.1)
    {
        Node* node = Сopy_branch(root, root->parent, pool_allocator);
        Count_expression(node, range, pool_allocator);

        if (isinf(node->elem.argument.number) || isnan(node->elem.argument.number) || node->elem.type != Types_NUMBER)
        {
           i--;
           Tree_dtor(node, pool_allocator);

           continue;
        }

        else
        {
        array_coordinates_points[i].x = range;
        array_coordinates_points[i].y = node->elem.argument.number;

        fprintf(file, "%.2lf %.2lf\n", array_coordinates_points[i].x, array_coordinates_points[i].y);
        
        Tree_dtor(node, pool_allocator);
        i++;
        }
    }

    fclose(file);

    return array_coordinates_points;
}

double Count_expression(Node* node, double x, Pool_allocator* pool_allocator)
{
    if (!Is_operation(node)) { return 0; }

    if (Is_operation(node->left))
    {
        double ret_l = Count_expression(node->left, x, pool_allocator);
        if (Is_inf_or_nan(ret_l)) { return ret_l; }
    }
    if (Is_operation(node->right)) 
    {
        double ret_r = Count_expression(node->right, x, pool_allocator);
        if (Is_inf_or_nan(ret_r)) { return ret_r; }
    }

    double ret_n = Counting_leaflet_at_point(node, x, pool_allocator);
    if (Is_inf_or_nan(ret_n)) { return ret_n; }

    else return 0;
}

static double Counting_leaflet_at_point(Node* node, double x, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    #define CASE(op, op_sign)                                                                                 \
    case op:                                                                                                  \
    {                                                                                                         \
        if (node->left->elem.type == Types_VARIABLE)                                                          \
        {                                                                                                     \
            node->left->elem.type = Types_NUMBER;                                                             \
            node->left->elem.argument.number = x;                                                             \
        }                                                                                                     \
        if (node->right->elem.type == Types_VARIABLE)                                                         \
        {                                                                                                     \
            node->right->elem.type = Types_NUMBER;                                                            \
            node->right->elem.argument.number = x;                                                            \
        }                                                                                                     \
                                                                                                              \
                                                                                                              \
        if (Is_equal(node->right->elem.argument.number, 0) && op == Operation_DIV) { return NAN; }            \
        double value = node->left->elem.argument.number op_sign node->right->elem.argument.number;            \
        if (Is_inf_or_nan(value)) { return value; }                                                           \
        node->elem.type = Types_NUMBER;                                                                       \
        node->elem.argument.number = value;                                                                   \
                                                                                                              \
        Tree_dtor(node->left, pool_allocator);                                                                \
        Tree_dtor(node->right, pool_allocator);                                                               \
        node->left = NULL;                                                                                    \
        node->right = NULL;                                                                                   \
    }                                                                                                         \
    break;

    #define CASE_NO_DANGER(op, op_sign)                                      \
    case op:                                                                 \
    {                                                                        \
        if (node->left != NULL && node->left->elem.type == Types_VARIABLE)   \
        {                                                                    \
            node->left->elem.type = Types_NUMBER;                            \
            node->left->elem.argument.number = x;                            \
                                                                             \
            double value = op_sign(node->left->elem.argument.number);        \
            Tree_dtor(node->left, pool_allocator);                           \
            node->left = NULL;                                               \
            if (Is_inf_or_nan(value)) { return value; }                      \
                                                                             \
            node->elem.type = Types_NUMBER;                                  \
            node->elem.argument.number = value;                              \
                                                                             \
        }                                                                    \
        if (node->right != NULL && node->right->elem.type == Types_VARIABLE) \
        {                                                                    \
            node->right->elem.type = Types_NUMBER;                           \
            node->right->elem.argument.number = x;                           \
                                                                             \
            double value = op_sign(node->right->elem.argument.number);       \
            Tree_dtor(node->right, pool_allocator);                          \
            node->right = NULL;                                              \
            if (Is_inf_or_nan(value)) { return value; }                      \
                                                                             \
            node->elem.type = Types_NUMBER;                                  \
            node->elem.argument.number = value;                              \
                                                                             \
        }                                                                    \
    }                                                                        \
    break;

    #define CASE_CT(op, op_sign)                                             \
    case op:                                                                 \
    {                                                                        \
        if (node->left != NULL && node->left->elem.type == Types_VARIABLE)   \
        {                                                                    \
            node->left->elem.type = Types_NUMBER;                            \
            node->left->elem.argument.number = x;                            \
                                                                             \
            double value = 1.0 / op_sign(node->left->elem.argument.number);  \
            Tree_dtor(node->left, pool_allocator);                           \
            node->left = NULL;                                               \
            if (Is_inf_or_nan(value)) { return value; }                      \
                                                                             \
            node->elem.type = Types_NUMBER;                                  \
            node->elem.argument.number = value;                              \
                                                                             \
        }                                                                    \
        if (node->right != NULL && node->right->elem.type == Types_VARIABLE) \
        {                                                                    \
            node->right->elem.type = Types_NUMBER;                           \
            node->right->elem.argument.number = x;                           \
                                                                             \
            double value = 1.0 / op_sign(node->right->elem.argument.number); \
            Tree_dtor(node->right, pool_allocator);                          \
            node->right = NULL;                                              \
            if (Is_inf_or_nan(value)) { return value; }                      \
                                                                             \
            node->elem.type = Types_NUMBER;                                  \
            node->elem.argument.number = value;                              \
                                                                             \
        }                                                                    \
    }                                                                        \
    break;

    #define CASE_ARCCTG()                                                      \
    case Operation_ARCCTG:                                                     \
    {                                                                          \
        if (node->left != NULL && node->left->elem.type == Types_VARIABLE)     \
        {                                                                      \
            node->left->elem.type = Types_NUMBER;                              \
            node->left->elem.argument.number = x;                              \
                                                                               \
            double value = M_PI / 2 - atan(node->left->elem.argument.number);  \
            Tree_dtor(node->left, pool_allocator);                             \
            node->left = NULL;                                                 \
            if (Is_inf_or_nan(value)) { return value; }                        \
                                                                               \
            node->elem.type = Types_NUMBER;                                    \
            node->elem.argument.number = value;                                \
                                                                               \
        }                                                                      \
        if (node->right != NULL && node->right->elem.type == Types_VARIABLE)   \
        {                                                                      \
            node->right->elem.type = Types_NUMBER;                             \
            node->right->elem.argument.number = x;                             \
                                                                               \
            double value = M_PI / 2 - atan(node->right->elem.argument.number); \
            Tree_dtor(node->right, pool_allocator);                            \
            node->right = NULL;                                                \
            if (Is_inf_or_nan(value)) { return value; }                        \
                                                                               \
            node->elem.type = Types_NUMBER;                                    \
            node->elem.argument.number = value;                                \
                                                                               \
        }                                                                      \
    }                                                                          \
    break;

    #define CASE_POWER()                                                                           \
    case Operation_POWER:                                                                          \
    {                                                                                              \
        if (node->left->elem.type == Types_VARIABLE)                                               \
        {                                                                                          \
            node->left->elem.type = Types_NUMBER;                                                  \
            node->left->elem.argument.number = x;                                                  \
        }                                                                                          \
        if (node->right->elem.type == Types_VARIABLE)                                              \
        {                                                                                          \
            node->right->elem.type = Types_NUMBER;                                                 \
            node->right->elem.argument.number = x;                                                 \
        }                                                                                          \
                                                                                                   \
                                                                                                   \
        double value = pow(node->left->elem.argument.number, node->right->elem.argument.number);   \
        Tree_dtor(node->left, pool_allocator);                                                     \
        Tree_dtor(node->right, pool_allocator);                                                    \
        node->left = NULL;                                                                         \
        node->right = NULL;                                                                        \
        if (Is_inf_or_nan(value)) { return value; }                                                \
        node->elem.type = Types_NUMBER;                                                            \
        node->elem.argument.number = value;                                                        \
                                                                                                   \
    }                                                                                              \
    break;

    #define CASE_LOG()                                                                                 \
    case Operation_LOG:                                                                                \
    {                                                                                                  \
        if (node->left->elem.type == Types_VARIABLE)                                                   \
        {                                                                                              \
            node->left->elem.type = Types_NUMBER;                                                      \
            node->left->elem.argument.number = x;                                                      \
        }                                                                                              \
        if (node->right->elem.type == Types_VARIABLE)                                                  \
        {                                                                                              \
            node->right->elem.type = Types_NUMBER;                                                     \
            node->right->elem.argument.number = x;                                                     \
        }                                                                                              \
                                                                                                       \
        double value = log(node->right->elem.argument.number) / log(node->left->elem.argument.number); \
        Tree_dtor(node->left, pool_allocator);                                                                              \
        Tree_dtor(node->right, pool_allocator);                                                                             \
        node->left = NULL;                                                                             \
        node->right = NULL;                                                                            \
        if (Is_inf_or_nan(value)) { return value; }                                                    \
        node->elem.type = Types_NUMBER;                                                                \
        node->elem.argument.number = value;                                                            \
                                                                                                       \
    }                                                                                                  \
    break;

    #define CASE_LN()                                                        \
    case Operation_LN:                                                       \
    {                                                                        \
        if (node->left != NULL && node->left->elem.type == Types_VARIABLE)   \
        {                                                                    \
            node->left->elem.type = Types_NUMBER;                            \
            node->left->elem.argument.number = x;                            \
                                                                             \
            double value = log(node->left->elem.argument.number);            \
            Tree_dtor(node->left, pool_allocator);                           \
            node->left = NULL;                                               \
            if (Is_inf_or_nan(value)) { return value; }                      \
                                                                             \
            node->elem.type = Types_NUMBER;                                  \
            node->elem.argument.number = value;                              \
                                                                             \
        }                                                                    \
        if (node->right != NULL && node->right->elem.type == Types_VARIABLE) \
        {                                                                    \
            node->right->elem.type = Types_NUMBER;                           \
            node->right->elem.argument.number = x;                           \
                                                                             \
            double value = log(node->right->elem.argument.number);           \
            Tree_dtor(node->right, pool_allocator);                          \
            node->right = NULL;                                              \
            if (Is_inf_or_nan(value)) { return value; }                      \
                                                                             \
            node->elem.type = Types_NUMBER;                                  \
            node->elem.argument.number = value;                              \
                                                                             \
        }                                                                    \
    }                                                                        \
    break;                                                                   

    #define CASE_NO_OP() case Operation_NO_OPERATION: {}break;   
    #define DEF() default: { assert(0 && "No operation"); }


    switch(node->elem.argument.operation)
    {
        CASE           (Operation_ADD,    +   );
        CASE           (Operation_SUB,    -   );
        CASE           (Operation_MUL,    *   );
        CASE           (Operation_DIV,    /   );

        CASE_NO_DANGER (Operation_SIN,    sin );
        CASE_NO_DANGER (Operation_COS,    cos );
        CASE_NO_DANGER (Operation_TG,     tan );
        CASE_NO_DANGER (Operation_SH,     sinh);
        CASE_NO_DANGER (Operation_CH,     cosh);
        CASE_NO_DANGER (Operation_TH,     tanh);
        CASE_NO_DANGER (Operation_ARCSIN, asin);
        CASE_NO_DANGER (Operation_ARCCOS, acos);
        CASE_NO_DANGER (Operation_ARCTG,  atan);
        CASE_NO_DANGER (Operation_EXP,    exp );

        CASE_CT        (Operation_CTH,    tanh);
        CASE_CT        (Operation_CTG,    tan );
        CASE_ARCCTG    ();
        CASE_POWER     ();
        CASE_LOG       ();
        CASE_LN        ();
        CASE_NO_OP     ();
        DEF            ();
    }

    #undef CASE
    #undef CASE_NO_DANGER
    #undef CASE_CT
    #undef CASE_ARCCTG
    #undef CASE_LN
    #undef CASE_LOG
    #undef CASE_POWER
    #undef CASE_NO_OP
    #undef DEF

    return 0;
}
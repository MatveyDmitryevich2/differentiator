#include "dif_function.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include "utils.h"

static void Fold_constants(Node* node);
static void Reduce_redundant_ops(Node* node);
// static void Create_node_op(Node* node, Node* parent, Types type, Operation operation, Node* left, Node* right);

static Node* Add_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Sub_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Mul_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Dif_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Cos_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Sin_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Ln_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Log_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Power_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Exp_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Tg_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Ctg_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Sh_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Ch_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Th_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Cth_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* ArcSin_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Arccos_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Arctg_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Arcctg_dif(Node* node, Pool_allocator* Pool_allocator);

static Node* Dif_number(Node* node);
static Node* Dif_variable(Node* node);
static Node* Dif_operation(Node* node, Pool_allocator* pool_allocator);

void Calculation(Node* node)
{
    assert(node != NULL);

    if ((node->elem.type == Types_NUMBER) || (node->elem.type == Types_VARIABLE)) { return; }

    DPRINTF("\n%p\n", node);
    DPRINTF("\n%p\n", node->left);

    if ((node->left->elem.type != Types_NUMBER) 
        && (node->left->elem.type != Types_VARIABLE)) { Calculation(node->left); }
    if ((node->right->elem.type != Types_NUMBER)
        && (node->right->elem.type != Types_VARIABLE)) { Calculation(node->right); }

    if ((node->left->elem.type == Types_NUMBER) && (node->right->elem.type == Types_NUMBER)) { Fold_constants(node); }

    return;
}

void Simplification(Node* node)
{
    assert(node != NULL);

    if ((node->elem.type == Types_NUMBER) || (node->elem.type == Types_VARIABLE)) { return; }

    if ((node->left->elem.type != Types_NUMBER) 
        && (node->left->elem.type != Types_VARIABLE)) { Simplification(node->left); }
    if ((node->right->elem.type != Types_NUMBER)
        && (node->right->elem.type != Types_VARIABLE)) { Simplification(node->right); }

    if ((node->left->elem.type == Types_VARIABLE) && (node->right->elem.type == Types_NUMBER)) { Reduce_redundant_ops(node); }
    else if ((node->left->elem.type == Types_NUMBER) && (node->right->elem.type == Types_VARIABLE)) { Reduce_redundant_ops(node); }

    return;
}

Node* Dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    if (node->elem.type == Types_NUMBER)    { return Dif_number(node);    }
    if (node->elem.type == Types_VARIABLE)  { return Dif_variable(node);  }
    if (node->elem.type == Types_OPERATION) { return Dif_operation(node, pool_allocator); }

    return node;
}

static void Fold_constants(Node* node)
{
    assert(node != NULL);

    #define CASE_NOT_USE(op) case op: {} break;

    #define CASE(op, op_sign) case op:                                                                           \
    {                                                                                                            \
        node->elem.type = Types_NUMBER;                                                                                \
        node->elem.argument.number = node->left->elem.argument.number op_sign node->right->elem.argument.number; \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    #define CASE_Operation_POWER(op) case op:                                                                              \
    {                                                                                                            \
        node->elem.type = Types_NUMBER;                                                                                \
        node->elem.argument.number = pow(node->left->elem.argument.number, node->right->elem.argument.number);   \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    #define CASE_ANOTHER(op, op_sign) case op:                                                                           \
    {                                                                                                            \
        node->elem.type = Types_NUMBER;                                                                                \
        node->elem.argument.number = op_sign(node->left->elem.argument.number);                                  \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    switch(node->elem.argument.operation)
    {
        CASE(Operation_ADD, +);
        CASE(Operation_SUB, -);
        CASE(Operation_MUL, *);
        CASE(Operation_DIV, /);
        CASE_ANOTHER(Operation_SIN, sin);
        CASE_ANOTHER(Operation_COS, cos);
        CASE_ANOTHER(Operation_TG, tan);
        CASE_ANOTHER(Operation_SH, sinh);
        CASE_ANOTHER(Operation_CH, cosh);
        CASE_ANOTHER(Operation_ARCSIN, asin);
        CASE_ANOTHER(Operation_ARCCOS, acos);
        CASE_ANOTHER(Operation_ARCTG, atan);
        CASE_ANOTHER(Operation_LOG, log);
        CASE_ANOTHER(Operation_EXP, exp);
        CASE_NOT_USE(Operation_CTH);
        CASE_NOT_USE(Operation_TH);
        CASE_NOT_USE(Operation_CTG);
        CASE_NOT_USE(Operation_LN);
        CASE_NOT_USE(Operation_ARCCTG);
        CASE_NOT_USE(Operation_NO_OPERATION);
        CASE_Operation_POWER(Operation_POWER);
        default: { assert(0 && "Unknoun operation"); };
    }

    #undef CASE
    #undef CASE_NOT_USE
    #undef CASE_Operation_POWER
    #undef CASE_ANOTHER
}

// static void Create_node_op(Node* node, Node* parent, Types type, Operation operation, Node* left, Node* right)
// {
//     node->parent = parent;
//     node->elem.type = type;
//     node->elem.argument.operation = operation;
//     node->left = left;
//     node->right = right;
// }

static void Reduce_redundant_ops(Node* node)
{
    assert(node != NULL);

    #define REDUCTION(op_enum, op, right_hand_value)                                                                                    \
        node->elem.type = op_enum;                                                                               \
        node->elem.argument.op = right_hand_value;                                                               \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      

    switch(node->elem.argument.operation)
    {
        case Operation_ADD:
        {
            if (Sravnenie(node->right->elem.argument.number, 0))
                { REDUCTION(Types_VARIABLE, variable, node->left->elem.argument.variable); }

            else if (Sravnenie(node->left->elem.argument.number, 0))
                { REDUCTION(Types_VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case Operation_MUL:
        {
            if (Sravnenie(node->right->elem.argument.number, 0))
                { REDUCTION(Types_NUMBER, number, 0); }

            else if (Sravnenie(node->left->elem.argument.number, 0))
                { REDUCTION(Types_NUMBER, number, 0); }

            else if (Sravnenie(node->right->elem.argument.number, 1))
                { REDUCTION(Types_VARIABLE, variable, node->left->elem.argument.variable); }

            else if (Sravnenie(node->left->elem.argument.number, 1))
                { REDUCTION(Types_VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case Operation_SUB:
        {
            if (Sravnenie(node->right->elem.argument.number, 0))
                { REDUCTION(Types_VARIABLE, variable, node->left->elem.argument.variable); }

            else if (Sravnenie(node->left->elem.argument.number, 0))
                { REDUCTION(Types_VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case Operation_POWER:
        {
                 if (Sravnenie(node->right->elem.argument.number, 0))
                    { REDUCTION(Types_NUMBER, number, 1); }

            else if (Sravnenie(node->left->elem.argument.number, 0))
                    { REDUCTION(Types_NUMBER, number, 1); }

            else if (Sravnenie(node->right->elem.argument.number, 1))
                    { REDUCTION(Types_VARIABLE, variable, node->left->elem.argument.variable); }

            else if (Sravnenie(node->left->elem.argument.number, 1))
                    { REDUCTION(Types_VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case Operation_DIV:
        {
            if (Sravnenie(node->right->elem.argument.number, 1))
               { REDUCTION(Types_VARIABLE, variable, node->left->elem.argument.variable); }
        }
        break;

        case Operation_COS: {} break;
        case Operation_SIN: {} break;
        case Operation_TG: {} break;
        case Operation_CTG: {} break;
        case Operation_SH: {} break;
        case Operation_CH: {} break;
        case Operation_ARCCTG: {} break;
        case Operation_ARCTG: {} break;
        case Operation_ARCSIN: {} break;
        case Operation_ARCCOS: {} break;
        case Operation_LN: {} break;
        case Operation_EXP: {} break;
        case Operation_LOG: {} break;
        case Operation_TH: {} break;
        case Operation_CTH: {} break;
        case Operation_NO_OPERATION: {} break;
        default: { assert(0 && "Unknoun opertion"); };
    }

    #undef REDUCTION
}

static Node* Dif_number(Node* node)
{
    assert(node != NULL);

    node->elem.argument.number = 0;

    return node;
}

static Node* Dif_variable(Node* node)
{
    assert(node != NULL);

    node->elem.type = Types_NUMBER;
    node->elem.argument.number = 1;

    return node;
}

static Node* Dif_operation(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    switch (node->elem.argument.operation)
    {
        case Operation_ADD:    { return Add_dif(node, pool_allocator);    }
        case Operation_SUB:    { return Sub_dif(node, pool_allocator);    }
        case Operation_MUL:    { return Mul_dif(node, pool_allocator);    }
        case Operation_DIV:    { return Dif_dif(node, pool_allocator);    }
        case Operation_COS:    { return Cos_dif(node, pool_allocator);    }
        case Operation_SIN:    { return Sin_dif(node, pool_allocator);    }
        case Operation_TG:     { return Tg_dif(node, pool_allocator);     }
        case Operation_CTG:    { return Ctg_dif(node, pool_allocator);    }
        case Operation_SH:     { return Sh_dif(node, pool_allocator);     }
        case Operation_CH:     { return Ch_dif(node, pool_allocator);     }
        case Operation_TH:     { return Th_dif(node, pool_allocator);     }
        case Operation_CTH:    { return Cth_dif(node, pool_allocator);    }
        case Operation_ARCSIN: { return ArcSin_dif(node, pool_allocator); }
        case Operation_ARCCOS: { return Arccos_dif(node, pool_allocator); }
        case Operation_ARCTG:  { return Arctg_dif(node, pool_allocator);  }
        case Operation_ARCCTG: { return Arcctg_dif(node, pool_allocator); }
        case Operation_LOG:    { return Log_dif(node, pool_allocator);    }
        case Operation_LN:     { return Ln_dif(node, pool_allocator);     }
        case Operation_EXP:    { return Exp_dif(node, pool_allocator);    } 
        case Operation_POWER:  { return Power_dif(node, pool_allocator);  }
        case Operation_NO_OPERATION: {}
        default: { assert(0 && "Unknown operation"); };
    }

    return node;
}
//(Element){.type = Types_VARIABLE, .argument.variable = 'x'}

static Node* Arcctg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);

    Node* r_r_l = Сopy_branch(node->left, NULL, r_r, pool_allocator);
    
    Node* l_r = Сopy_branch(node->left, NULL, l, pool_allocator);
    l_r = Dif(l_r, pool_allocator);

    l_l->parent = l;
    l_l->elem.type = Types_NUMBER;
    l_l->elem.argument.number = -1;

    // FIXME макрос или функция ВЕЗДЕ!!!
    l->parent = node;
    l->elem.type = Types_OPERATION;
    l->elem.argument.operation = Operation_MUL;
    l->left = l_l;
    l->right = l_r;

    r->parent = node;
    r->elem.type = Types_OPERATION;
    r->elem.argument.operation = Operation_ADD;
    r->left = r_l;
    r->right = r_r;

    r_r->parent = r;
    r_r->elem.type = Types_OPERATION;
    r_r->elem.argument.operation = Operation_POWER;
    r_r->left = r_r_l;
    r_r->right = r_r_r;

    r_l->parent = r;
    r_l->elem.type = Types_NUMBER;
    r_l->elem.argument.number = 1;

    r_r_r->parent = r_r;
    r_r_r->elem.type = Types_NUMBER;
    r_r_r->elem.argument.number = 2;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Arctg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);

    Node* r_r_l = Сopy_branch(node->left, NULL, r_r, pool_allocator);
    
    Node* l = Сopy_branch(node->left, NULL, node, pool_allocator);
    l = Dif(l, pool_allocator);

    r->parent = node;
    r->elem.type = Types_OPERATION;
    r->elem.argument.operation = Operation_ADD;
    r->left = r_l;
    r->right = r_r;

    r_r->parent = r;
    r_r->elem.type = Types_OPERATION;
    r_r->elem.argument.operation = Operation_POWER;
    r_r->left = r_r_l;
    r_r->right = r_r_r;

    r_l->parent = r;
    r_l->elem.type = Types_NUMBER;
    r_l->elem.argument.number = 1;

    r_r_r->parent = r_r;
    r_r_r->elem.type = Types_NUMBER;
    r_r_r->elem.argument.number = 2;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Arccos_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r_r = (Node*)Pool_alloc(pool_allocator);
    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);

    Node* r_l_r_l = Сopy_branch(node->left, NULL, r_l_r, pool_allocator);
    
    Node* l_r = Сopy_branch(node->left, NULL, l, pool_allocator);
    l_r = Dif(l_r, pool_allocator);

    l_l->parent = l;
    l_l->elem.type = Types_NUMBER;
    l_l->elem.argument.number = -1;

    l->parent = node;
    l->elem.type = Types_OPERATION;
    l->elem.argument.operation = Operation_MUL;
    l->left = l_l;
    l->right = l_r;

    r_l_r_r->parent = r_l_r;
    r_l_r_r->elem.type = Types_NUMBER;
    r_l_r_r->elem.argument.number = 2;

    r_l_r->parent = r_l;
    r_l_r->elem.type = Types_OPERATION;
    r_l_r->elem.argument.operation = Operation_POWER;
    r_l_r->left = r_l_r_l;
    r_l_r->right = r_l_r_r;

    r_l_l->parent = r_l;
    r_l_l->elem.type = Types_NUMBER;
    r_l_l->elem.argument.number = 1;

    r_l->parent = r;
    r_l->elem.type = Types_OPERATION;
    r_l->elem.argument.operation = Operation_SUB;
    r_l->left = r_l_l;
    r_l->right = r_l_r;

    r_r->parent = r;
    r_r->elem.type = Types_NUMBER;
    r_r->elem.argument.number = 0.5;

    r->parent = node;
    r->elem.type = Types_OPERATION;
    r->elem.argument.operation = Operation_POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* ArcSin_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r_r = (Node*)Pool_alloc(pool_allocator);

    Node* r_l_r_l = Сopy_branch(node->left, NULL, r_l_r, pool_allocator);
    
    Node* l = Сopy_branch(node->left, NULL, node, pool_allocator);
    l = Dif(l, pool_allocator);

    r_l_r_r->parent = r_l_r;
    r_l_r_r->elem.type = Types_NUMBER;
    r_l_r_r->elem.argument.number = 2;

    r_l_r->parent = r_l;
    r_l_r->elem.type = Types_OPERATION;
    r_l_r->elem.argument.operation = Operation_POWER;
    r_l_r->left = r_l_r_l;
    r_l_r->right = r_l_r_r;

    r_l_l->parent = r_l;
    r_l_l->elem.type = Types_NUMBER;
    r_l_l->elem.argument.number = 1;

    r_l->parent = r;
    r_l->elem.type = Types_OPERATION;
    r_l->elem.argument.operation = Operation_SUB;
    r_l->left = r_l_l;
    r_l->right = r_l_r;

    r_r->parent = r;
    r_r->elem.type = Types_NUMBER;
    r_r->elem.argument.number = 0.5;

    r->parent = node;
    r->elem.type = Types_OPERATION;
    r->elem.argument.operation = Operation_POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Cth_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);

    Node* l = Сopy_branch(node->right, NULL, node, pool_allocator);
    l = Dif(l, pool_allocator);

    Node* r_l_l = Сopy_branch(node->right, NULL, r_l, pool_allocator);

    r_l->parent = r;
    r_l->elem.type = Types_OPERATION;
    r_l->elem.argument.operation = Operation_SH;
    r_l->left = r_l_l;

    r_r->parent = r;
    r_r->elem.type = Types_NUMBER;
    r_r->elem.argument.number = 2;

    r->parent = node;
    r->elem.type = Types_OPERATION;
    r->elem.argument.operation = Operation_POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Th_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);

    Node* l = Сopy_branch(node->right, NULL, node, pool_allocator);
    l = Dif(l, pool_allocator);

    Node* r_l_l = Сopy_branch(node->right, NULL, r_l, pool_allocator);

    r_l->parent = r;
    r_l->elem.type = Types_OPERATION;
    r_l->elem.argument.operation = Operation_CH;
    r_l->left = r_l_l;

    r_r->parent = r;
    r_r->elem.type = Types_NUMBER;
    r_r->elem.argument.number = 2;

    r->parent = node;
    r->elem.type = Types_OPERATION;
    r->elem.argument.operation = Operation_POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Ch_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = Сopy_branch(node->right, NULL, node, pool_allocator);
    r = Dif(r, pool_allocator);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l_r = (Node*)Pool_alloc(pool_allocator);

    Node* l_l_l_l = Сopy_branch(node->right, NULL, l_l_l, pool_allocator);
    Node* l_l_r_l_l = Сopy_branch(node->right, NULL, l_l_r_l, pool_allocator);

    l_l_r_l_r->parent = l_l_r_l;
    l_l_r_l_r->elem.type = Types_NUMBER;
    l_l_r_l_r->elem.argument.number = -1;

    l_l_r_l->parent = l_l_r;
    l_l_r_l->elem.type = Types_OPERATION;
    l_l_r_l->elem.argument.operation = Operation_POWER;
    l_l_r_l->left = l_l_r_l_l;
    l_l_r_l->right = l_l_r_l_r;

    l_l_r->parent = l_l;
    l_l_r->elem.type = Types_OPERATION;
    l_l_r->elem.argument.operation = Operation_EXP;
    l_l_r->left = l_l_r_l;

    l_l_l->parent = l_l;
    l_l_l->elem.type = Types_OPERATION;
    l_l_l->elem.argument.operation = Operation_EXP;
    l_l_l->left = l_l_l_l;

    l_l->parent = l;
    l_l->elem.type = Types_OPERATION;
    l_l->elem.argument.operation = Operation_SUB;
    l_l->left = l_l_l;
    l_l->right = l_l_r;

    l_r->parent = l;
    l_r->elem.type = Types_NUMBER;
    l_r->elem.argument.number = 2;

    l->parent = node;
    l->elem.type = Types_OPERATION;
    l->elem.argument.operation = Operation_DIV;
    l->left = l_l;
    l->right = l_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_MUL;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Sh_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r = Сopy_branch(node->right, NULL, node, pool_allocator);
    r = Dif(r, pool_allocator);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l_r = (Node*)Pool_alloc(pool_allocator);

    Node* l_l_l_l = Сopy_branch(node->right, NULL, l_l_l, pool_allocator);
    Node* l_l_r_l_l = Сopy_branch(node->right, NULL, l_l_r_l, pool_allocator);

    l_l_r_l_r->parent = l_l_r_l;
    l_l_r_l_r->elem.type = Types_NUMBER;
    l_l_r_l_r->elem.argument.number = -1;

    l_l_r_l->parent = l_l_r;
    l_l_r_l->elem.type = Types_OPERATION;
    l_l_r_l->elem.argument.operation = Operation_POWER;
    l_l_r_l->left = l_l_r_l_l;
    l_l_r_l->right = l_l_r_l_r;

    l_l_r->parent = l_l;
    l_l_r->elem.type = Types_OPERATION;
    l_l_r->elem.argument.operation = Operation_EXP;
    l_l_r->left = l_l_r_l;

    l_l_l->parent = l_l;
    l_l_l->elem.type = Types_OPERATION;
    l_l_l->elem.argument.operation = Operation_EXP;
    l_l_l->left = l_l_l_l;

    l_l->parent = l;
    l_l->elem.type = Types_OPERATION;
    l_l->elem.argument.operation = Operation_ADD;
    l_l->left = l_l_l;
    l_l->right = l_l_r;

    l_r->parent = l;
    l_r->elem.type = Types_NUMBER;
    l_r->elem.argument.number = 2;

    l->parent = node;
    l->elem.type = Types_OPERATION;
    l->elem.argument.operation = Operation_DIV;
    l->left = l_l;
    l->right = l_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_MUL;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Ctg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* right_right = (Node*)Pool_alloc(pool_allocator);
    Node* right = (Node*)Pool_alloc(pool_allocator);

    right_right->parent = right;
    right_right->elem.type = Types_NUMBER;
    right_right->elem.argument.number = 2;

    Node* right_left = (Node*)Pool_alloc(pool_allocator);
    Node* right_left_left = Сopy_branch(node->right, NULL, right_left, pool_allocator);

    right_left->parent = right;
    right_left->elem.type = Types_OPERATION;
    right_left->elem.argument.operation = Operation_SIN;
    right_left->left = right_left_left;

    right->parent = node;
    right->elem.type = Types_OPERATION;
    right->elem.argument.operation = Operation_POWER;
    right->left = right_left;
    right->right = right_right;

    Node* left = (Node*)Pool_alloc(pool_allocator);
    left->parent = node;
    left->elem.type = Types_OPERATION;
    left->elem.argument.operation = Operation_MUL;

    Node* left_right = (Node*)Pool_alloc(pool_allocator);
    left_right->elem.type = Types_NUMBER;
    left_right->elem.argument.number = -1;
    left_right->parent = left;


    Node* left_left = Сopy_branch(node->right, NULL, left, pool_allocator);
    left_left = Dif(left_left, pool_allocator);

    left->left = left_left;
    left->right = left_right;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Tg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* right_right = (Node*)Pool_alloc(pool_allocator);
    Node* right = (Node*)Pool_alloc(pool_allocator);

    right_right->parent = right;
    right_right->elem.type = Types_NUMBER;
    right_right->elem.argument.number = 2;

    Node* right_left = (Node*)Pool_alloc(pool_allocator);
    Node* right_left_left = Сopy_branch(node->right, NULL, right_left, pool_allocator);

    right_left->parent = right;
    right_left->elem.type = Types_OPERATION;
    right_left->elem.argument.operation = Operation_COS;
    right_left->left = right_left_left;

    right->parent = node;
    right->elem.type = Types_OPERATION;
    right->elem.argument.operation = Operation_POWER;
    right->left = right_left;
    right->right = right_right;

    Node* left = Сopy_branch(node->right, NULL, node, pool_allocator);
    left = Dif(left, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Exp_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    
    Node* right = Сopy_branch(node->left, NULL, node, pool_allocator);

    Node* left = Сopy_branch(node, NULL, node, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_MUL;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Power_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* right = Сopy_branch(node, NULL, node, pool_allocator);

    Node* left = (Node*)Pool_alloc(pool_allocator);
    Node* left_left = (Node*)Pool_alloc(pool_allocator);

    Node* left_left_left = Сopy_branch(node->left, NULL, left_left, pool_allocator);

    Node* left_right = Сopy_branch(node->right, NULL, left, pool_allocator);

    left->parent = node;
    left->left = left_left;
    left->right = left_right;
    left->elem.type = Types_OPERATION;
    left->elem.argument.operation = Operation_MUL;

    left_left->parent = left;
    left_left->left = left_left_left;
    left_left->elem.type = Types_OPERATION;
    left_left->elem.argument.operation = Operation_LN;

    left->parent = node;
    left->left = left_left;
    left->right = left_right;
    left->elem.type = Types_OPERATION;
    left->elem.argument.operation = Operation_MUL;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_MUL;
    node->left = left;
    node->right = right;

    left = Dif(left, pool_allocator);

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Log_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* left = (Node*)Pool_alloc(pool_allocator);
    Node* right = (Node*)Pool_alloc(pool_allocator);

    Node* left_left = Сopy_branch(node->right, NULL, left, pool_allocator);
    Node* right_left = Сopy_branch(node->left, NULL, right, pool_allocator);

    left->parent = node;
    left->left = left_left;
    left->elem.type = Types_OPERATION;
    left->elem.argument.operation = Operation_LN;

    right->parent = node;
    right->left = right_left;
    right->elem.type = Types_OPERATION;
    right->elem.argument.operation = Operation_LN;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_DIV;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    node = Dif(node, pool_allocator);

    return node;
}

static Node* Ln_dif(Node* node, Pool_allocator* pool_allocator)
{

    assert(node != NULL);

    node->elem.argument.operation = Operation_DIV;

    Node* left = Сopy_branch(node->left, NULL, node, pool_allocator);
    left = Dif(left, pool_allocator);

    Node* right = Сopy_branch(node->left, NULL, node, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Add_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* left = Сopy_branch(node->left, NULL, node, pool_allocator);
    left = Dif(left, pool_allocator);

    Node* right = Сopy_branch(node->right, NULL, node, pool_allocator);
    right = Dif(right, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);
    
    return node;
}


static Node* Sub_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* left = Сopy_branch(node->left, NULL, node, pool_allocator);
    left = Dif(left, pool_allocator);

    Node* right = Сopy_branch(node->right, NULL, node, pool_allocator);
    right = Dif(right, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);
    
    return node;
}

static Node* Mul_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* new_left = (Node*)Pool_alloc(pool_allocator);
    Node* new_right = (Node*)Pool_alloc(pool_allocator);

    new_left->parent = node;
    new_right->parent = node;

    new_left->elem.type = Types_OPERATION;
    new_right->elem.type = Types_OPERATION;
    new_left->elem.argument.operation = Operation_MUL;
    new_right->elem.argument.operation = Operation_MUL;

    Node* node_left_left = Сopy_branch(node->right, NULL , new_left, pool_allocator);
    Node* node_right_right = Сopy_branch(node->left, NULL , new_right, pool_allocator);
    Node* node_left_right = Сopy_branch(node->right, NULL , new_left, pool_allocator);
    Node* node_right_left = Сopy_branch(node->left, NULL, new_right, pool_allocator);

    node_left_left = Dif(node_left_left, pool_allocator);
    node_right_right = Dif(node_right_right, pool_allocator);

    new_left->left = node_left_left;
    new_right->right = node_right_right;

    node->elem.argument.operation = Operation_ADD;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = new_left;
    node->right = new_right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    node_left_left->parent = new_left;
    node_right_right->parent = new_right;
    new_left->right = node_right_left;
    new_right->left = node_left_right;

    return node;
}

static Node* Dif_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* r_right = (Node*)Pool_alloc(pool_allocator);
    Node* r_right_right = (Node*)Pool_alloc(pool_allocator);

    Node* r_right_left = Сopy_branch(node->right, NULL , r_right, pool_allocator);

    r_right->parent = node;
    r_right->elem.type = Types_OPERATION;
    r_right->elem.argument.operation = Operation_POWER;
    r_right->left = r_right_left;
    r_right->right = r_right_right;

    r_right_right->parent = r_right;
    r_right_right->elem.type = Types_NUMBER;
    r_right_right->elem.argument.number = 2;


    Node* left = (Node*)Pool_alloc(pool_allocator);
    Node* left_left = (Node*)Pool_alloc(pool_allocator);
    Node* left_right = (Node*)Pool_alloc(pool_allocator);

    Node* left_left_right = Сopy_branch(node->right, NULL , left_left, pool_allocator);
    Node* left_right_left = Сopy_branch(node->left, NULL , left_right, pool_allocator);

    Node* left_left_left = Сopy_branch(node->left, NULL , left_left, pool_allocator);
    left_left_left = Dif(left_left_left, pool_allocator);

    Node* left_right_right = Сopy_branch(node->right, NULL , left_right, pool_allocator);
    left_right_right = Dif(left_right_right, pool_allocator);

    left_left->parent = left;
    left_left->elem.type = Types_OPERATION;
    left_left->elem.argument.operation = Operation_MUL;
    left_left->left = left_left_left;
    left_left->right = left_left_right;

    left_right->parent = left;
    left_right->elem.type = Types_OPERATION;
    left_right->elem.argument.operation = Operation_MUL;
    left_right->left = left_right_left;
    left_right->right = left_right_right;

    left->parent = node;
    left->elem.type = Types_OPERATION;
    left->elem.argument.operation = Operation_SUB;
    left->left = left_left;
    left->right = left_right;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = left;
    node->right = r_right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Cos_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* left = (Node*)Pool_alloc(pool_allocator);
    Node* left_right = (Node*)Pool_alloc(pool_allocator);

    Node* left_left = Сopy_branch(node->left, NULL , left, pool_allocator);

    left_right->parent = left;
    left_right->elem.type = Types_NUMBER;
    left_right->elem.argument.number = 0;

    left->parent = node;
    left->elem.type = Types_OPERATION;
    left->elem.argument.operation = Operation_SIN;
    left->left = left_left;
    left->right = left_right;

    Node* right = Сopy_branch(node->left, NULL, node, pool_allocator);
    right = Dif(right, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_MUL;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

static Node* Sin_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    //(Element){.type = Types_VARIABLE, .argument.variable = 'x'}

    Node* right = (Node*)Pool_alloc(pool_allocator);
    Node* left = (Node*)Pool_alloc(pool_allocator);
    Node* right_right = (Node*)Pool_alloc(pool_allocator);
    Node* left_right = (Node*)Pool_alloc(pool_allocator);

    Node* left_left = Сopy_branch(node->left, NULL , left, pool_allocator);
    
    left_right->parent = left;
    left_right->elem.type = Types_NUMBER;
    left_right->elem.argument.number = 0;

    left->parent = node;
    left->elem.type = Types_OPERATION;
    left->elem.argument.operation = Operation_COS;
    left->left = left_left;
    left->right = left_right;

    Node* right_left = Сopy_branch(node->left, NULL , right, pool_allocator);
    right_left = Dif(right_left, pool_allocator);

    right_right->parent = right;
    right_right->elem.type = Types_NUMBER;
    right_right->elem.argument.number = -1;

    right->parent = node;
    right->elem.type = Types_OPERATION;
    right->elem.argument.operation = Operation_MUL;
    right->left = right_left;
    right->right = right_right;


    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_MUL;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}
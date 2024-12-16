#include "dif_function.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include "dif_function.h"
#include "utils.h"
#include "tree_utils.h"
#include "com.h"
#include "tree.h"


static void Fold_constants(Node* node);
static void Reduce_redundant_ops(Node* node, Pool_allocator* pool_allocator);

static Node* Add_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Sub_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Mul_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Dif_div   (Node* node, Pool_allocator* Pool_allocator);
static Node* Cos_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Sin_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Ln_dif    (Node* node, Pool_allocator* Pool_allocator);
static Node* Log_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Power_dif (Node* node, Pool_allocator* Pool_allocator);
static Node* Exp_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Tg_dif    (Node* node, Pool_allocator* Pool_allocator);
static Node* Ctg_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Sh_dif    (Node* node, Pool_allocator* Pool_allocator);
static Node* Ch_dif    (Node* node, Pool_allocator* Pool_allocator);
static Node* Th_dif    (Node* node, Pool_allocator* Pool_allocator);
static Node* Cth_dif   (Node* node, Pool_allocator* Pool_allocator);
static Node* Arcsin_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Arccos_dif(Node* node, Pool_allocator* Pool_allocator);
static Node* Arctg_dif (Node* node, Pool_allocator* Pool_allocator);
static Node* Arcctg_dif(Node* node, Pool_allocator* Pool_allocator);

static Node* Dif_number   (Node* node);
static Node* Dif_variable (Node* node);
static Node* Dif_operation(Node* node, Pool_allocator* pool_allocator);

Node* Dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);
    
    Calculation(node);
    Simplification(node, pool_allocator); 

    if (Is_number(node))    { return Dif_number(node);                    }
    if (Is_veriable(node))  { return Dif_variable(node);                  }
    if (Is_operation(node)) { return Dif_operation(node, pool_allocator); }

    return node;
}

void Calculation(Node* node)
{
    if (!Is_operation(node)) { return; }

    DPRINTF("\n%p\n", node);
    DPRINTF("\n%p\n", node->left);

    if (Is_operation(node->left))  { Calculation(node->left);  }
    if (Is_operation(node->right)) { Calculation(node->right); }

    if (Is_number(node->left) && Is_number(node->right)) { Fold_constants(node); }

    return;
}

void Simplification(Node* node, Pool_allocator* pool_allocator)
{
    if (!Is_operation(node)) { return; }

    if (Is_operation(node->left)) { Simplification(node->left, pool_allocator); }
    if (Is_operation(node->right)) { Simplification(node->right, pool_allocator); }

    Reduce_redundant_ops(node, pool_allocator);

    return;
}

static void Fold_constants(Node* node)
{
    assert(node != NULL);

    #define CASE_NOT_USE(op) case op: {} break;

    #define CASE(op, op_sign) case op:                                                                           \
    {                                                                                                            \
        node->elem.type = Types_NUMBER;                                                                          \
        node->elem.argument.number = node->left->elem.argument.number op_sign node->right->elem.argument.number; \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    #define CASE_Operation_POWER(op) case op:                                                                    \
    {                                                                                                            \
        node->elem.type = Types_NUMBER;                                                                          \
        node->elem.argument.number = pow(node->left->elem.argument.number, node->right->elem.argument.number);   \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    #define CASE_ANOTHER(op, op_sign) case op:                                                                   \
    {                                                                                                            \
        node->elem.type = Types_NUMBER;                                                                          \
        node->elem.argument.number = op_sign(node->left->elem.argument.number);                                  \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    switch(node->elem.argument.operation)
    {
        CASE                (Operation_ADD,    +   );
        CASE                (Operation_SUB,    -   );
        CASE                (Operation_MUL,    *   );
        CASE                (Operation_DIV,    /   );

        CASE_ANOTHER        (Operation_SIN,    sin );
        CASE_ANOTHER        (Operation_COS,    cos );
        CASE_ANOTHER        (Operation_TG,     tan );
        CASE_ANOTHER        (Operation_SH,     sinh);
        CASE_ANOTHER        (Operation_CH,     cosh);
        CASE_ANOTHER        (Operation_ARCSIN, asin);
        CASE_ANOTHER        (Operation_ARCCOS, acos);
        CASE_ANOTHER        (Operation_ARCTG,  atan);
        CASE_ANOTHER        (Operation_LOG,    log );
        CASE_ANOTHER        (Operation_EXP,    exp );
        
        CASE_NOT_USE        (Operation_CTH         );
        CASE_NOT_USE        (Operation_TH          );
        CASE_NOT_USE        (Operation_CTG         );
        CASE_NOT_USE        (Operation_LN          );
        CASE_NOT_USE        (Operation_ARCCTG      );
        CASE_NOT_USE        (Operation_NO_OPERATION);
        CASE_Operation_POWER(Operation_POWER       );
        default: { assert(0 && "Unknoun operation"); };
    }

    #undef CASE
    #undef CASE_NOT_USE
    #undef CASE_Operation_POWER
    #undef CASE_ANOTHER
}

static void Reduce_redundant_ops(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    #define REDUCTION(op_enum, op, value)       \
        node->elem.type = op_enum;              \
        node->elem.argument.op = value;         \
        Tree_dtor(node->left, pool_allocator);  \
        Tree_dtor(node->right, pool_allocator); \
        node->left = NULL;                      \
        node->right = NULL;

    #define REDUCTION_F(num, fun)                                                                                \
        Node* f = node->fun;                                                                                     \
        Node* one_or_null = node->num;                                                                           \
                                                                                                                 \
        node->elem.type = node->fun->elem.type;                                                                  \
                                                                                                                 \
        switch(node->fun->elem.type)                                                                             \
        {                                                                                                        \
            case Types_VARIABLE:  { node->elem.argument.variable = node->fun->elem.argument.variable;   } break; \
            case Types_NUMBER:    { node->elem.argument.number = node->fun->elem.argument.number;       } break; \
            case Types_OPERATION: { node->elem.argument.operation = node->fun->elem.argument.operation; } break; \
            default: { assert(0 && "No tipes"); }                                                                \
        }                                                                                                        \
                                                                                                                 \
        node->num = node->fun->num;                                                                              \
        node->fun = node->fun->fun;                                                                              \
                                                                                                                 \
        free(one_or_null);                                                                                       \
        free(f);

    switch(node->elem.argument.operation)
    {
        case Operation_ADD:
        {
            if (Is_number(node->right) && Is_equal(node->right->elem.argument.number, 0))
            {
                REDUCTION_F(right, left);
            }
            else if (Is_number(node->left) && Is_equal(node->left->elem.argument.number, 0))
            {
                REDUCTION_F(left, right);
            }
        } 
        break;

        case Operation_MUL:
        {
            if (Is_number(node->right) && Is_equal(node->right->elem.argument.number, 0))
            {
                REDUCTION(Types_NUMBER, number, 0);
            }
            else if (Is_number(node->left) && Is_equal(node->left->elem.argument.number, 0))
            {
                REDUCTION(Types_NUMBER, number, 0);
            }
            else if (Is_number(node->right) && Is_equal(node->right->elem.argument.number, 1))
            {
                REDUCTION_F(right, left);
            }
            else if (Is_number(node->left) && Is_equal(node->left->elem.argument.number, 1))
            {
                REDUCTION_F(left, right);
            }
        } 
        break;

        case Operation_SUB:
        {
            if (Is_number(node->right) && Is_equal(node->right->elem.argument.number, 0))
            {
                REDUCTION_F(right, left);
            }
        } 
        break;

        case Operation_POWER:
        {
            if (Is_number(node->right) && Is_equal(node->right->elem.argument.number, 0))
            {
                REDUCTION(Types_NUMBER, number, 1);
            }
            else if (Is_number(node->left) && Is_equal(node->left->elem.argument.number, 0))
            {
                REDUCTION(Types_NUMBER, number, 1);
            }
            else if (Is_number(node->right) && Is_equal(node->right->elem.argument.number, 1))
            {
                REDUCTION_F(right, left);
            }
            else if (Is_number(node->left) && Is_equal(node->left->elem.argument.number, 1))
            {
                REDUCTION_F(left, right);
            }
        } 
        break;

        case Operation_DIV:
        {
            if (Is_number(node->right) && Is_equal(node->right->elem.argument.number, 1))
            {
                REDUCTION_F(right, left);
            }
        }
        break;

        case Operation_COS:          {} break;
        case Operation_SIN:          {} break;
        case Operation_TG:           {} break;
        case Operation_CTG:          {} break;
        case Operation_SH:           {} break;
        case Operation_CH:           {} break;
        case Operation_ARCCTG:       {} break;
        case Operation_ARCTG:        {} break;
        case Operation_ARCSIN:       {} break;
        case Operation_ARCCOS:       {} break;
        case Operation_LN:           {} break;
        case Operation_EXP:          {} break;
        case Operation_LOG:          {} break;
        case Operation_TH:           {} break;
        case Operation_CTH:          {} break;
        case Operation_NO_OPERATION: {} break;
        default: { assert(0 && "Unknoun opertion"); };
    }

    #undef REDUCTION
    #undef REDUCTION_F

    // fprintf(stderr, "node: %p\n", node);
    // fprintf(stderr, "node->left: %p\n", node->left);
    // fprintf(stderr, "node->right: %p\n",  node->right);
    // fprintf(stderr, "node->left->left: %p\n", node->left->left);
    // fprintf(stderr, "node->left->right: %p\n", node->left->right);
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
        case Operation_SUB:    { return Sub_dif    (node, pool_allocator); }
        case Operation_ADD:    { return Add_dif    (node, pool_allocator); }
        case Operation_MUL:    { return Mul_dif    (node, pool_allocator); }
        case Operation_DIV:    { return Dif_div    (node, pool_allocator); }
        case Operation_COS:    { return Cos_dif    (node, pool_allocator); }
        case Operation_SIN:    { return Sin_dif    (node, pool_allocator); }
        case Operation_TG:     { return Tg_dif     (node, pool_allocator); }
        case Operation_CTG:    { return Ctg_dif    (node, pool_allocator); }
        case Operation_SH:     { return Sh_dif     (node, pool_allocator); }
        case Operation_CH:     { return Ch_dif     (node, pool_allocator); }
        case Operation_TH:     { return Th_dif     (node, pool_allocator); }
        case Operation_CTH:    { return Cth_dif    (node, pool_allocator); }
        case Operation_ARCSIN: { return Arcsin_dif (node, pool_allocator); }
        case Operation_ARCCOS: { return Arccos_dif (node, pool_allocator); }
        case Operation_ARCTG:  { return Arctg_dif  (node, pool_allocator); }
        case Operation_ARCCTG: { return Arcctg_dif (node, pool_allocator); }
        case Operation_LOG:    { return Log_dif    (node, pool_allocator); }
        case Operation_LN:     { return Ln_dif     (node, pool_allocator); }
        case Operation_EXP:    { return Exp_dif    (node, pool_allocator); } 
        case Operation_POWER:  { return Power_dif  (node, pool_allocator); }
        case Operation_NO_OPERATION: {}

        default: { assert(0 && "Unknown operation"); };
    }

    return node;
}
//(Element){.type = Types_VARIABLE, .argument.variable = 'x'}

void Link_node_dif(Node* node, Node* parent, Node* left, Node* right, Element elem)
{
    node->parent = parent;
    node->left = left;
    node->right = right;

    node->elem = elem;
}

static Node* Arcctg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);

    Node* r_r_l = NULL;
    if (node->left  != NULL) { r_r_l = Сopy_branch(node->left,   r_r, pool_allocator); }
    if (node->right != NULL) { r_r_l = Сopy_branch(node->right,  r_r, pool_allocator); }
    
    Node* l_r = NULL;
    if (node->left != NULL) { l_r = Сopy_branch(node->left,  l, pool_allocator); l_r = Dif(l_r, pool_allocator); }
    if (node->right != NULL) { l_r = Сopy_branch(node->right,  l, pool_allocator); l_r = Dif(l_r, pool_allocator); }

    Link_node_dif(l_l, l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = -1}});
    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_ADD}});
    Link_node_dif(r_l, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 1}});
    Link_node_dif(r_r, r, r_r_l, r_r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r_r, r_r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Arctg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r_r = (Node*)Pool_alloc(pool_allocator);

    Node* r_r_l = NULL;
    if (node->left != NULL) { r_r_l = Сopy_branch(node->left,  r_r, pool_allocator); }
    if (node->right != NULL) { r_r_l = Сopy_branch(node->right,  r_r, pool_allocator); }
    
    Node* l = NULL;
    if (node->left != NULL) { l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator); }
    if (node->right != NULL) { l = Сopy_branch(node->right,  node, pool_allocator); l = Dif(l, pool_allocator); }

    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_ADD}});
    Link_node_dif(r_l, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 1}});
    Link_node_dif(r_r, r, r_r_l, r_r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r_r, r_r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Arccos_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r_r = (Node*)Pool_alloc(pool_allocator);
    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);

    Node* r_l_r_l = NULL;
    if (node->left != NULL) { r_l_r_l = Сopy_branch(node->left,  r_l_r, pool_allocator); }
    if (node->right != NULL) { r_l_r_l = Сopy_branch(node->right,  r_l_r, pool_allocator); }
    
    Node* l_r = NULL;
    if (node->left != NULL) { l_r = Сopy_branch(node->left,  l, pool_allocator); l_r = Dif(l_r, pool_allocator); }
    if (node->right != NULL) { l_r = Сopy_branch(node->right,  l, pool_allocator); l_r = Dif(l_r, pool_allocator); }

    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 0.5}});
    Link_node_dif(r_l, r, r_l_l, r_l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SUB}});
    Link_node_dif(r_l_l, r_l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 1}});
    Link_node_dif(r_l_r, r_l, r_l_r_l, r_l_r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_l_r_r, r_l_r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});

    Link_node_dif(l_l, l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = -1}});
    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Arcsin_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* r_l_r_r = (Node*)Pool_alloc(pool_allocator);

    Node* r_l_r_l = NULL;
    if (node->left != NULL) { r_l_r_l = Сopy_branch(node->left,  r_l_r, pool_allocator); }
    if (node->right != NULL) { r_l_r_l = Сopy_branch(node->right,  r_l_r, pool_allocator); }
    
    Node* l = NULL;
    if (node->left != NULL) { l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator); }
    if (node->right != NULL) { l = Сopy_branch(node->right,  node, pool_allocator); l = Dif(l, pool_allocator); }

    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 0.5}});
    Link_node_dif(r_l, r, r_l_l, r_l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SUB}});
    Link_node_dif(r_l_l, r_l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 1}});
    Link_node_dif(r_l_r, r_l, r_l_r_l, r_l_r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_l_r_r, r_l_r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Cth_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);

    Node* r_l_l = NULL;
    if (node->left != NULL) { r_l_l = Сopy_branch(node->left,  r_l, pool_allocator); }
    if (node->right != NULL) { r_l_l = Сopy_branch(node->right,  r_l, pool_allocator); }
    
    Node* l = NULL;
    if (node->left != NULL) { l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator); }
    if (node->right != NULL) { l = Сopy_branch(node->right,  node, pool_allocator); l = Dif(l, pool_allocator); }

    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});
    Link_node_dif(r_l, r, r_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SH}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Th_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);

    Node* r_l_l = NULL;
    if (node->left != NULL) { r_l_l = Сopy_branch(node->left,  r_l, pool_allocator); }
    if (node->right != NULL) { r_l_l = Сopy_branch(node->right,  r_l, pool_allocator); }
    
    Node* l = NULL;
    if (node->left != NULL) { l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator); }
    if (node->right != NULL) { l = Сopy_branch(node->right,  node, pool_allocator); l = Dif(l, pool_allocator); }

    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});
    Link_node_dif(r_l, r, r_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_CH}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Ch_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l_r = (Node*)Pool_alloc(pool_allocator);

    Node* l_l_l_l = NULL;
    if (node->left != NULL) { l_l_l_l = Сopy_branch(node->left,  l_l_l, pool_allocator); }
    if (node->right != NULL) { l_l_l_l = Сopy_branch(node->right,  l_l_l, pool_allocator); }
    
    Node* l_l_r_l_l = NULL;
    if (node->left != NULL) { l_l_r_l_l = Сopy_branch(node->left,  l_l_r_l, pool_allocator); }
    if (node->right != NULL) { l_l_r_l_l = Сopy_branch(node->right,  l_l_r_l, pool_allocator); }

    Node* r = NULL;
    if (node->left != NULL) { r = Сopy_branch(node->left,  node, pool_allocator); r = Dif(r, pool_allocator); }
    if (node->right != NULL) { r = Сopy_branch(node->right,  node, pool_allocator); r = Dif(r, pool_allocator); }

    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});
    Link_node_dif(l_l, l, l_l_l, l_l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SUB}});
    Link_node_dif(l_r, l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});
    Link_node_dif(l_l_l, l_l, l_l_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_EXP}});
    Link_node_dif(l_l_r, l_l, l_l_r_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_EXP}});
    Link_node_dif(l_l_r_l, l_l_r, l_l_r_l_l, l_l_r_l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(l_l_r_l_r, l_l_r_l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = -1}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Sh_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l_r_l_r = (Node*)Pool_alloc(pool_allocator);

    Node* l_l_l_l = NULL;
    if (node->left != NULL) { l_l_l_l = Сopy_branch(node->left,  l_l_l, pool_allocator); }
    if (node->right != NULL) { l_l_l_l = Сopy_branch(node->right,  l_l_l, pool_allocator); }
    
    Node* l_l_r_l_l = NULL;
    if (node->left != NULL) { l_l_r_l_l = Сopy_branch(node->left,  l_l_r_l, pool_allocator); }
    if (node->right != NULL) { l_l_r_l_l = Сopy_branch(node->right,  l_l_r_l, pool_allocator); }

    Node* r = NULL;
    if (node->left != NULL) { r = Сopy_branch(node->left,  node, pool_allocator); r = Dif(r, pool_allocator); }
    if (node->right != NULL) { r = Сopy_branch(node->right,  node, pool_allocator); r = Dif(r, pool_allocator); }

    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});
    Link_node_dif(l_l, l, l_l_l, l_l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_ADD}});
    Link_node_dif(l_r, l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});
    Link_node_dif(l_l_l, l_l, l_l_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_EXP}});
    Link_node_dif(l_l_r, l_l, l_l_r_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_EXP}});
    Link_node_dif(l_l_r_l, l_l_r, l_l_r_l_l, l_l_r_l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(l_l_r_l_r, l_l_r_l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = -1}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Ctg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_r = (Node*)Pool_alloc(pool_allocator);

    Node* l_l = NULL;
    if (node->left != NULL) { l_l = Сopy_branch(node->left,  l, pool_allocator); l_l = Dif(l_l, pool_allocator); }
    if (node->right != NULL) { l_l = Сopy_branch(node->right,  l, pool_allocator); l_l = Dif(l_l, pool_allocator); }
    
    Node* r_l_l = NULL;
    if (node->left != NULL) { r_l_l = Сopy_branch(node->left,  r_l, pool_allocator); }
    if (node->right != NULL) { r_l_l = Сopy_branch(node->right,  r_l, pool_allocator); }

    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(l_r, l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = -1}});
    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});
    Link_node_dif(r_l, r, r_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SIN}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Tg_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* r_l = (Node*)Pool_alloc(pool_allocator);

    Node* l = NULL;
    if (node->left != NULL) { l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator); }
    if (node->right != NULL) { l = Сopy_branch(node->right,  node, pool_allocator); l = Dif(l, pool_allocator); }
    
    Node* r_l_l = NULL;
    if (node->left != NULL) { r_l_l = Сopy_branch(node->left,  r_l, pool_allocator); }
    if (node->right != NULL) { r_l_l = Сopy_branch(node->right,  r_l, pool_allocator); }

    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});
    Link_node_dif(r_l, r, r_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_COS}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Exp_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);
    
    Node* l = Сopy_branch(node, node, pool_allocator);
    
    Node* r = NULL;
    if (node->left != NULL) { r = Сopy_branch(node->left,  node, pool_allocator); r = Dif(r, pool_allocator); }
    if (node->right != NULL) { r = Сopy_branch(node->right,  node, pool_allocator); r = Dif(r, pool_allocator); }

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Power_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);

    Node* r = Сopy_branch(node, node, pool_allocator);
    Node* l_r = Сopy_branch(node->right, l, pool_allocator);
    Node* l_l_l = Сopy_branch(node->left, l_l, pool_allocator);

    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(l_l, l, l_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_LN}});

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});

    Tree_dtor(left_dtor, pool_allocator);
    Tree_dtor(right_dtor, pool_allocator);

    node->left = Dif(node->left, pool_allocator);

    return node;
}

static Node* Log_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* r = (Node*)Pool_alloc(pool_allocator);

    Node* l_l = Сopy_branch(node->right,  l, pool_allocator);
    Node* r_l = Сopy_branch(node->left,  r, pool_allocator);

    Link_node_dif(l, node, l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_LN}});
    Link_node_dif(r, node, r_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_LN}});

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(left_dtor, pool_allocator);
    Tree_dtor(right_dtor, pool_allocator);

    node = Dif(node, pool_allocator);

    return node;
}

static Node* Ln_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* r = NULL;
    if (node->left != NULL) { r = Сopy_branch(node->left,  node, pool_allocator); }
    if (node->right != NULL) { r = Сopy_branch(node->right,  node, pool_allocator); }
    
    Node* l = NULL;
    if (node->left != NULL) { l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator); }
    if (node->right != NULL) { l = Сopy_branch(node->right,  node, pool_allocator); l = Dif(l, pool_allocator); }

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Add_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator);
    Node* r = Сopy_branch(node->right,  node, pool_allocator); r = Dif(r, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_ADD}});

    Tree_dtor(left_dtor, pool_allocator);
    Tree_dtor(right_dtor, pool_allocator);

    return node;
}


static Node* Sub_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = Сopy_branch(node->left,  node, pool_allocator); l = Dif(l, pool_allocator);
    Node* r = Сopy_branch(node->right,  node, pool_allocator); r = Dif(r, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SUB}});

    Tree_dtor(left_dtor, pool_allocator);
    Tree_dtor(right_dtor, pool_allocator);

    return node;
}

static Node* Mul_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* left = (Node*)Pool_alloc(pool_allocator);
    Node* right = (Node*)Pool_alloc(pool_allocator);

    Node* left_left = Сopy_branch(node->left, left, pool_allocator);
    Node* left_right = Сopy_branch(node->right, left, pool_allocator);
    Node* right_left = Сopy_branch(node->left, right, pool_allocator);
    Node* right_right = Сopy_branch(node->right, right, pool_allocator);

    Link_node_dif(left, node, left_left, left_right, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(right, node, right_left, right_right, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});

    left_left = Dif(left_left, pool_allocator);
    right_right = Dif(right_right, pool_allocator);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = Operation_ADD;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor, pool_allocator);
    Tree_dtor(right_dtor, pool_allocator);

    return node;
}

static Node* Dif_div(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* r = (Node*)Pool_alloc(pool_allocator);
    Node* r_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);
    Node* l_r = (Node*)Pool_alloc(pool_allocator);

    Node* l_l_l = Сopy_branch(node->left, l_l, pool_allocator);
    Node* l_l_r = Сopy_branch(node->right, l_l, pool_allocator);
    Node* l_r_l = Сopy_branch(node->left, l_r, pool_allocator);
    Node* l_r_r = Сopy_branch(node->right, l_r, pool_allocator);
    Node* r_l = Сopy_branch(node->right, r, pool_allocator);

    l_l_l = Dif(l_l_l, pool_allocator);
    l_r_r = Dif(l_r_r, pool_allocator);

    Link_node_dif(l_l, l, l_l_l, l_l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(l_r, l, l_r_l, l_r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SUB}});
    Link_node_dif(r, node, r_l, r_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_POWER}});
    Link_node_dif(r_r, r, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = 2}});

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_DIV}});

    Tree_dtor(left_dtor, pool_allocator);
    Tree_dtor(right_dtor, pool_allocator);

    return node;
}

static Node* Cos_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);
    Node* l_r = (Node*)Pool_alloc(pool_allocator);
    Node* l_l = (Node*)Pool_alloc(pool_allocator);

    Node* l_l_l = NULL;
    if (node->left != NULL) { l_l_l = Сopy_branch(node->left,  l_l, pool_allocator); }
    if (node->right != NULL) { l_l_l = Сopy_branch(node->right,  l_l, pool_allocator); }
    
    Node* r = NULL;
    if (node->left != NULL) { r = Сopy_branch(node->left, node, pool_allocator); r = Dif(r, pool_allocator); }
    if (node->right != NULL) { r = Сopy_branch(node->right, node, pool_allocator); r = Dif(r, pool_allocator); }

    Link_node_dif(l, node, l_l, l_r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(l_r, l, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = -1}});
    Link_node_dif(l_l, l, l_l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SIN}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}

static Node* Sin_dif(Node* node, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(pool_allocator != NULL);

    Node* l = (Node*)Pool_alloc(pool_allocator);

    Node* l_l = NULL;
    if (node->left != NULL) { l_l = Сopy_branch(node->left,  l, pool_allocator); }
    if (node->right != NULL) { l_l = Сopy_branch(node->right,  l, pool_allocator); }
    
    Node* r = NULL;
    if (node->left != NULL) { r = Сopy_branch(node->left, node, pool_allocator); r = Dif(r, pool_allocator); }
    if (node->right != NULL) { r = Сopy_branch(node->right, node, pool_allocator); r = Dif(r, pool_allocator); }

    Link_node_dif(l, node, l_l, NULL, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_COS}});

    Node* node_dtor = NULL;
    if (node->left  != NULL) { node_dtor = node->left;  }
    if (node->right != NULL) { node_dtor = node->right; }

    Link_node_dif(node, node->parent, l, r, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});

    Tree_dtor(node_dtor, pool_allocator);

    return node;
}
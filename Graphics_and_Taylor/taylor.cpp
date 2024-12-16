#include "taylor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>

#include "../Graphics_and_Taylor/graf.h"
#include "../com.h"
#include "../dif_function.h"

static Node* Create(Node* parent, Node* left, Node* right, Element elem);
static void Link(Node* node);

Node* Assembling_formula(Node* node, int64_t order_of_decomposition,
                             int64_t function_point, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* root = Calculating_the_n_term(node, 0, function_point, pool_allocator);

    for (int i = 1; i <= order_of_decomposition; i++)
    {
        Node* node_2 = Calculating_the_n_term(node, i, function_point, pool_allocator);
        root = Create(NULL, root, node_2, (Element){.type = Types_OPERATION, .argument = { .operation = Operation_ADD}});
        Link(root);
    }

    return root;
}

Node* Calculating_the_n_term(Node* node, int64_t number_member_in_decomposition,
                             int64_t function_point, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    node = Сopy_branch(node, node->parent, pool_allocator);

    for (int64_t i = 0; i < number_member_in_decomposition; i++)
    { 
        node = Dif(node, pool_allocator);

        Calculation(node);
        Simplification(node, pool_allocator);
    }

    int factorial = 1;
    if (number_member_in_decomposition != 1 && number_member_in_decomposition != 0)
    {
        for (int i = 1; i <= number_member_in_decomposition; i++) { factorial *= i; }
    }

    Count_expression(node, (double)function_point, pool_allocator);
    double ratio = node->elem.argument.number / factorial;

    //fprintf(stderr, "%.2lf\n", ratio);

    Tree_dtor(node, pool_allocator);

    Node* root = (Node*)Pool_alloc(pool_allocator);
    Node* left = (Node*)Pool_alloc(pool_allocator);
    Node* right = (Node*)Pool_alloc(pool_allocator);
    Node* right_right = (Node*)Pool_alloc(pool_allocator); 
    Node* right_left = (Node*)Pool_alloc(pool_allocator);

    Link_node_dif(root, NULL, left, right, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_MUL}});
    Link_node_dif(right, root, right_left, right_right, (Element){.type = Types_OPERATION, .argument = {.operation = Operation_SUB}});
    Link_node_dif(right_right, right, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = (double)function_point}});
    Link_node_dif(right_left, right, NULL, NULL, (Element){.type = Types_VARIABLE, .argument = {.variable = 'x'}});
    Link_node_dif(left, root, NULL, NULL, (Element){.type = Types_NUMBER, .argument = {.number = ratio}});

    return root;
}

static Node* Create(Node* parent, Node* left, Node* right, Element elem)
{
    Node* node = (Node*)calloc(1, sizeof(Node));

    node->parent = parent;
    node->left = left;
    node->right = right;

    node->elem = elem;

    return node;
}

static void Link(Node* node)
{
    if ((node->left != NULL) && (node->left->parent == NULL))   { node->left->parent = node; Link(node->left);   }
    if ((node->right != NULL) && (node->right->parent == NULL)) { node->right->parent = node; Link(node->right); }

    return;
}
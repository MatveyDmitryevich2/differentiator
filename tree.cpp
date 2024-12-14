#include "tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

#include "Pool_allocator/pool_allocator.h"
#include "com.h"
#include "utils.h"
#include "dump.h"

static void Parse_line(char** buffer, char* line_buffer);
static Node* Parse_tree(char** buffer, Pool_allocator* pool_allocator);
static Node* New_node(const char* argument, Pool_allocator* pool_allocator);
static Operation Str_to_op(const char* argument);
static void Get_node_arg_from_str(const char* argument, Node* node);
static void Recursive_tree_save(Node* node, FILE* file_ctor);

//-------------------------------------------------global---------------------------------------------------------------
void Dump_node(Node* node)
{
    fprintf(stderr, "------------------------\n");
    fprintf(stderr, "node: %p\n", node);
    if (node != NULL) {
        fprintf(stderr, "node->left: %p\n", node->left);
        fprintf(stderr, "node->right: %p\n", node->right);
        fprintf(stderr, "node->parent: %p\n", node->parent);
        fprintf(stderr, "type: %d\n", node->elem.type);
    }
    fprintf(stderr, "------------------------\n");
}

void Read_file_buffer(const char* name_file, Info_buffer* info_buffer)
{
    assert(name_file != NULL);

    FILE* file_for_save = fopen(name_file, "r");

    info_buffer->size = Get_file_size(file_for_save);
    info_buffer->buffer = (char*)calloc(info_buffer->size + 1, sizeof(char));

    fread(info_buffer->buffer, sizeof(char), info_buffer->size, file_for_save);

    fclose(file_for_save);
}

void Tree_dtor(Node* node, Pool_allocator* pool_allocator)
{
    if (!node) { return; }

    if (node->left)  { Tree_dtor(node->left, pool_allocator);  }
    if (node->right) { Tree_dtor(node->right, pool_allocator); }

    Pool_free(pool_allocator, node);
}

void Error_handling(Error err)
{ 
    switch (err)
    {
        case Error_CLOSE_FILE: { fprintf(stderr, "\nFile opening error\n"); }
        break;
        
        case Error_OPEN_FILE: { fprintf(stderr, "\nFile cloOperation_SINg error\n"); }
        break;

        case Error_NO_ERROR: { fprintf(stderr, "\nThere are no errors\n"); }
        break;

        case Error_MEMORY_ALLOCATION_ERROR: { fprintf(stderr, "\nCan't calloc memory for reading\n"); }
        break;

        case Error_READING_THE_FILE: { fprintf(stderr, "\nError reading the file\n"); }
        break;

        default:
        break;
    }
}

Error Save_tree(Node* node, const char* name_file)
{
    assert(node != NULL);
    assert(name_file != NULL);

    FILE* file_for_storing_tree = fopen(name_file, "r+");
    if ( file_for_storing_tree == NULL) { return Error_OPEN_FILE; }

    Recursive_tree_save(node, file_for_storing_tree);

    if ( fclose(file_for_storing_tree) == EOF) { return Error_CLOSE_FILE; }

    return Error_NO_ERROR;
}

//-----------------------------------------------------static-----------------------------------------------------------

static void Recursive_tree_save(Node* node, FILE* file_ctor)
{
    assert(node != NULL);
    assert(file_ctor != NULL);

    if (!node) { return; }

    fputc(LEFT_BRACKET, file_ctor);

    if (node->left) {
        Recursive_tree_save(node->left, file_ctor);
    }

    switch(node->elem.type)
    {
        case Types_OPERATION:
        {
            fprintf(file_ctor, "%s", Enum_op_to_str(node->elem.argument.operation));
        }
        break;

        case Types_NUMBER:
        {
            fprintf(file_ctor, "%.0lf", node->elem.argument.number);
        }
        break;

        case Types_VARIABLE:
        {
            fprintf(file_ctor, "%c", node->elem.argument.variable);
        }
        break;

        default: { assert(0 && "Unknoun types"); }
    }

    if (node->right) {
        Recursive_tree_save(node->right, file_ctor);
    }

    fputc(RIGHT_BRACKET, file_ctor);
}

// static void Recursive_tree_save(Node* node, FILE* file_ctor)
// {
//     assert(node != NULL);
//     assert(file_ctor != NULL);

//     if (!node) { return; }

//     fputc(LEFT_BRACKET, file_ctor);

//     switch(node->elem.type)
//     {
//         case Types_OPERATION:
//         {
//             fprintf(file_ctor, "%s", Enum_op_to_str(node->elem.argument.operation));
//         } 
//         break;

//         case Types_NUMBER:
//         {
//             fprintf(file_ctor, "%.2lf", node->elem.argument.number);
//         } 
//         break;

//         case Types_VARIABLE:
//         {
//             fprintf(file_ctor, "%c", node->elem.argument.variable);
//         } 
//         break;

//         default: { assert(0); };
//     }

//     if (node->left)  { Recursive_tree_save(node->left, file_ctor); }
//     if (node->right) { Recursive_tree_save(node->right, file_ctor); }
//     fputc(RIGHT_BRACKET, file_ctor);
// }

static Node* New_node(const char* argument, Pool_allocator* pool_allocator)
{ 
    assert(argument != NULL);
    assert(pool_allocator != NULL);

    Node* node = (Node*)Pool_alloc(pool_allocator);

    Get_node_arg_from_str(argument, node);

    return node;
}

static void Get_node_arg_from_str(const char* argument, Node* node)
{
    assert(argument != NULL);
    assert(node != NULL);

    double number = 0;
    if (sscanf(argument, "%lf", &number) != 0)
    {
        node->elem.type = Types_NUMBER;
        node->elem.argument.number = number;
    }
    else
    {
        Operation operation = Str_to_op(argument);

        if (operation == Operation_NO_OPERATION)
        {
            node->elem.type = Types_VARIABLE;
            node->elem.argument.variable = argument[0];
        }

        else
        {
            node->elem.type = Types_OPERATION;
            node->elem.argument.operation = operation;
        }
    }
}

static Operation Str_to_op(const char* argument)
{
    #define IF(OP, ENUM)           if (strncmp(argument, OP, sizeof(OP)) == 0) { return ENUM; }
    #define ELSE_IF(OP, ENUM) else if (strncmp(argument, OP, sizeof(OP)) == 0) { return ENUM; }
    #define ELSE() { return Operation_NO_OPERATION; }

    assert(argument != NULL);

    IF(ADD_, Operation_ADD)
    ELSE_IF(SUB_, Operation_SUB)
    ELSE_IF(MUL_, Operation_MUL)
    ELSE_IF(DIV_, Operation_DIV)
    ELSE_IF(POWER_, Operation_POWER)
    ELSE_IF(SIN_, Operation_SIN)
    ELSE_IF(COS_, Operation_COS)
    ELSE_IF(TG_, Operation_TG)
    ELSE_IF(CTG_, Operation_CTG)
    ELSE_IF(SH_, Operation_SH)
    ELSE_IF(CH_, Operation_CH)
    ELSE_IF(TH_, Operation_TH)
    ELSE_IF(CTH_, Operation_CTH)
    ELSE_IF(ARCSIN_, Operation_ARCSIN)
    ELSE_IF(ARCCOS_, Operation_ARCCOS)
    ELSE_IF(ARCTG_, Operation_ARCTG)
    ELSE_IF(ARCCTG_, Operation_ARCCTG)
    ELSE_IF(LOG_, Operation_LOG)
    ELSE_IF(LN_, Operation_LN)
    ELSE_IF(EXP_, Operation_EXP)
    ELSE()

    #undef IF
    #undef ELSE_IF
    #undef ELSE
}

static Node* Parse_tree(char** buffer, Pool_allocator* pool_allocator)
{
    assert(buffer != NULL);
    assert(pool_allocator != NULL);

    if (**buffer == '\0') { return NULL; }

    Node* right = NULL;
    Node* left = NULL;
    
    if (**buffer == LEFT_BRACKET)
    {
        (*buffer)++;
        left = Parse_tree(buffer, pool_allocator);
        (*buffer)++;
    }

    char line_buffer[MAX_SIZE_LINE] = {};
    Parse_line(buffer, line_buffer);
    Node* node = New_node(line_buffer, pool_allocator);

    node->left = left;
    if (left != NULL) { left->parent = node; }

    if (**buffer == LEFT_BRACKET)
    {
        (*buffer)++;
        right = Parse_tree(buffer, pool_allocator);
        (*buffer)++;
    }

    node->right = right;
    if (right != NULL) { right->parent = node; }

    return node;
}

static void Parse_line(char** buffer, char* line_buffer)
{
    assert(line_buffer != NULL);
    assert(buffer != NULL);

    char* start = *buffer;
    char* end = *buffer;

    while((*end != LEFT_BRACKET) && (*end != RIGHT_BRACKET) && (*end != '\0')) { end++; }

    size_t size_line = (size_t)(end - start);
    strncpy(line_buffer, start, size_line);
    line_buffer[size_line] = '\0';

    *buffer = end;
}

Node* Сopy_branch(Node* node, Node* parent, Pool_allocator* pool_allocator)
{
    assert(node != NULL);

    Node* new_node = (Node*)Pool_alloc(pool_allocator);
    new_node->parent = parent;
    new_node->elem.type = node->elem.type;
    new_node->elem.argument = node->elem.argument;

    if (node->left  != NULL) { 
        new_node->left =  Сopy_branch(node->left,  new_node, pool_allocator); 
        // fprintf(stderr, "new_node->left: %p\n", new_node->left);
    }
    if (node->right != NULL) { 
        new_node->right = Сopy_branch(node->right, new_node, pool_allocator); 
        // fprintf(stderr, "new_node->right: %p\n", new_node->right);
    }

    return new_node;
}


// void Recursive_tree_save(Node* node, FILE* file_ctor, int level)
// {
//     if (!node) { return; }

//     for (int i = 0; i < level; ++i) { fprintf(file_ctor, "    "); }
//     fprintf(file_ctor, "{\n");

//     for (int i = 0; i < level; ++i) { fprintf(file_ctor, "    "); }
//     fprintf(file_ctor, "%s\n", node->elem);

//     if (node->left) { Recursive_tree_save(node->left, file_ctor, level + 1); }
//     if (node->right) { Recursive_tree_save(node->right, file_ctor, level + 1); }

//     for (int i = 0; i < level; ++i) { fprintf(file_ctor, "    "); }
//     fprintf(file_ctor, "}\n");
// }
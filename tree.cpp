#include "tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

#include "dump.h"
#include "stack.h"

enum User_response
{
    User_response_YES   = 1,
    User_response_NO    = 0,
    User_response_EXIT  = 2,
};

enum Array_sizes
{
    Array_sizes_EQUAL         = 0,
    Array_sizes_FIRST_BIGGER  = 1,
    Array_sizes_SECOND_BIGGER = 2,

};

static size_t Reads_file_size(FILE* file);
static void Parsing_line(char** buffer, char* line_buffer);
static Node* Parsing_tree(char** buffer, Node* parent);
static Node* New_node(char* argument, Node* parent_node);
static Operation Str_to_op(char* argument);
static void Writing_argument(char* argument, Node* node);

//-------------------------------------------------global---------------------------------------------------------------

void Tree_dtor(Node* node)
{
    if (!node) { return; }

    if (node->left) { Tree_dtor(node->left); }
    if (node->right) { Tree_dtor(node->right); }

    free(node);
}

Node* Decod_tree()
{
    FILE* file_for_storing_tree = fopen(NAME_FILE_STOR, "r");

    size_t size_buffer = Reads_file_size(file_for_storing_tree);
    char* buffer = (char*)calloc(size_buffer + 1, sizeof(char));
    char* buffer_origin = buffer;

    fgetc(file_for_storing_tree);
    fread(buffer, sizeof(char), size_buffer, file_for_storing_tree);

    Node* root = Parsing_tree(&buffer, NULL);

    free(buffer_origin);
    fclose(file_for_storing_tree);

    return root;
}

//-----------------------------------------------------static-----------------------------------------------------------

static Node* New_node(char* argument, Node* parent_node)
{
    assert(argument != NULL);

    Node* node = (Node*)calloc(1, sizeof(Node));
    node->parent = parent_node;

    Writing_argument(argument, node);

    return node;
}

static void Writing_argument(char* argument, Node* node)
{
    double number = 0;
    if (sscanf(argument, "%lf", &number) != 0)
    {
        node->elem.type = NUMBER;
        node->elem.argument.number = number;
    }

    else
    {
        Operation operation = Str_to_op(argument);

        if (operation == NO_OPERATION)
        {
            node->elem.type = VARIABLE;
            node->elem.argument.variable = argument[0];
        }

        else
        {
            node->elem.type = OPERATION;
            node->elem.argument.operation = operation;
        }
    }
}

static Operation Str_to_op(char* argument)
{
    assert(argument != NULL);

         if (strncmp(argument, _ADD,   sizeof(_ADD))   == 0) { return ADD;          }
    else if (strncmp(argument, _SUB,   sizeof(_SUB))   == 0) { return SUB;          }
    else if (strncmp(argument, _MUL,   sizeof(_MUL))   == 0) { return MUL;          }
    else if (strncmp(argument, _DIV,   sizeof(_DIV))   == 0) { return DIV;          }
    else if (strncmp(argument, _POWER, sizeof(_POWER)) == 0) { return POWER;        }
    else if (strncmp(argument, _SIN,   sizeof(_SIN))   == 0) { return SIN;          }
    else if (strncmp(argument, _COS,   sizeof(_COS))   == 0) { return COS;          }
    else                                                     { return NO_OPERATION; }
}

static Node* Parsing_tree(char** buffer, Node* parent)
{
    assert(buffer != NULL);

    if (**buffer == '\0') { return NULL; }

    char line_buffer[MAX_SIZE_LINE] = {};

    Parsing_line(buffer, line_buffer);
    Node* node = New_node(line_buffer, parent);

    if (**buffer == '{')
    {
        (*buffer)++;
        node->left = Parsing_tree(buffer, node);
        (*buffer)++;
    }
    if (**buffer == '{')
    {
        (*buffer)++;
        node->right = Parsing_tree(buffer, node);
        (*buffer)++;
    }

    return node;
}

static void Parsing_line(char** buffer, char* line_buffer)
{
    assert(buffer != NULL);

    char* start = *buffer;
    char* end = *buffer;

    while((*end != '{') && (*end != '}') && (*end != '\0')) { end++; }//FIXME чиатть через квадратные скобки

    size_t size_line = (size_t)(end - start);
    strncpy(line_buffer, start, size_line);
    line_buffer[size_line] = '\0';

    *buffer = end;
}

static size_t Reads_file_size(FILE *file)
{
    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    size_t size_file = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    return size_file;
}





// void Recursive_tree_entry(Node* node, FILE* file_stor, int level)
// {
//     if (!node) { return; }

//     for (int i = 0; i < level; ++i) { fprintf(file_stor, "    "); }
//     fprintf(file_stor, "{\n");

//     for (int i = 0; i < level; ++i) { fprintf(file_stor, "    "); }
//     fprintf(file_stor, "%s\n", node->elem);

//     if (node->left) { Recursive_tree_entry(node->left, file_stor, level + 1); }
//     if (node->right) { Recursive_tree_entry(node->right, file_stor, level + 1); }

//     for (int i = 0; i < level; ++i) { fprintf(file_stor, "    "); }
//     fprintf(file_stor, "}\n");
// }
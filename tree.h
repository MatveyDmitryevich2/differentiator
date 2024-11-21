#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdbool.h>

#include "stack.h"

typedef char* tree_type;

const char YES[] = "да";
const char NO[] = "нет";
const char EXIT[] = "выход";
const int SIZE_ANSWER = 13;
const char NAME_FILE_STOR[] = "tree.txt"; 
const size_t MAX_SIZE_LINE = 500;
const size_t MAX_TREE_DEPTH = 50;

enum Game_modes
{
    Game_modes_ELEM_SEARCH                     = 1,
    Game_modes_MAKE_DEFENITION_ELEM            = 2,
    Game_modes_FIND_COMMONALITIES_BETWEEN_ELEM = 3,
    Game_modes_EXIT                            = 4,
};

static const char Comandi_ADD_ [] = "add";
static const char Comandi_SUB_ [] = "sub";
static const char Comandi_MUL_ [] = "mul";
static const char Comandi_DIVv_[] = "div";
static const char Comandi_sqrt_[] = "sqrt";
static const char Comandi_SIN_ [] = "sin";
static const char Comandi_COS_ [] = "cos";

enum To_build_definitions_elem
{
    LEFT_YES = 1,
    RIGHT_NO = 0,
    PARENT   = -1,
};

enum Is_found
{
    FOUND     = 1,
    NOT_FOUND = 0,
};

enum Types
{
    VARIABLE  = 1,
    NUMBER    = 2,
    OPERATION = 3,
};

static const char _ADD   [] = "+";
static const char _SUB   [] = "-";
static const char _MUL   [] = "*";
static const char _DIV   [] = "/";
static const char _POWER [] = "^";
static const char _SIN   [] = "sin";
static const char _COS   [] = "cos";
static const char _NO_OPERATION   [] = "I fuck your mom";

enum Operation
{
    NO_OPERATION = 0,
    ADD          = 1,
    SUB          = 2,
    MUL          = 3,
    DIV          = 4,
    POWER        = 5,
    SIN          = 6,
    COS          = 7,
};

struct Node
{
    struct Element
    {
        Types type;

        union Argument
        {
            char variable;
            double number;
            Operation operation;
        } argument;
    } elem;

    Node* left;
    Node* right;
    Node* parent;
};

Node* Decod_tree();
void Tree_dtor(Node* node);

#endif //TREE_H
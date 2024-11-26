#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdbool.h>

static const size_t MAX_SIZE_LINE = 500;
static const int SIZE_ANSWER = 13;
static const char NAME_FILE_STOR[] = "tree_old.txt"; 
static const char NAME_FILE_DTOR[] = "tree_new.txt"; 

enum Types
{
    VARIABLE  = 1,
    NUMBER    = 2,
    OPERATION = 3,
};

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
    TG           = 8,
    CTG          = 9,
    SH           = 10,
    CH           = 11,
    TH           = 12,
    CTH          = 13,
    ARCSIN       = 14,
    ARCCOS      = 15,
    ARCTG       = 16,
    ARCCTG      = 17,
    LOG          = 18,
    LN           = 19,
    EXP          = 20,
};

static const char _VARIABLE  [] = "variable";
static const char _NUMBER    [] = "number";
static const char _OPERATION [] = "operation";

static const char LEFT_BRACKET = '(';
static const char RIGHT_BRACKET = ')';

static const char _ADD          [] = "+";
static const char _SUB          [] = "-";
static const char _MUL          [] = "*";
static const char _DIV          [] = "/";
static const char _POWER        [] = "^";

static const char _SIN          [] = "sin";
static const char _COS          [] = "cos";
static const char _TG           [] = "tg";
static const char _CTG          [] = "ctg";
static const char _SH           [] = "sh";
static const char _CH           [] = "ch";
static const char _TH           [] = "th";
static const char _CTH          [] = "cth";

static const char _ARCSIN       [] = "arcsin";
static const char _ARCCOS       [] = "arccos";
static const char _ARCTG        [] = "arctg";
static const char _ARCCTG       [] = "arcctg";

static const char _LOG          [] = "log";
static const char _LN           [] = "ln";
static const char _EXP          [] = "exp";

static const char _NO_OPERATION [] = "noop";

struct Element
{
    Types type;

    union Argument
    {
        char variable;
        double number;
        Operation operation;
    } argument;
};

struct Node
{
    Element elem;

    Node* left;
    Node* right;
    Node* parent;
};

Node* Decod_tree(const char* name_file);
void Tree_dtor(Node* node);
void Save_tree(Node* node, const char* name_file);

void Calculation(Node* node);
void Simplification(Node* node);

Node* Сopy_branch(Node* node, Node* new_node, Node* parent);

Node* Dif(Node* node);
Node* Add_dif(Node* node);
Node* Sub_dif(Node* node);
Node* Mul_dif(Node* node);
Node* Div_dif(Node* node);
Node* Cos_dif(Node* node);
Node* Sin_dif(Node* node);
Node* Ln_dif(Node* node);
Node* Log_dif(Node* node);
Node* Power_dif(Node* node);
Node* Exp_dif(Node* node);
Node* Tg_dif(Node* node);
Node* Ctg_dif(Node* node);
Node* Sh_dif(Node* node);
Node* Ch_dif(Node* node);
Node* Th_dif(Node* node);
Node* Cth_dif(Node* node);
Node* Arcsin_dif(Node* node);
Node* Arccos_dif(Node* node);
Node* Arctg_dif(Node* node);
Node* Arcctg_dif(Node* node);

//----------------------------------------------------simplification----------------------------------------------------

const size_t MAX_TREE_DEPTH = 50;

enum Is_found
{
    FOUND     = 1,
    NOT_FOUND = 0,
};

#endif //TREE_H
#include "tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

//#include "pool_allocator.h"

//FIXME проверка ошибок
//FIXME доделать упрощалк
//FIXME подключть пул алокатор


#include "dump.h"

static size_t Get_file_size(FILE* file);
static void Pars_line(char** buffer, char* line_buffer);
static Node* Pars_tree(char** buffer);
static Node* New_node(char* argument);
static Operation Str_to_op(char* argument);
static void Get_node_arg_from_str(char* argument, Node* node);
static void Recursive_tree_save(Node* node, FILE* file_ctor);

static void Fold_constants(Node* node);
static void Reduce_redundant_ops(Node* node);
static int Sravnenie(double a, double b);

static Node* Func_number(Node* node);
static Node* Func_variable(Node* node);
static Node* Func_operation(Node* node);
//static Node* Create_node(Node* parent, Element elem, Node* left, Node* right);

//-------------------------------------------------global---------------------------------------------------------------

void Tree_dtor(Node* node)
{
    if (!node) { return; }

    if (node->left) { Tree_dtor(node->left); }
    if (node->right) { Tree_dtor(node->right); }

    free(node);
}

Node* Decod_tree(const char* name_file)
{
    assert(name_file != NULL);

    FILE* file_for_storing_tree = fopen(name_file, "r");
    assert(file_for_storing_tree != NULL); // NOTE проверять нормально а не ассертом

    size_t size_buffer = Get_file_size(file_for_storing_tree);

    char* buffer = (char*)calloc(size_buffer + 1, sizeof(char));
    assert(buffer != NULL); // NOTE аналогично

    char* buffer_move = buffer;

    size_t result = fread(buffer, sizeof(char), size_buffer, file_for_storing_tree);
    fclose(file_for_storing_tree);

    if (result != size_buffer) { assert(0); } // NOTE аналогично

    Node* root = Pars_tree(&buffer_move);

    free(buffer); 

    return root;
}

void Save_tree(Node* node, const char* name_file)
{
    assert(node != NULL);
    assert(name_file != NULL);

    FILE* file_for_storing_tree = fopen(name_file, "r+");
    assert(file_for_storing_tree != NULL); // NOTE аналогично

    Recursive_tree_save(node, file_for_storing_tree);
    fclose(file_for_storing_tree);
}

//-----------------------------------------------------static-----------------------------------------------------------

static void Recursive_tree_save(Node* node, FILE* file_ctor)
{
    assert(node != NULL);
    assert(file_ctor != NULL);

    if (!node) { return; }

    fputc(LEFT_BRACKET, file_ctor);

    switch(node->elem.type)
    {
        case OPERATION:
        {
            fprintf(file_ctor, "%s", Enum_op_to_str(node->elem.argument.operation));
        } 
        break;

        case NUMBER:
        {
            fprintf(file_ctor, "%.2lf", node->elem.argument.number);
        } 
        break;

        case VARIABLE:
        {
            fprintf(file_ctor, "%c", node->elem.argument.variable);
        } 
        break;

        default: { assert(0); };
    }

    if (node->left)  { Recursive_tree_save(node->left, file_ctor); }
    if (node->right) { Recursive_tree_save(node->right, file_ctor); }
    fputc(RIGHT_BRACKET, file_ctor);
}

static Node* New_node(char* argument) // FIXME const char*
{ 
    assert(argument != NULL);

    Node* node = (Node*)calloc(1, sizeof(Node));

    Get_node_arg_from_str(argument, node);

    return node;
}

static void Get_node_arg_from_str(char* argument, Node* node)
{
    assert(argument != NULL);
    assert(node != NULL);

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

         if (strncmp(argument, _ADD,    sizeof(_ADD))    == 0) { return ADD;          }
    else if (strncmp(argument, _SUB,    sizeof(_SUB))    == 0) { return SUB;          }
    else if (strncmp(argument, _MUL,    sizeof(_MUL))    == 0) { return MUL;          }
    else if (strncmp(argument, _DIV,    sizeof(_DIV))    == 0) { return DIV;          }
    else if (strncmp(argument, _POWER,  sizeof(_POWER))  == 0) { return POWER;        }
    else if (strncmp(argument, _SIN,    sizeof(_SIN))    == 0) { return SIN;          }
    else if (strncmp(argument, _COS,    sizeof(_COS))    == 0) { return COS;          }
    else if (strncmp(argument, _TG,     sizeof(_TG))     == 0) { return TG;           }
    else if (strncmp(argument, _CTG,    sizeof(_CTG))    == 0) { return CTG;          }
    else if (strncmp(argument, _SH,     sizeof(_SH))     == 0) { return SH;           }
    else if (strncmp(argument, _CH,     sizeof(_CH))     == 0) { return CH;           }
    else if (strncmp(argument, _TH,     sizeof(_TH))     == 0) { return TH;           }
    else if (strncmp(argument, _CTH,    sizeof(_CTH))    == 0) { return CTH;          }
    else if (strncmp(argument, _ARCSIN, sizeof(_ARCSIN)) == 0) { return ARCSIN;       }
    else if (strncmp(argument, _ARCCOS, sizeof(_ARCCOS)) == 0) { return ARCCOS;       }
    else if (strncmp(argument, _ARCTG,  sizeof(_ARCTG))  == 0) { return ARCTG;        }
    else if (strncmp(argument, _ARCCTG, sizeof(_ARCCTG)) == 0) { return ARCCTG;       }
    else if (strncmp(argument, _LOG,    sizeof(_LOG))    == 0) { return LOG;          }
    else if (strncmp(argument, _LN,     sizeof(_LN))     == 0) { return LN;           }
    else if (strncmp(argument, _EXP,    sizeof(_EXP))    == 0) { return EXP;          }
    else                                                       { return NO_OPERATION; }
}

static Node* Pars_tree(char** buffer)
{
    assert(buffer != NULL);

    if (**buffer == '\0') { return NULL; }

    Node* right = NULL;
    Node* left = NULL;
    
    if (**buffer == LEFT_BRACKET)
    {
        (*buffer)++;
        left = Pars_tree(buffer);
        (*buffer)++;
    }

    char line_buffer[MAX_SIZE_LINE] = {};
    Pars_line(buffer, line_buffer);
    Node* node = New_node(line_buffer);

    node->left = left;
    if (left != NULL) { left->parent = node; }

    if (**buffer == LEFT_BRACKET)
    {
        (*buffer)++;
        right = Pars_tree(buffer);
        (*buffer)++;
    }

    node->right = right;
    if (right != NULL) { right->parent = node; }

    return node;
}

static void Pars_line(char** buffer, char* line_buffer)
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

static size_t Get_file_size(FILE *file)
{
    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    size_t size_file = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    return size_file;
}

//----------------------------------------------------simplification----------------------------------------------------

void Calculation(Node* node)
{
    assert(node != NULL);

    if ((node->elem.type == NUMBER) || (node->elem.type == VARIABLE)) { return; }

    fprintf(stderr, "\n%p\n", node);
    fprintf(stderr, "\n%p\n", node->left);

    if ((node->left->elem.type != NUMBER) 
        && (node->left->elem.type != VARIABLE)) { Calculation(node->left); }
    if ((node->right->elem.type != NUMBER)
        && (node->right->elem.type != VARIABLE)) { Calculation(node->right); }

    if ((node->left->elem.type == NUMBER) && (node->right->elem.type == NUMBER)) { Fold_constants(node); }

    return;
}

static void Fold_constants(Node* node)
{
    assert(node != NULL);

    #define CASE_NOT_USE(op) case op: {} break;

    #define CASE(op, op_sign) case op:                                                                           \
    {                                                                                                            \
        node->elem.type = NUMBER;                                                                                \
        node->elem.argument.number = node->left->elem.argument.number op_sign node->right->elem.argument.number; \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    #define CASE_POWER(op) case op:                                                                              \
    {                                                                                                            \
        node->elem.type = NUMBER;                                                                                \
        node->elem.argument.number = pow(node->left->elem.argument.number, node->right->elem.argument.number);   \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    #define CASE_ANOTHER(op, op_sign) case op:                                                                           \
    {                                                                                                            \
        node->elem.type = NUMBER;                                                                                \
        node->elem.argument.number = op_sign(node->left->elem.argument.number);                                  \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      \
    }                                                                                                            \
    break;

    switch(node->elem.argument.operation)
    {
        CASE(ADD, +);
        CASE(SUB, -);
        CASE(MUL, *);
        CASE(DIV, /);
        CASE_ANOTHER(SIN, sin);
        CASE_ANOTHER(COS, cos);
        CASE_ANOTHER(TG, tan);
        CASE_ANOTHER(SH, sinh);
        CASE_ANOTHER(CH, cosh);
        CASE_ANOTHER(ARCSIN, asin);
        CASE_ANOTHER(ARCCOS, acos);
        CASE_ANOTHER(ARCTG, atan);
        CASE_ANOTHER(LN, log);
        CASE_ANOTHER(EXP, exp);
        CASE_NOT_USE(CTH);
        CASE_NOT_USE(TH);
        CASE_NOT_USE(CTG);
        CASE_NOT_USE(LOG);
        CASE_NOT_USE(ARCCTG);
        CASE_NOT_USE(NO_OPERATION);
        CASE_POWER(POWER);
        default: { assert(0); };
    }

    #undef CASE
    #undef CASE_NOT_USE
    #undef CASE_POWER
    #undef CASE_ANOTHER
}

void Simplification(Node* node)
{
    assert(node != NULL);

    if ((node->elem.type == NUMBER) || (node->elem.type == VARIABLE)) { return; }

    if ((node->left->elem.type != NUMBER) 
        && (node->left->elem.type != VARIABLE)) { Simplification(node->left); }
    if ((node->right->elem.type != NUMBER)
        && (node->right->elem.type != VARIABLE)) { Simplification(node->right); }

    if ((node->left->elem.type == VARIABLE) && (node->right->elem.type == NUMBER)) { Reduce_redundant_ops(node); }
    else if ((node->left->elem.type == NUMBER) && (node->right->elem.type == VARIABLE)) { Reduce_redundant_ops(node); }

    return;
}

static void Reduce_redundant_ops(Node* node)
{
    assert(node != NULL);

    #define SEX(op_enum, op, right_hand_value)                                                                                    \
        node->elem.type = op_enum;                                                                               \
        node->elem.argument.op = right_hand_value;                                                               \
        free(node->left);                                                                                        \
        free(node->right);                                                                                       \
        node->left = NULL;                                                                                       \
        node->right = NULL;                                                                                      

    switch(node->elem.argument.operation)
    {
        case ADD: 
        {
            if ((node->left->elem.type == VARIABLE) && Sravnenie(node->right->elem.argument.number, 0))
                { SEX(VARIABLE, variable, node->left->elem.argument.variable); }

            else if ((node->right->elem.type == VARIABLE) && Sravnenie(node->left->elem.argument.number, 0))
                { SEX(VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case MUL:
        {
            if ((node->left->elem.type == VARIABLE) && Sravnenie(node->right->elem.argument.number, 0))
                { SEX(NUMBER, number, 0); }

            else if ((node->right->elem.type == VARIABLE) && Sravnenie(node->left->elem.argument.number, 0))
                { SEX(NUMBER, number, 0); }

            else if ((node->left->elem.type == VARIABLE) && Sravnenie(node->right->elem.argument.number, 1))
                { SEX(VARIABLE, variable, node->left->elem.argument.variable); }

            else if ((node->right->elem.type == VARIABLE) && Sravnenie(node->left->elem.argument.number, 1))
                { SEX(VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case SUB:
        {
            if ((node->left->elem.type == VARIABLE) && Sravnenie(node->right->elem.argument.number, 0))
                { SEX(VARIABLE, variable, node->left->elem.argument.variable); }

            else if ((node->right->elem.type == VARIABLE) && Sravnenie(node->left->elem.argument.number, 0))
                { SEX(VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case POWER:
        {
            if ((node->left->elem.type == VARIABLE) && Sravnenie(node->right->elem.argument.number, 0))
                { SEX(NUMBER, number, 1); }

            else if ((node->right->elem.type == VARIABLE) && Sravnenie(node->left->elem.argument.number, 0))
                { SEX(NUMBER, number, 1); }

            else if ((node->left->elem.type == VARIABLE) && Sravnenie(node->right->elem.argument.number, 1))
                { SEX(VARIABLE, variable, node->left->elem.argument.variable); }

            else if ((node->right->elem.type == VARIABLE) && Sravnenie(node->left->elem.argument.number, 1))
                { SEX(VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case DIV:
        {
            if ((node->left->elem.type == VARIABLE) && Sravnenie(node->right->elem.argument.number, 1))
            { SEX(VARIABLE, variable, node->left->elem.argument.variable); }

            else if ((node->right->elem.type == VARIABLE) && Sravnenie(node->left->elem.argument.number, 1))
            { SEX(VARIABLE, variable, node->right->elem.argument.variable); }
        } 
        break;

        case COS: {} break;
        case SIN: {} break;
        case TG: {} break;
        case CTG: {} break;
        case SH: {} break;
        case CH: {} break;
        case ARCCTG: {} break;
        case ARCTG: {} break;
        case ARCSIN: {} break;
        case ARCCOS: {} break;
        case LN: {} break;
        case EXP: {} break;
        case LOG: {} break;
        case TH: {} break;
        case CTH: {} break;
        case NO_OPERATION: {} break;
        default: { assert(0); };
    }

    #undef SEX
}

static int Sravnenie(double a, double b)
{
    int resultat = 0;
    if (fabs(a - b) < 0.0001) {resultat = 1; }

    return resultat;
}

//--------------------------------------------------------dif-----------------------------------------------------------

Node* Сopy_branch(Node* node, Node* new_node, Node* parent)
{
    assert(node != NULL);

    new_node = (Node*)calloc(1, sizeof(Node));
    new_node->parent = parent;
    new_node->elem.type = node->elem.type;
    new_node->elem.argument = node->elem.argument;

    if(node->left != NULL) { new_node->left = Сopy_branch(node->left, new_node->left, new_node); }
    if(node->right != NULL) { new_node->right = Сopy_branch(node->right, new_node->right, new_node); }

    return new_node;
}

Node* Dif(Node* node)
{
    assert(node != NULL);

    if (node->elem.type == NUMBER)    { return Func_number(node);    }
    if (node->elem.type == VARIABLE)  { return Func_variable(node);  }
    if (node->elem.type == OPERATION) { return Func_operation(node); }

    return node;
}

static Node* Func_number(Node* node)
{
    assert(node != NULL);

    node->elem.argument.number = 0;

    return node;
}

static Node* Func_variable(Node* node)
{
    assert(node != NULL);

    node->elem.type = NUMBER;
    node->elem.argument.number = 1;

    return node;
}

static Node* Func_operation(Node* node)
{
    assert(node != NULL);

    switch (node->elem.argument.operation)
    {
        case ADD:    { return Add_dif(node);        }
        case SUB:    { return Sub_dif(node);        }
        case MUL:    { return Mul_dif(node);        }
        case DIV:    { return Div_dif(node);        }
        case COS:    { return Cos_dif(node);        }
        case SIN:    { return Sin_dif(node);        }
        case TG:     { return Tg_dif(node);         }
        case CTG:    { return Ctg_dif(node);        }
        case SH:     { return Sh_dif(node);         }
        case CH:     { return Ch_dif(node);         }
        case TH:     { return Th_dif(node);         }
        case CTH:    { return Cth_dif(node);        }
        case ARCSIN: { return Arcsin_dif(node);     }
        case ARCCOS: { return Arccos_dif(node);     }
        case ARCTG:  { return Arctg_dif(node);      }
        case ARCCTG: { return Arcctg_dif(node);     }
        case LOG:    { return Log_dif(node);        }
        case LN:     { return Ln_dif(node);         }
        case EXP:    { return Exp_dif(node);        } 
        case POWER:  { return Power_dif(node);      }
        case NO_OPERATION: {return Power_dif(node); }
        default: { assert(0); };
    }

    return node;
}
//(Element){.type = VARIABLE, .argument.variable = 'x'}
//Create_node(Node* parent, Element elem, Node* left, Node* right)

Node* Arcctg_dif(Node* node)
{
    assert(node != NULL);

    Node* r = (Node*)calloc(1, sizeof(Node));
    Node* r_l = (Node*)calloc(1, sizeof(Node));
    Node* r_r_r = (Node*)calloc(1, sizeof(Node));
    Node* r_r = (Node*)calloc(1, sizeof(Node));
    Node* l = (Node*)calloc(1, sizeof(Node));
    Node* l_l = (Node*)calloc(1, sizeof(Node));

    Node* r_r_l = Сopy_branch(node->left, NULL, r_r);
    
    Node* l_r = Сopy_branch(node->left, NULL, l);
    l_r = Dif(l_r);

    l_l->parent = l;
    l_l->elem.type = NUMBER;
    l_l->elem.argument.number = -1;

    l->parent = node;
    l->elem.type = OPERATION;
    l->elem.argument.operation = MUL;
    l->left = l_l;
    l->right = l_r;

    r->parent = node;
    r->elem.type = OPERATION;
    r->elem.argument.operation = ADD;
    r->left = r_l;
    r->right = r_r;

    r_r->parent = r;
    r_r->elem.type = OPERATION;
    r_r->elem.argument.operation = POWER;
    r_r->left = r_r_l;
    r_r->right = r_r_r;

    r_l->parent = r;
    r_l->elem.type = NUMBER;
    r_l->elem.argument.number = 1;

    r_r_r->parent = r_r;
    r_r_r->elem.type = NUMBER;
    r_r_r->elem.argument.number = 2;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Arctg_dif(Node* node)
{
    assert(node != NULL);

    Node* r = (Node*)calloc(1, sizeof(Node));
    Node* r_l = (Node*)calloc(1, sizeof(Node));
    Node* r_r_r = (Node*)calloc(1, sizeof(Node));
    Node* r_r = (Node*)calloc(1, sizeof(Node));

    Node* r_r_l = Сopy_branch(node->left, NULL, r_r);
    
    Node* l = Сopy_branch(node->left, NULL, node);
    l = Dif(l);

    r->parent = node;
    r->elem.type = OPERATION;
    r->elem.argument.operation = ADD;
    r->left = r_l;
    r->right = r_r;

    r_r->parent = r;
    r_r->elem.type = OPERATION;
    r_r->elem.argument.operation = POWER;
    r_r->left = r_r_l;
    r_r->right = r_r_r;

    r_l->parent = r;
    r_l->elem.type = NUMBER;
    r_l->elem.argument.number = 1;

    r_r_r->parent = r_r;
    r_r_r->elem.type = NUMBER;
    r_r_r->elem.argument.number = 2;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Arccos_dif(Node* node)
{
    assert(node != NULL);

    Node* r = (Node*)calloc(1, sizeof(Node));
    Node* r_r = (Node*)calloc(1, sizeof(Node));
    Node* r_l = (Node*)calloc(1, sizeof(Node));
    Node* r_l_r = (Node*)calloc(1, sizeof(Node));
    Node* r_l_l = (Node*)calloc(1, sizeof(Node));
    Node* r_l_r_r = (Node*)calloc(1, sizeof(Node));
    Node* l = (Node*)calloc(1, sizeof(Node));
    Node* l_l = (Node*)calloc(1, sizeof(Node));

    Node* r_l_r_l = Сopy_branch(node->left, NULL, r_l_r);
    
    Node* l_r = Сopy_branch(node->left, NULL, l);
    l_r = Dif(l_r);

    l_l->parent = l;
    l_l->elem.type = NUMBER;
    l_l->elem.argument.number = -1;

    l->parent = node;
    l->elem.type = OPERATION;
    l->elem.argument.operation = MUL;
    l->left = l_l;
    l->right = l_r;

    r_l_r_r->parent = r_l_r;
    r_l_r_r->elem.type = NUMBER;
    r_l_r_r->elem.argument.number = 2;

    r_l_r->parent = r_l;
    r_l_r->elem.type = OPERATION;
    r_l_r->elem.argument.operation = POWER;
    r_l_r->left = r_l_r_l;
    r_l_r->right = r_l_r_r;

    r_l_l->parent = r_l;
    r_l_l->elem.type = NUMBER;
    r_l_l->elem.argument.number = 1;

    r_l->parent = r;
    r_l->elem.type = OPERATION;
    r_l->elem.argument.operation = SUB;
    r_l->left = r_l_l;
    r_l->right = r_l_r;

    r_r->parent = r;
    r_r->elem.type = NUMBER;
    r_r->elem.argument.number = 0.5;

    r->parent = node;
    r->elem.type = OPERATION;
    r->elem.argument.operation = POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Arcsin_dif(Node* node)
{
    assert(node != NULL);

    Node* r = (Node*)calloc(1, sizeof(Node));
    Node* r_r = (Node*)calloc(1, sizeof(Node));
    Node* r_l = (Node*)calloc(1, sizeof(Node));
    Node* r_l_r = (Node*)calloc(1, sizeof(Node));
    Node* r_l_l = (Node*)calloc(1, sizeof(Node));
    Node* r_l_r_r = (Node*)calloc(1, sizeof(Node));

    Node* r_l_r_l = Сopy_branch(node->left, NULL, r_l_r);
    
    Node* l = Сopy_branch(node->left, NULL, node);
    l = Dif(l);

    r_l_r_r->parent = r_l_r;
    r_l_r_r->elem.type = NUMBER;
    r_l_r_r->elem.argument.number = 2;

    r_l_r->parent = r_l;
    r_l_r->elem.type = OPERATION;
    r_l_r->elem.argument.operation = POWER;
    r_l_r->left = r_l_r_l;
    r_l_r->right = r_l_r_r;

    r_l_l->parent = r_l;
    r_l_l->elem.type = NUMBER;
    r_l_l->elem.argument.number = 1;

    r_l->parent = r;
    r_l->elem.type = OPERATION;
    r_l->elem.argument.operation = SUB;
    r_l->left = r_l_l;
    r_l->right = r_l_r;

    r_r->parent = r;
    r_r->elem.type = NUMBER;
    r_r->elem.argument.number = 0.5;

    r->parent = node;
    r->elem.type = OPERATION;
    r->elem.argument.operation = POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Cth_dif(Node* node)
{
    assert(node != NULL);

    Node* r = (Node*)calloc(1, sizeof(Node));
    Node* r_r = (Node*)calloc(1, sizeof(Node));
    Node* r_l = (Node*)calloc(1, sizeof(Node));
    Node* r_l_r = (Node*)calloc(1, sizeof(Node));

    Node* l = Сopy_branch(node->left, NULL, node);
    l = Dif(l);

    Node* r_l_l = Сopy_branch(node->left, NULL, r_l);

    r_l_r->parent = r_l;
    r_l_r->elem.type = NUMBER;
    r_l_r->elem.argument.number = 0;

    r_l->parent = r;
    r_l->elem.type = OPERATION;
    r_l->elem.argument.operation = CH;
    r_l->left = r_l_l;
    r_l->right = r_l_r;

    r_r->parent = r;
    r_r->elem.type = NUMBER;
    r_r->elem.argument.number = 2;

    r->parent = node;
    r->elem.type = OPERATION;
    r->elem.argument.operation = POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Th_dif(Node* node)
{
    assert(node != NULL);

    Node* r = (Node*)calloc(1, sizeof(Node));
    Node* r_r = (Node*)calloc(1, sizeof(Node));
    Node* r_l = (Node*)calloc(1, sizeof(Node));
    Node* r_l_r = (Node*)calloc(1, sizeof(Node));

    Node* l = Сopy_branch(node->left, NULL, node);
    l = Dif(l);

    Node* r_l_l = Сopy_branch(node->left, NULL, r_l);

    r_l_r->parent = r_l;
    r_l_r->elem.type = NUMBER;
    r_l_r->elem.argument.number = 0;

    r_l->parent = r;
    r_l->elem.type = OPERATION;
    r_l->elem.argument.operation = CH;
    r_l->left = r_l_l;
    r_l->right = r_l_r;

    r_r->parent = r;
    r_r->elem.type = NUMBER;
    r_r->elem.argument.number = 2;

    r->parent = node;
    r->elem.type = OPERATION;
    r->elem.argument.operation = POWER;
    r->left = r_l;
    r->right = r_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Ch_dif(Node* node)
{
    assert(node != NULL);

    Node* r = Сopy_branch(node->left, NULL, node);
    r = Dif(r);

    Node* l = (Node*)calloc(1, sizeof(Node));
    Node* l_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l = (Node*)calloc(1, sizeof(Node));
    Node* l_l_l = (Node*)calloc(1, sizeof(Node));
    Node* l_l_l_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r_l = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r_l_r = (Node*)calloc(1, sizeof(Node));

    Node* l_l_l_l = Сopy_branch(node->left, NULL, l_l_l);
    Node* l_l_r_l_l = Сopy_branch(node->left, NULL, l_l_r_l);

    l_l_r_l_r->parent = l_l_r_l;
    l_l_r_l_r->elem.type = NUMBER;
    l_l_r_l_r->elem.argument.number = -1;

    l_l_r_l->parent = l_l_r;
    l_l_r_l->elem.type = OPERATION;
    l_l_r_l->elem.argument.operation = POWER;
    l_l_r_l->left = l_l_r_l_l;
    l_l_r_l->right = l_l_r_l_r;

    l_l_r_r->parent = l_l_r;
    l_l_r_r->elem.type = NUMBER;
    l_l_r_r->elem.argument.number = 0;

    l_l_r->parent = l_l;
    l_l_r->elem.type = OPERATION;
    l_l_r->elem.argument.operation = EXP;
    l_l_r->left = l_l_r_l;
    l_l_r->right = l_l_r_r;

    l_l_l_r->parent = l_l_l;
    l_l_l_r->elem.type = NUMBER;
    l_l_l_r->elem.argument.number = 0;

    l_l_l->parent = l_l;
    l_l_l->elem.type = OPERATION;
    l_l_l->elem.argument.operation = EXP;
    l_l_l->left = l_l_l_l;
    l_l_l->right = l_l_l_r;

    l_l->parent = l;
    l_l->elem.type = OPERATION;
    l_l->elem.argument.operation = ADD;
    l_l->left = l_l_l;
    l_l->right = l_l_r;

    l_r->parent = l;
    l_r->elem.type = NUMBER;
    l_r->elem.argument.number = 2;

    l->parent = node;
    l->elem.type = OPERATION;
    l->elem.argument.operation = DIV;
    l->left = l_l;
    l->right = l_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = MUL;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Sh_dif(Node* node)
{
    assert(node != NULL);

    Node* r = Сopy_branch(node->left, NULL, node);
    r = Dif(r);

    Node* l = (Node*)calloc(1, sizeof(Node));
    Node* l_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l = (Node*)calloc(1, sizeof(Node));
    Node* l_l_l = (Node*)calloc(1, sizeof(Node));
    Node* l_l_l_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r_r = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r_l = (Node*)calloc(1, sizeof(Node));
    Node* l_l_r_l_r = (Node*)calloc(1, sizeof(Node));

    Node* l_l_l_l = Сopy_branch(node->left, NULL, l_l_l);
    Node* l_l_r_l_l = Сopy_branch(node->left, NULL, l_l_r_l);

    l_l_r_l_r->parent = l_l_r_l;
    l_l_r_l_r->elem.type = NUMBER;
    l_l_r_l_r->elem.argument.number = -1;

    l_l_r_l->parent = l_l_r;
    l_l_r_l->elem.type = OPERATION;
    l_l_r_l->elem.argument.operation = POWER;
    l_l_r_l->left = l_l_r_l_l;
    l_l_r_l->right = l_l_r_l_r;

    l_l_r_r->parent = l_l_r;
    l_l_r_r->elem.type = NUMBER;
    l_l_r_r->elem.argument.number = 0;

    l_l_r->parent = l_l;
    l_l_r->elem.type = OPERATION;
    l_l_r->elem.argument.operation = EXP;
    l_l_r->left = l_l_r_l;
    l_l_r->right = l_l_r_r;

    l_l_l_r->parent = l_l_l;
    l_l_l_r->elem.type = NUMBER;
    l_l_l_r->elem.argument.number = 0;

    l_l_l->parent = l_l;
    l_l_l->elem.type = OPERATION;
    l_l_l->elem.argument.operation = EXP;
    l_l_l->left = l_l_l_l;
    l_l_l->right = l_l_l_r;

    l_l->parent = l;
    l_l->elem.type = OPERATION;
    l_l->elem.argument.operation = ADD;
    l_l->left = l_l_l;
    l_l->right = l_l_r;

    l_r->parent = l;
    l_r->elem.type = NUMBER;
    l_r->elem.argument.number = 2;

    l->parent = node;
    l->elem.type = OPERATION;
    l->elem.argument.operation = DIV;
    l->left = l_l;
    l->right = l_r;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = MUL;
    node->left = l;
    node->right = r;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Ctg_dif(Node* node)
{
    assert(node != NULL);

    Node* right_right = (Node*)calloc(1, sizeof(Node));
    Node* right = (Node*)calloc(1, sizeof(Node));

    right_right->parent = right;
    right_right->elem.type = NUMBER;
    right_right->elem.argument.number = 2;

    Node* right_left = (Node*)calloc(1, sizeof(Node));
    Node* right_left_left = Сopy_branch(node->left, NULL, right_left);

    Node* right_left_right = (Node*)calloc(1, sizeof(Node));
    right_left_right->elem.type = NUMBER;
    right_left_right->elem.argument.number = 0;
    right_left_right->parent = right_left;

    right_left->parent = right;
    right_left->elem.type = OPERATION;
    right_left->elem.argument.operation = COS;
    right_left->left = right_left_left;
    right_left->right = right_left_right;

    right->parent = node;
    right->elem.type = OPERATION;
    right->elem.argument.operation = POWER;
    right->left = right_left;
    right->right = right_right;

    Node* left = (Node*)calloc(1, sizeof(Node));
    left->parent = node;
    left->elem.type = OPERATION;
    left->elem.argument.operation = MUL;

    Node* left_right = (Node*)calloc(1, sizeof(Node));
    left_right->elem.type = NUMBER;
    left_right->elem.argument.number = -1;
    left_right->parent = left;


    Node* left_left = Сopy_branch(node->left, NULL, left);
    left_left = Dif(left_left);

    left->left = left_left;
    left->right = left_right;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Tg_dif(Node* node)
{
    assert(node != NULL);

    Node* right_right = (Node*)calloc(1, sizeof(Node));
    Node* right = (Node*)calloc(1, sizeof(Node));

    right_right->parent = right;
    right_right->elem.type = NUMBER;
    right_right->elem.argument.number = 2;

    Node* right_left = (Node*)calloc(1, sizeof(Node));
    Node* right_left_left = Сopy_branch(node->left, NULL, right_left);

    Node* right_left_right = (Node*)calloc(1, sizeof(Node));
    right_left_right->elem.type = NUMBER;
    right_left_right->elem.argument.number = 0;
    right_left_right->parent = right_left;

    right_left->parent = right;
    right_left->elem.type = OPERATION;
    right_left->elem.argument.operation = COS;
    right_left->left = right_left_left;
    right_left->right = right_left_right;

    right->parent = node;
    right->elem.type = OPERATION;
    right->elem.argument.operation = POWER;
    right->left = right_left;
    right->right = right_right;

    Node* left = Сopy_branch(node->left, NULL, node);
    left = Dif(left);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Exp_dif(Node* node)
{
    assert(node != NULL);
    
    Node* right = Сopy_branch(node->left, NULL, node);

    Node* left = Сopy_branch(node, NULL, node);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = MUL;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Power_dif(Node* node)
{
    assert(node != NULL);

    Node* right = Сopy_branch(node, NULL, node);

    Node* left = (Node*)calloc(1, sizeof(Node));
    Node* left_left = (Node*)calloc(1, sizeof(Node));

    Node* left_left_left = Сopy_branch(node->left, NULL, left_left);

    Node* left_left_right = (Node*)calloc(1, sizeof(Node));
    left_left_right->elem.type = NUMBER;
    left_left_right->elem.argument.number = 0;
    left_left_right->parent = left_left;

    Node* left_right = Сopy_branch(node->right, NULL, left);

    left->parent = node;
    left->left = left_left;
    left->right = left_right;
    left->elem.type = OPERATION;
    left->elem.argument.operation = MUL;

    left_left->parent = left;
    left_left->left = left_left_left;
    left_left->right = left_left_right;
    left_left->elem.type = OPERATION;
    left_left->elem.argument.operation = LN;

    left->parent = node;
    left->left = left_left;
    left->right = left_right;
    left->elem.type = OPERATION;
    left->elem.argument.operation = MUL;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = MUL;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Log_dif(Node* node)
{
    assert(node != NULL);

    Node* left = (Node*)calloc(1, sizeof(Node));
    Node* right = (Node*)calloc(1, sizeof(Node));
    Node* left_right = (Node*)calloc(1, sizeof(Node));
    Node* right_right = (Node*)calloc(1, sizeof(Node));

    Node* left_left = Сopy_branch(node->right, NULL, left);
    Node* right_left = Сopy_branch(node->left, NULL, right);

    left->parent = node;
    left->left = left_left;
    left->right = left_right;
    left->elem.type = OPERATION;
    left->elem.argument.operation = LN;

    right->parent = node;
    right->left = right_left;
    right->right = right_right;
    right->elem.type = OPERATION;
    right->elem.argument.operation = LN;

    left_right->parent = left;
    left_right->elem.type = NUMBER;
    left_right->elem.argument.number = 0;
    right_right->parent = right;
    right_right->elem.type = NUMBER;
    right_right->elem.argument.number = 0;

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = DIV;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    node = Dif(node);

    return node;
}

Node* Ln_dif(Node* node)
{

    assert(node != NULL);

    node->elem.argument.operation = DIV;

    Node* left = Сopy_branch(node->left, NULL, node);
    left = Dif(left);

    Node* right = Сopy_branch(node->left, NULL, node);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Add_dif(Node* node)
{
    assert(node != NULL);

    Node* left = Сopy_branch(node->left, NULL, node);
    left = Dif(left);

    Node* right = Сopy_branch(node->right, NULL, node);
    right = Dif(right);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);
    
    return node;
}


Node* Sub_dif(Node* node)
{
    assert(node != NULL);

    Node* left = Сopy_branch(node->left, NULL, node);
    left = Dif(left);

    Node* right = Сopy_branch(node->right, NULL, node);
    right = Dif(right);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);
    
    return node;
}

Node* Mul_dif(Node* node)
{
    assert(node != NULL);

    Node* new_left = (Node*)calloc(1, sizeof(Node));
    Node* new_right = (Node*)calloc(1, sizeof(Node));

    new_left->parent = node;
    new_right->parent = node;

    new_left->elem.type = OPERATION;
    new_right->elem.type = OPERATION;
    new_left->elem.argument.operation = MUL;
    new_right->elem.argument.operation = MUL;

    Node* node_left_left = Сopy_branch(node->right, NULL , new_left);
    Node* node_right_right = Сopy_branch(node->left, NULL , new_right);
    Node* node_left_right = Сopy_branch(node->right, NULL , new_left);
    Node* node_right_left = Сopy_branch(node->left, NULL, new_right);

    node_left_left = Dif(node_left_left);
    node_right_right = Dif(node_right_right);

    new_left->left = node_left_left;
    new_right->right = node_right_right;

    node->elem.argument.operation = ADD;

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

Node* Div_dif(Node* node)
{
    assert(node != NULL);

    Node* r_right = (Node*)calloc(1, sizeof(Node));
    Node* r_right_right = (Node*)calloc(1, sizeof(Node));

    Node* r_right_left = Сopy_branch(node->right, NULL , r_right);

    r_right->parent = node;
    r_right->elem.type = OPERATION;
    r_right->elem.argument.operation = POWER;
    r_right->left = r_right_left;
    r_right->right = r_right_right;

    r_right_right->parent = r_right;
    r_right_right->elem.type = NUMBER;
    r_right_right->elem.argument.number = 2;


    Node* left = (Node*)calloc(1, sizeof(Node));
    Node* left_left = (Node*)calloc(1, sizeof(Node));
    Node* left_right = (Node*)calloc(1, sizeof(Node));

    Node* left_left_right = Сopy_branch(node->right, NULL , left_left);
    Node* left_right_left = Сopy_branch(node->left, NULL , left_right);

    Node* left_left_left = Сopy_branch(node->left, NULL , left_left);
    left_left_left = Dif(left_left_left);

    Node* left_right_right = Сopy_branch(node->right, NULL , left_right);
    left_right_right = Dif(left_right_right);

    left_left->parent = left;
    left_left->elem.type = OPERATION;
    left_left->elem.argument.operation = MUL;
    left_left->left = left_left_left;
    left_left->right = left_left_right;

    left_right->parent = left;
    left_right->elem.type = OPERATION;
    left_right->elem.argument.operation = MUL;
    left_right->left = left_right_left;
    left_right->right = left_right_right;

    left->parent = node;
    left->elem.type = OPERATION;
    left->elem.argument.operation = SUB;
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

Node* Cos_dif(Node* node)
{
    assert(node != NULL);

    Node* left = (Node*)calloc(1, sizeof(Node));
    Node* left_right = (Node*)calloc(1, sizeof(Node));

    Node* left_left = Сopy_branch(node->left, NULL , left);

    left_right->parent = left;
    left_right->elem.type = NUMBER;
    left_right->elem.argument.number = 0;

    left->parent = node;
    left->elem.type = OPERATION;
    left->elem.argument.operation = SIN;
    left->left = left_left;
    left->right = left_right;

    Node* right = Сopy_branch(node->left, NULL, node);
    right = Dif(right);

    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = MUL;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

Node* Sin_dif(Node* node)
{
    assert(node != NULL);

    //(Element){.type = VARIABLE, .argument.variable = 'x'}

    Node* right = (Node*)calloc(1, sizeof(Node));
    Node* left = (Node*)calloc(1, sizeof(Node));
    Node* right_right = (Node*)calloc(1, sizeof(Node));
    Node* left_right = (Node*)calloc(1, sizeof(Node));

    Node* left_left = Сopy_branch(node->left, NULL , left);
    
    left_right->parent = left;
    left_right->elem.type = NUMBER;
    left_right->elem.argument.number = 0;

    left->parent = node;
    left->elem.type = OPERATION;
    left->elem.argument.operation = COS;
    left->left = left_left;
    left->right = left_right;

    Node* right_left = Сopy_branch(node->left, NULL , right);
    right_left = Dif(right_left);

    right_right->parent = right;
    right_right->elem.type = NUMBER;
    right_right->elem.argument.number = -1;

    right->parent = node;
    right->elem.type = OPERATION;
    right->elem.argument.operation = MUL;
    right->left = right_left;
    right->right = right_right;


    Node* left_dtor = node->left;
    Node* right_dtor = node->right;

    node->elem.argument.operation = MUL;
    node->left = left;
    node->right = right;

    Tree_dtor(left_dtor);
    Tree_dtor(right_dtor);

    return node;
}

// static Node* Create_node(Node* parent, Element elem, Node* left, Node* right)
// {
//     Node* node = (Node*)calloc(1, sizeof(Node));

//     node->parent = parent;
//     node->elem = elem;
//     node->left = left;
//     node->right = right;

//     return node;
// }




























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
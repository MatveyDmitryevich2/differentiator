#ifndef LEXICAL_ANALYSIS_H
#define LEXICAL_ANALYSIS_H

#include <stdio.h>
#include <stddef.h>

#include "../utils.h"
#include "../com.h"

enum Type_expression
{
    Type_expression_VARIABLE      = 1,
    Type_expression_NUMBER        = 2,
    Type_expression_OPERATION     = 3,
    Type_expression_LEFT_BRACKET  = 4,
    Type_expression_RIGHT_BRACKET = 5,
    Type_expression_END_SYMBOL    = 6,
};

struct Identifiers
{
    size_t index_first_symbol;
    size_t index_last_symbol;
    size_t size_in_char;
    
    Type_expression type;

    union Argument
    {
        char variable;
        double number;
        Operation operation;
    } argument;
};

Identifiers* Lexical_analysis(Info_buffer* info_buffer);

#endif //LEXICAL_ANALYSIS_H
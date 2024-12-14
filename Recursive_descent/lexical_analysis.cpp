#include "lexical_analysis.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <cctype>
#include <iostream>
#include <cstring>

#include "../utils.h"
#include "../com.h"

static void Process_bracket(size_t* iterating_through_array, size_t* i, Identifiers* identifiers, Type_expression side_bracket);
static void Process_number(size_t* iterating_through_array, size_t* i,
                           Identifiers* identifiers, Info_buffer* info_buffer);
static void Process_operation(size_t* iterating_through_array, size_t* i,
                              Identifiers* identifiers, Info_buffer* info_buffer);
static void Process_end_symbol(size_t* iterating_through_array, size_t* i, Identifiers* identifiers);

static Operation Str_to_op(const char* argument);


Identifiers* Lexical_analysis(Info_buffer* info_buffer)
{
    assert(info_buffer != NULL);

    Identifiers* identifiers = (Identifiers*)calloc(MAX_QUANTITY_IDENTIFIERS, sizeof(Identifiers));

    for (size_t i = 0, iterating_through_array = 0; i < info_buffer->size; )
    {
        while (isspace(info_buffer->buffer[i])) { i++; }

        if (info_buffer->buffer[i] == LEFT_BRACKET)
        {
            DPRINTF("LEFT_BRACKET_: %c\n", info_buffer->buffer[i]);
            Process_bracket(&iterating_through_array, &i, identifiers + iterating_through_array, Type_expression_LEFT_BRACKET);
        }

        else if (info_buffer->buffer[i] == RIGHT_BRACKET)
        {
            DPRINTF("RIGHT_BRACKET_: %c\n", info_buffer->buffer[i]);
            Process_bracket(&iterating_through_array, &i, identifiers + iterating_through_array, Type_expression_RIGHT_BRACKET);
        }

        else if (info_buffer->buffer[i] == END)
        {
            DPRINTF("END: %c\n", info_buffer->buffer[i]);
            Process_end_symbol(&iterating_through_array, &i, identifiers + iterating_through_array);
        }

        else if (isdigit(info_buffer->buffer[i]))
        {
            DPRINTF("NUMBER: %c\n", info_buffer->buffer[i]);
            Process_number(&iterating_through_array, &i, identifiers + iterating_through_array, info_buffer);
        }

        else
        {
            DPRINTF("WORD: %c\n", info_buffer->buffer[i]);
            Process_operation(&iterating_through_array, &i, identifiers + iterating_through_array, info_buffer);
        }
    }

    return identifiers;
}

static void Process_end_symbol(size_t* iterating_through_array, size_t* i, Identifiers* identifiers)
{
    assert(iterating_through_array != NULL);
    assert(i != NULL);
    assert(identifiers != NULL);
               
    identifiers->index_first_symbol = *i;    
    identifiers->index_last_symbol = *i;     
    identifiers->size_in_char = 1;           
    identifiers->type = Type_expression_END_SYMBOL;                
                                             
    (*iterating_through_array)++;              
    (*i)++;
}

static void Process_operation(size_t* iterating_through_array, size_t* i,
                              Identifiers* identifiers, Info_buffer* info_buffer)
{
    assert(iterating_through_array != NULL);
    assert(i != NULL);
    assert(identifiers != NULL);
    assert(info_buffer != NULL);

    identifiers->index_first_symbol = *i;

    if (Is_operator(info_buffer->buffer[*i]) == true)
    {
        identifiers->index_last_symbol = *i;
        identifiers->size_in_char = 1;

        char* word_buffer = (char*)calloc(identifiers->size_in_char + 1, sizeof(char));
        strncpy(word_buffer, info_buffer->buffer + identifiers->index_first_symbol, identifiers->size_in_char);

        Operation operation = Str_to_op(word_buffer);
        identifiers->type = Type_expression_OPERATION;
        identifiers->argument.operation = operation;

        // fprintf(stderr, "_______ index_first_symbol = %lu_______\n", identifiers->index_first_symbol);
        // fprintf(stderr, "_______ index_last_symbol = %lu_______\n", identifiers->index_last_symbol);
        // fprintf(stderr, "_______ size_in_char = %lu_______\n", identifiers->size_in_char);
        // fprintf(stderr, "_______ type = %d_______\n", identifiers->type);
        // fprintf(stderr, "_______ operation = %d_______\n", identifiers->argument.operation);

        free(word_buffer);
        (*i)++;
    }

    else
    {
        while ((isalnum(info_buffer->buffer[*i])) || (info_buffer->buffer[*i] == UNDERSCORE)) { (*i)++; }

        identifiers->index_last_symbol = *i - 1;
        identifiers->size_in_char = *i - identifiers->index_first_symbol;

        char* word_buffer = (char*)calloc(identifiers->size_in_char + 1, sizeof(char));
        strncpy(word_buffer, info_buffer->buffer + identifiers->index_first_symbol, identifiers->size_in_char);

        Operation operation = Str_to_op(word_buffer);

        if (operation == Operation_NO_OPERATION)
        {
            identifiers->type = Type_expression_VARIABLE;
            identifiers->argument.variable = word_buffer[0];
        }

        else
        {
            identifiers->type = Type_expression_OPERATION;
            identifiers->argument.operation = operation;
        }

        free(word_buffer);
    }

    (*iterating_through_array)++;
}

static void Process_number(size_t* iterating_through_array, size_t* i,
                           Identifiers* identifiers, Info_buffer* info_buffer)
{
    assert(iterating_through_array != NULL);
    assert(i != NULL);
    assert(identifiers != NULL);
    assert(info_buffer != NULL);

    identifiers->index_first_symbol = *i;
    identifiers->type = Type_expression_NUMBER;

    while ((info_buffer->buffer[*i] == DOT) || (isdigit(info_buffer->buffer[*i]))) { (*i)++; }

    identifiers->index_last_symbol = *i - 1;
    identifiers->size_in_char = *i - identifiers->index_first_symbol;

    char* number_buffer = (char*)calloc(identifiers->size_in_char + 1, sizeof(char));
    strncpy(number_buffer, info_buffer->buffer + identifiers->index_first_symbol, identifiers->size_in_char);

    identifiers->argument.number = strtod(number_buffer, NULL);

    (*iterating_through_array)++;
    
    free(number_buffer);
}

static void Process_bracket(size_t* iterating_through_array, size_t* i, Identifiers* identifiers, Type_expression side_bracket)
{
    assert(iterating_through_array != NULL);
    assert(i != NULL);
    assert(identifiers != NULL);
               
    identifiers->index_first_symbol = *i;    
    identifiers->index_last_symbol = *i;     
    identifiers->size_in_char = 1;           
    identifiers->type = side_bracket;                
                                             
    (*iterating_through_array)++;              
    (*i)++;
}

static Operation Str_to_op(const char* argument)
{
    #define IF(OP, ENUM) if (strncmp(argument, OP, sizeof(OP)) == 0) { return ENUM; }
    #define ELSE_IF(OP, ENUM) else if (strncmp(argument, OP, sizeof(OP)) == 0) { return ENUM; }
    #define ELSE() { return Operation_NO_OPERATION; }

    assert(argument != NULL);

         IF(ADD_,    Operation_ADD)
    ELSE_IF(SUB_,    Operation_SUB)
    ELSE_IF(MUL_,    Operation_MUL)
    ELSE_IF(DIV_,    Operation_DIV)
    ELSE_IF(POWER_,  Operation_POWER)
    ELSE_IF(SIN_,    Operation_SIN)
    ELSE_IF(COS_,    Operation_COS)
    ELSE_IF(TG_,     Operation_TG)
    ELSE_IF(CTG_,    Operation_CTG)
    ELSE_IF(SH_,     Operation_SH)
    ELSE_IF(CH_,     Operation_CH)
    ELSE_IF(TH_,     Operation_TH)
    ELSE_IF(CTH_,    Operation_CTH)
    ELSE_IF(ARCSIN_, Operation_ARCSIN)
    ELSE_IF(ARCCOS_, Operation_ARCCOS)
    ELSE_IF(ARCTG_,  Operation_ARCTG)
    ELSE_IF(ARCCTG_, Operation_ARCCTG)
    ELSE_IF(LOG_,    Operation_LOG)
    ELSE_IF(LN_,     Operation_LN)
    ELSE_IF(EXP_,    Operation_EXP)
    ELSE()

    #undef IF
    #undef ELSE_IF
    #undef ELSE
}
#ifndef RBNF_H
#define RBNF_H

#include <stdio.h>
#include <stddef.h>
#include "../com.h"

#include "lexical_analysis.h"
#include "../com.h"

Node* GetG(Identifiers* identifiers);
Node* Create_node(Node* parent, Node* left, Node* right, Element elem);

#endif //RBNF_H
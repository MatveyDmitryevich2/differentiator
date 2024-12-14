#ifndef DIF_FUNCTION_H
#define DIF_FUNCTION_H

#include "com.h"
#include "Pool_allocator/pool_allocator.h"

Node* Dif(Node* node, Pool_allocator* pool_allocator);
void Calculation(Node* node);
void Simplification(Node* node, Pool_allocator* pool_allocator);

#endif // DIF_FUNCTION_H
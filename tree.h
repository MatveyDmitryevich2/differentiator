#ifndef TREE_H
#define TREE_H

#include <stdio.h>
#include <stdbool.h>

#include "Pool_allocator/pool_allocator.h"
#include "com.h"

void Tree_dtor(Node* node, Pool_allocator* pool_allocator);
Error Save_tree(Node* node, const char* name_file);
void Error_handling(Error err);
void Read_file_buffer(const char* name_file, Info_buffer* info_buffer);
void Dump_node(Node* node);

Node* Сopy_branch(Node* node, Node* parent, Pool_allocator* pool_allocator);

#endif //TREE_H
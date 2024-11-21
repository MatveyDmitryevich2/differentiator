#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "tree.h"
#include "dump.h"

int main()
{
    Write_html_mode(Dump_html_BEGIN);

    Node* root = Decod_tree();

    Dump(root); 
    Write_html_mode(Dump_html_END);

    Tree_dtor(root);

    return 0;
}
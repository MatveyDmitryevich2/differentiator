#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "tree.h"
#include "dump.h"

int main()
{
    Write_html_mode(Dump_html_BEGIN);

    Node* root = Decod_tree(NAME_FILE_STOR);
    root = Dif(root);
    Calculation(root);
    Simplification(root);

    Dump(root); 
    Write_html_mode(Dump_html_END);

    Save_tree(root, NAME_FILE_DTOR);
    Tree_dtor(root); 

    return 0;
}
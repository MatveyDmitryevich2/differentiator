#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "Pool_allocator/pool_allocator.h"
#include "tree.h"
#include "dump.h"
#include "dif_function.h"

int main() // FIXME принимать argc argv чтоб передавать файлы 
{
    Error err = Error_NO_ERROR;

    Pool_allocator* pool_allocator = Pool_allocator_ctor(sizeof(Node));
    err = Write_html_mode(Dump_html_BEGIN);
    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE; }
    Node* root = Decod_tree(NAME_FILE_STOR, pool_allocator, &err);  
    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE; }

    Dump_LaTex(root, NAME_FILE_LATEX, pool_allocator);

    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE;}
    err = Write_html_mode(Dump_html_END);
    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE;}
    err = Save_tree(root, NAME_FILE_DTOR);
    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE;}
    Tree_dtor(root, pool_allocator);
    Pool_allocator_dtor(pool_allocator);

    return EXIT_SUCCESS;
}
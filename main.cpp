#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "com.h"
#include "Pool_allocator/pool_allocator.h"
#include "tree.h"
#include "dump.h"
#include "dif_function.h"
#include "Recursive_descent/RBNF.h"

int main()
{
    Error err = Error_NO_ERROR;

    Pool_allocator* pool_allocator = Pool_allocator_ctor(sizeof(Node));
    err = Write_html_mode(Dump_html_BEGIN);
    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE; }

    Info_buffer info_buffer = {};
    Read_file_buffer(NAME_FILE_CTOR, &info_buffer);

    Identifiers* array_identifiers = Lexical_analysis(&info_buffer);

    Node* node = GetG(array_identifiers);

    Dump_LaTex(node, NAME_FILE_LATEX, pool_allocator);

    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE;}
    err = Write_html_mode(Dump_html_END);
    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE;}
    err = Save_tree(node, NAME_FILE_DTOR);
    if (err != Error_NO_ERROR) { Error_handling(err); return EXIT_FAILURE;}

    free(array_identifiers);
    Tree_dtor(node, pool_allocator);
    Pool_allocator_dtor(pool_allocator);

    return EXIT_SUCCESS;
}
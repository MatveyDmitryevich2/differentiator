#ifndef DUMP_FILE_H
#define DUMP_FILE_H

#include <stdio.h>

#include "tree.h"

enum Dump_html
{
    Dump_html_BEGIN = 0,
    Dump_html_END   = 1,
};

const char INFO_GRAPH[] = "digraph structs {\n charset = \"UTF-8\";\n rankdir=TB;\n bgcolor = \"#FFFFFF\";\n"
                          "fontcolor = black;\n fontsize = 18;\n style = \"rounded\";\n margin = 0.3;\n splines = ortho"
                          ";\nranksep = 1.0;\n nodesep = 0.9;\n edge [color=\"#A600A6\", style=solid, penwidth=2];\n";
const char INFO_HTML[] = "<!DOCTYPE html>\n<html lang=\"en\">\n\n<head>\n<meta charset=\"utf-8\">\n"
                       "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                       "<title>list dump</title>\n<link rel=\"stylesheet\" href=\"styles.css\">\n</head>\n\n";


void Dump(Node* root);
void Write_html_mode(Dump_html mode);
const char* Enum_op_to_str(Operation operation);

#endif //DUMP_FILE_H
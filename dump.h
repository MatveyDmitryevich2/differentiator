#ifndef DUMP_FILE_H
#define DUMP_FILE_H

#include <stdio.h>

#include "com.h"
#include "Pool_allocator/pool_allocator.h"

enum Dump_html
{
    Dump_html_BEGIN = 0,
    Dump_html_END   = 1,
};

enum Dump_LaTex
{
    Dump_LaTex_BEGIN = 0,
    Dump_LaTex_END   = 1,
};

const char INFO_GRAPH[] = "digraph structs\n{\n    charset = \"UTF-8\";\n    rankdir=TB;\n    bgcolor = \"#FFFFFF\";\n"
                          "    fontcolor = black;\n    fontsize = 18;\n    style = \"rounded\";\n    margin = 0.3;\n    splines = ortho"
                          ";\n    ranksep = 1.0;\n    nodesep = 0.9;\n    edge [color=\"#A600A6\", style=solid, penwidth=2];\n";
const char INFO_HTML[] = "<!DOCTYPE html>\n<html lang=\"en\">\n\n<head>\n<meta charset=\"utf-8\">\n"
                       "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                       "<title>list dump</title>\n<link rel=\"stylesheet\" href=\"styles.css\">\n</head>\n\n";

const char INFO_BEGIN_TEX[] = "\\documentclass[a4paper,12pt]{article}\n\\usepackage[T1]{fontenc}\n"
                            "\\usepackage[utf8]{inputenc}\n\\usepackage[english,russian]{babel}\n"
                            "\\usepackage{amsmath,amsfonts,amssymb,amsthm,mathtools}\n"
                            "\\usepackage[left=10mm, top=10mm, right=10mm, bottom=20mm, nohead, nofoot]{geometry}"
                            "\n\\geometry{landscape}\n"
                            "\n\\usepackage{graphicx}\n"
                            "\n\\begin{document}\n\n\n";

const char INFO_END_TEX[] = "\n\n\n\\end{document}";

Error Dump_grapviz(Node* root);
Error Write_html_mode(Dump_html mode);
const char* Enum_op_to_str(Operation operation);
Error Dump_LaTex(Node* node, const char* file, Pool_allocator* pool_allocator);

#endif //DUMP_FILE_H
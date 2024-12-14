#include "dump.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include <sys/time.h>
#include <unistd.h>

#include "utils.h"
#include "tree_utils.h"
#include "com.h"
#include "dif_function.h"

static Error Write_before_body();
static Error Write_body();
static Error Write_html();

static void Generate_nodes(Node* node, FILE* file);

static void Operation_output(Node* node, FILE* file_tex);
static void Recursive_entry_formula(Node* node, FILE* file_tex);

//--------------------------------------------------global--------------------------------------------------------------

Error Write_html_mode(Dump_html mode)
{
    Error err = Error_NO_ERROR;

    if (mode == Dump_html_BEGIN) 
    { 
        err = Write_before_body(); 
        err = Write_body(); 
    }
    
    if (mode == Dump_html_END)   
    { 
        err = Write_body(); 
        err = Write_html(); 
    }

    return err;
}

Error Dump_grapviz(Node* root) 
{
    assert(root != NULL);

    Error err = Error_NO_ERROR;

    struct timeval tv;
    gettimeofday(&tv, NULL);

    long seconds = tv.tv_sec;
    long microseconds = tv.tv_usec;
    
    char filename[100] = {0};
    snprintf(filename, sizeof(filename), "file_%ld_%06ld.dot", seconds, microseconds);

    FILE* file_html = fopen("dump.html", "a+");
    if (file_html == NULL) { return Error_OPEN_FILE; }

    FILE* file_dump = fopen(filename, "a+");
    if (file_dump == NULL) { return Error_OPEN_FILE; }

    fprintf(file_dump, INFO_GRAPH);

    Generate_nodes(root, file_dump);

    fprintf(file_dump, "}\n");

    if (fclose(file_dump) == EOF) { return Error_CLOSE_FILE; }

    char command[256];
    snprintf(command, sizeof(command), "dot -Tpng %s -o %.22s.png", filename, filename);

    // fprintf(stderr, "pid: %d\n", getpid());

    system(command);
    // if (errrr < 0) {
    //     perror("bebera");
    //     fprintf(stderr, "what a heck\n");
    // }
    // fprintf(stderr, "pid: %d\n", getpid());

    snprintf(filename, sizeof(filename), "file_%ld_%06ld.png", seconds, microseconds);
    fprintf(file_html, "<img src=\"%s\"/>\n", filename);
    if (fclose(file_html) == EOF) { return Error_CLOSE_FILE; }

    return err;
}

//--------------------------------------------------static--------------------------------------------------------------

static void Generate_nodes(Node* node, FILE* file)
{
    DPRINTF("\nАДРЕС НОДЫ ДЛЯ ПЕЧАТИ: %p\n", node);

    if (node == NULL) { return; }

    char node_id[100];
    snprintf(node_id, sizeof(node_id), "node_%lx", (long unsigned int)node);

    // fprintf(stderr, "node: %p\n", node);
    // fprintf(stderr, "node->left: %p\n", node->left);
    // fprintf(stderr, "node->right: %p\n", node->right);

    const char* node_color = "white";
    
    switch(node->elem.type)
    {
        case Types_VARIABLE:  { node_color = "blue";  } break;
        case Types_NUMBER:    { node_color = "green"; } break;
        case Types_OPERATION: { node_color = "red";   } break;
        default: { assert(0 && "No tipes"); }
    }

    fprintf(file, "    %s\n    [\n", node_id);
    fprintf(file, "        shape=plaintext,\n");
    fprintf(file, "        label=<\n");
    fprintf(file, "        <table border=\"0\" cellpadding=\"4\" cellborder=\"1\" width=\"300\" bgcolor=\"%s\">\n", node_color);
    fprintf(file, "        <tr><td align=\"center\" colspan=\"2\">Parent: %p</td></tr>\n", (void*)node->parent);

    if (node->elem.type == Types_OPERATION) {
        fprintf(file, "        <tr><td align=\"center\" colspan=\"2\">%s</td></tr>\n", Types_OPERATION_);
        fprintf(file, "        <tr><td align=\"center\" colspan=\"2\">%s</td></tr>\n", Enum_op_to_str(node->elem.argument.operation));
    } else 

    if (node->elem.type == Types_NUMBER) {
        fprintf(file, "        <tr><td align=\"center\" colspan=\"2\">%s</td></tr>\n", Types_NUMBER_);
        fprintf(file, "        <tr><td align=\"center\" colspan=\"2\">%.lf</td></tr>\n", node->elem.argument.number);
    } else 
    
    if (node->elem.type == Types_VARIABLE) {
        fprintf(file, "        <tr><td align=\"center\" colspan=\"2\">%s</td></tr>\n", Types_VARIABLE_);
        fprintf(file, "        <tr><td align=\"center\" colspan=\"2\">%c</td></tr>\n", node->elem.argument.variable);
    } else {
        assert(0 && "unknown type");
    }

    fprintf(file, "        <tr>\n");
    fprintf(file, "        <td align=\"center\" width=\"150\">Left: %p</td>\n", (void*)node->left);
    fprintf(file, "        <td align=\"center\" width=\"150\">Right: %p</td>\n", (void*)node->right);
    fprintf(file, "        </tr>\n");

    fprintf(file, "        </table>\n");
    fprintf(file, "        >\n");
    fprintf(file, "    ];\n");

    if (node->left != NULL)
    {
        char left_id[100];
        snprintf(left_id, sizeof(left_id), "node_%lx", (long unsigned int)node->left);
        fprintf(file, "    %s -> %s [color=\"#000000\"];\n", node_id, left_id);

        DPRINTF("\nПЕЧАТАЕМ ЛЕВОЕ ПОДДЕРЕВО\n",);
        Generate_nodes(node->left, file);
    }

    if (node->right != NULL)
    {
        char right_id[100];
        snprintf(right_id, sizeof(right_id), "node_%lx", (long unsigned int)node->right);
        fprintf(file, "    %s -> %s [color=\"#000000\"];\n", node_id, right_id);

        DPRINTF("\nПЕЧАТАЕМ ПРАВОЕ ПОДДЕРЕВО\n",);
        Generate_nodes(node->right, file);
    }
}

const char* Enum_op_to_str(Operation operation)
{
    #define CASE(enum, op) case enum: { return op; }

    switch (operation)
    {
        CASE(Operation_ADD,          ADD_               );
        CASE(Operation_SUB,          SUB_               );
        CASE(Operation_MUL,          MUL_               );
        CASE(Operation_DIV,          DIV_               ); 
        CASE(Operation_POWER,        POWER_             );
        CASE(Operation_SIN,          SIN_               ); 
        CASE(Operation_COS,          COS_               );
        CASE(Operation_TG,           TG_                );
        CASE(Operation_CTG,          CTG_               );
        CASE(Operation_SH,           SH_                );
        CASE(Operation_CH,           CH_                );
        CASE(Operation_TH,           TH_                );
        CASE(Operation_CTH,          CTH_               );
        CASE(Operation_ARCSIN,       ARCSIN_            );
        CASE(Operation_ARCCOS,       ARCCOS_            );
        CASE(Operation_ARCTG,        ARCTG_             );
        CASE(Operation_ARCCTG,       ARCCTG_            );
        CASE(Operation_LOG,          LOG_               );
        CASE(Operation_LN,           LN_                );
        CASE(Operation_EXP,          EXP_               );
        CASE(Operation_NO_OPERATION, NO_Types_OPERATION_);
        
        default: { return NULL; };
    }

    #undef CASE
}

static Error Write_before_body()
{
    FILE* html_file = fopen("dump.html", "a+");
    if (html_file == NULL) { return Error_OPEN_FILE; }

    fprintf(html_file, INFO_HTML);
    fclose(html_file);

    return Error_NO_ERROR;
}

static Error Write_body()
{
    FILE* html_file = fopen("dump.html", "a+");
    if (html_file == NULL) { return Error_OPEN_FILE; }

    fprintf(html_file, "<body>\n");

    fclose(html_file);

    return Error_NO_ERROR;
}

static Error Write_html()
{
    FILE* html_file = fopen("dump.html", "a+");
    if (html_file == NULL) { return Error_OPEN_FILE; }

    fprintf(html_file, "</html>\n");
    
    fclose(html_file);

    return Error_NO_ERROR;
}

//---------------------------------------------------dumpTEX------------------------------------------------------------

Error Dump_LaTex(Node* node, const char* filename, Pool_allocator* pool_allocator)
{
    assert(node != NULL);
    assert(filename != NULL);

    FILE* file_tex = fopen(filename, "r+");
    if (file_tex == NULL) { return Error_OPEN_FILE; }
    fprintf(file_tex, "\n");
    fprintf(file_tex, INFO_BEGIN_TEX);

    fprintf(file_tex, "\\begin{center}\n");
    fprintf(file_tex, "\\section*{Возможный ход вычисления производных:}\n");
    fprintf(file_tex, "\\end{center}\n\n");

    fprintf(file_tex, "Поступившая формула до дифференцирвоания:\\newline\n");
    fprintf(file_tex, "\\[");
    Dump_grapviz(node);
    fprintf(file_tex, "f(x) = ");
    Recursive_entry_formula(node, file_tex);
    fprintf(file_tex, "\\]\\newline\n\\newline\n\n\n");

    node = Dif(node, pool_allocator); // FIXME нахуй отсюад
    fprintf(file_tex, "Первая производная:\\newline\n");
    fprintf(file_tex, "\\[");
    Dump_grapviz(node);
    fprintf(file_tex, "f'(x) = ");
    Recursive_entry_formula(node, file_tex);
    fprintf(file_tex, "\\]\\newline\n\\newline\n\n\n");

    Calculation(node); // FIXME нахуй отсюад
    Simplification(node, pool_allocator); // FIXME нахуй отсюад
    fprintf(file_tex, "Формула первой производной после упрощения и свертывания констант:\\newline\n");
    fprintf(file_tex, "\\[");
    Dump_grapviz(node);
    fprintf(file_tex, "f'(x) = ");
    Recursive_entry_formula(node, file_tex);
    fprintf(file_tex, "\\]\\newline\n\\newline\n\n\n");

    node = Dif(node, pool_allocator); // FIXME нахуй отсюад
    fprintf(file_tex, "Вторая производная:\\newline\n");
    fprintf(file_tex, "\\[");
    Dump_grapviz(node);
    fprintf(file_tex, "f''(x) = ");
    Recursive_entry_formula(node, file_tex);
    fprintf(file_tex, "\\]\\newline\n\\newline\n\n\n");

    Calculation(node); // FIXME нахуй отсюад
    Simplification(node, pool_allocator); // FIXME нахуй отсюад
    fprintf(file_tex, "Формула второй производной после упрощения и свертывания констант:\\newline\n");
    fprintf(file_tex, "\\[");
    fprintf(file_tex, "f''(x) = ");
    Recursive_entry_formula(node, file_tex);
    fprintf(file_tex, "\\]\\newline\n\\newline\n\n\n");

    fprintf(file_tex, INFO_END_TEX);
    fclose(file_tex);
    return Error_NO_ERROR;
}

static void Recursive_entry_formula(Node* node, FILE* file_tex)
{
    assert(file_tex != NULL);

    if (node == NULL) { return; }

    switch(node->elem.type)
    {
        case Types_VARIABLE:
        {
            fprintf(file_tex, "%c", node->elem.argument.variable);
            return;
        }
        break;

        case Types_NUMBER:
        {
            fprintf(file_tex, "%.lf", node->elem.argument.number);
            return;
        }
        break;

        case Types_OPERATION:
        {
            Operation_output(node, file_tex);
        }
        break;

        default: { assert(0 && "No this types"); };
    }
}

static void Operation_output(Node* node, FILE* file_tex)
{
    assert(node != NULL);
    assert(file_tex != NULL);

    #define CASE_ONE_ARG(enum, str)                                                                              \
    case enum:                                                                                                   \
    {                                                                                                            \
        fprintf(file_tex, "\\");                                                                                 \
        fprintf(file_tex, str);                                                                                  \
        fprintf(file_tex, "{");                                                                                  \
        if (Is_operation(node->right) || Is_operation(node->left)) { fprintf(file_tex, "("); }                   \
        if (node->right != NULL) { Recursive_entry_formula(node->right, file_tex); }                             \
        if (node->left != NULL) { Recursive_entry_formula(node->left, file_tex); }                               \
        if (Is_operation(node->right) || Is_operation(node->left)) { fprintf(file_tex, ")"); }                   \
        fprintf(file_tex, "}");                                                                                  \
    }                                                                                                            \
    break;

    #define CASE_INF(enum, str)                                                                                  \
    case enum:                                                                                                   \
    {                                                                                                            \
        fprintf(file_tex, "{");                                                                                  \
        if (Is_operation(node->right) || Is_operation(node->left)) { fprintf(file_tex, "("); }                   \
        Recursive_entry_formula(node->left, file_tex);                                                           \
        if (Is_operation(node->right) || Is_operation(node->left)) { fprintf(file_tex, ")"); }                   \
        fprintf(file_tex, "}");                                                                                  \
        fprintf(file_tex, str);                                                                                  \
        fprintf(file_tex, "{");                                                                                  \
        Recursive_entry_formula(node->right, file_tex);                                                          \
        fprintf(file_tex, "}");                                                                                  \
    }                                                                                                            \
    break;

    #define CASE_PREF(enum, str)                                                                                 \
    case enum:                                                                                                   \
    {                                                                                                            \
        fprintf(file_tex, str);                                                                                  \
        fprintf(file_tex, "{");                                                                                  \
        Recursive_entry_formula(node->left, file_tex);                                                           \
        fprintf(file_tex, "}");                                                                                  \
        fprintf(file_tex, "{");                                                                                  \
        Recursive_entry_formula(node->right, file_tex);                                                          \
        fprintf(file_tex, "}");                                                                                  \
    }                                                                                                            \
    break;

    #define CASE_ADD_SUB(enum, str)                                                                              \
    case enum:                                                                                                   \
    {                                                                                                            \
        if (Is_op_mul(node->parent)) { fprintf(file_tex, "("); }                                                 \
        fprintf(file_tex, "{");                                                                                  \
        Recursive_entry_formula(node->left, file_tex);                                                           \
        fprintf(file_tex, "}");                                                                                  \
        fprintf(file_tex, str);                                                                                  \
        fprintf(file_tex, "{");                                                                                  \
        Recursive_entry_formula(node->right, file_tex);                                                          \
        fprintf(file_tex, "}");                                                                                  \
        if (Is_op_mul(node->parent)) { fprintf(file_tex, ")"); }                                                 \
    }                                                                                                            \
    break;

    #define CASE_MUL(enum, str)                                                                                  \
    case enum:                                                                                                   \
    {                                                                                                            \
        fprintf(file_tex, "{");                                                                                  \
        Recursive_entry_formula(node->left, file_tex);                                                           \
        fprintf(file_tex, "}");                                                                                  \
        fprintf(file_tex, str);                                                                                  \
        fprintf(file_tex, "{");                                                                                  \
        Recursive_entry_formula(node->right, file_tex);                                                          \
        fprintf(file_tex, "}");                                                                                  \
    }                                                                                                            \
    break;

    #define CASE_EXP                                                                                             \
    case Operation_EXP:                                                                                          \
    {                                                                                                            \
        fprintf(file_tex, "{");                                                                                  \
        fprintf(file_tex, "e");                                                                                  \
        fprintf(file_tex, "}");                                                                                  \
        fprintf(file_tex, "^");                                                                                  \
        fprintf(file_tex, "{");                                                                                  \
        if (node->right != NULL) { Recursive_entry_formula(node->right, file_tex); }                             \
        if (node->left != NULL) { Recursive_entry_formula(node->left, file_tex); }                               \
        fprintf(file_tex, "}");                                                                                  \
    }                                                                                                            \
    break;

    switch(node->elem.argument.operation)
    {
        CASE_EXP;
        CASE_ADD_SUB(Operation_ADD,    "+"      );
        CASE_ADD_SUB(Operation_SUB,    "-"      );
        CASE_MUL    (Operation_MUL,    "\\cdot" );
        CASE_INF    (Operation_POWER,  "^"      );
        CASE_PREF   (Operation_DIV,    "\\frac" );
        CASE_PREF   (Operation_LOG,    "\\log_" );
        CASE_ONE_ARG(Operation_LN,     "ln"     );
        CASE_ONE_ARG(Operation_SIN,    "sin"    );
        CASE_ONE_ARG(Operation_COS,    "cos"    );
        CASE_ONE_ARG(Operation_TG,     "tg"     );
        CASE_ONE_ARG(Operation_CTG,    "ctg"    );
        CASE_ONE_ARG(Operation_SH,     "sh"     );
        CASE_ONE_ARG(Operation_CH,     "ch"     );
        CASE_ONE_ARG(Operation_TH,     "th"     );
        CASE_ONE_ARG(Operation_CTH,    "cth"    );
        CASE_ONE_ARG(Operation_ARCSIN, "arcsin" );
        CASE_ONE_ARG(Operation_ARCCOS, "arccos" );
        CASE_ONE_ARG(Operation_ARCTG,  "arctg"  );
        CASE_ONE_ARG(Operation_ARCCTG, "arcctg" );

        case Operation_NO_OPERATION: {} break;
        default: { assert(0 && "No operation"); };
    }

    #undef CASE_ONE_ARG
    #undef CASE_INF
    #undef CASE_EXP
    #undef CASE_ADD_SUB
    #undef CASE_MUL
}

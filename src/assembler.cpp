#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <assembler.h>
#include <cmd.h>
#include <spuio.h>

void runAsm(FILE* fin, FILE* fout)
{
    #define ASM_CASE(command)               \
        if (strcmp(str_cmd, #command) == 0) \
        {                                   \
            code[ip++] = CMD_ ## command;   \
            continue;                       \
        }
    #define ASM_CASE_ARG(command)           \
        if (strcmp(str_cmd, #command) == 0) \
        {                                   \
            code[ip++] = CMD_ ## command;   \
            int arg = 0;                    \
            fscanf(fin, "%d", &arg);        \
            code[ip++] = arg;               \
            continue;                       \
        }
    #define ASM_CASE_LABEL_ARG(command)               \
        if (strcmp(str_cmd, #command) == 0)           \
        {                                             \
            code[ip++] = CMD_ ## command;             \
            fscanf(fin, "%s", label);                 \
            if (label[strlen(label) - 1] == ':')      \
            {                                         \
                int adr = -1;                         \
                getLabelAdr(&la, label, &adr);        \
                code[ip++] = adr;                     \
                if (adr == -1)                        \
                {                                     \
                    addFixup(&ft, ip - 1, label);     \
                }                                     \
            }                                         \
            else                                      \
            {                                         \
                code[ip++] = strtol(label, NULL, 10); \
            }                                         \
        }

    assert(fin != NULL && fout != NULL);

    char str_cmd[50] = "", label[20] = "";
    int code[100] = {};
    int ip = 0;
    LabelArray la = {};
    initLabelArray(&la);
    FixupTable ft = {};
    initFixupTable(&ft);
    
    while (1)
    {
        if (fscanf(fin, "%s", str_cmd) == EOF)
            break;

        if (str_cmd[strlen(str_cmd) - 1] == ':')
        {
            insertLabel(&la, ip, str_cmd);
        }
        
        ASM_CASE(HLT)
        ASM_CASE(IN)
        ASM_CASE(OUT)
        ASM_CASE(ADD)
        ASM_CASE(SUB)
        ASM_CASE(MUL)
        ASM_CASE(DIV)
        ASM_CASE(DUMP)
        ASM_CASE(RET)

        ASM_CASE_ARG(PUSH)
        ASM_CASE_ARG(PUSHR)
        ASM_CASE_ARG(POP)

        ASM_CASE_LABEL_ARG(JMP)
        ASM_CASE_LABEL_ARG(JB)
        ASM_CASE_LABEL_ARG(JBE)
        ASM_CASE_LABEL_ARG(JA)
        ASM_CASE_LABEL_ARG(JAE)
        ASM_CASE_LABEL_ARG(JE)
        ASM_CASE_LABEL_ARG(JNE)
        ASM_CASE_LABEL_ARG(CALL)

        // syntax error
    }

    fixup(code, &ft, &la);
    code[ip] = CMD_END;
    writeCode(fout, code, ip);
    labelArrayDtor(&la);
    fixupTableDtor(&ft);

    #undef ASM_CASE
}

void initLabel(Label* label)
{
    label->adr = -1;
    label->name = NULL;
}

void initLabelArray(LabelArray* la)
{
    la->max_labels = 10;
    la->free = 0;
    la->labels = (Label*)calloc(la->max_labels, sizeof(Label));
    for (int i = 0; i < la->max_labels; ++i)
    {
        initLabel(la->labels + i);
    }
}

void labelDtor(Label* label)
{
    free(label->name);
}

void labelArrayDtor(LabelArray* la)
{
    for (int i = 0; i < la->max_labels; ++i)
    {
        labelDtor(la->labels + i);
    }
    free(la->labels);
}

void insertLabel(LabelArray* la, int adr, char* name)
{
    int max_label_len = 20;
    la->labels[la->free].adr = adr;

    la->labels[la->free].name = (char*)calloc(max_label_len, sizeof(char));
    strcpy(la->labels[la->free].name, name);
    ++(la->free);
}

void getLabelAdr(LabelArray* la, char* name, int* adr)
{
    for (int i = 0; i < la->max_labels; ++i)
    {
        if (la->labels[i].name != NULL && strcmp(la->labels[i].name, name) == 0)
        {
            *adr = la->labels[i].adr;
            return;
        }
    }
    *adr = -1;
}

void initFixupElem(FixupElem* fe)
{
    fe->ip = 0;
    fe->name = NULL;
}

void initFixupTable(FixupTable* ft)
{
    ft->max_elems = 100;
    ft->n_fixups = 0;
    ft->table = (FixupElem*)calloc(ft->max_elems, sizeof(FixupElem));
}

void fixupElemDtor(FixupElem* fe)
{
    free(fe->name);
}

void fixupTableDtor(FixupTable* ft)
{
    for (int i = 0; i < ft->n_fixups; ++i)
    {
        fixupElemDtor(ft->table + i);
    }
    free(ft->table);
}

void addFixup(FixupTable* ft, int ip, char* name)
{
    int max_label_len = 20;
    ft->table[ft->n_fixups].ip = ip;

    ft->table[ft->n_fixups].name = (char*)calloc(max_label_len, sizeof(char));
    strcpy(ft->table[ft->n_fixups].name, name);
    ++(ft->n_fixups);
}

void fixup(int* code, FixupTable* ft, LabelArray* la)
{
    int adr = -1;

    for (int fixup_n = 0; fixup_n < ft->n_fixups; ++fixup_n)
    {
        getLabelAdr(la, ft->table[fixup_n].name, &adr);
        code[ft->table[fixup_n].ip] = adr;
    }
}
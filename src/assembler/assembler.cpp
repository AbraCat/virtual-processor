#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

#include <assembler.h>
#include <cmd.h>

const int MEM = 0x80, REG = 0x40, IMM = 0x20;

int fileSize(FILE *file, long *siz)
{
    assert(file != NULL && siz != NULL);

    if (fseek(file, 0L, SEEK_END))
    {
        *siz = -1L;
        return errno;
    }

    *siz = ftell(file);

    if (*siz == -1L)
        return errno;
    if (fseek(file, 0L, SEEK_SET))
        return errno;

    return 0;
}

int readFile(FILE* file, char** bufptr)
{
    assert(file != NULL && bufptr != NULL);

    long siz = 0;
    int error = fileSize(file, &siz);

    if (error != 0)
    {
        *bufptr = NULL;
        return error;
    }

    char* buf = *bufptr = (char*)calloc(siz + 2, sizeof(char));
    if (buf == NULL)
        return errno;

    int n_read = fread(buf, sizeof(char), siz, file);
    if (ferror(file))
    {
        free(buf);
        *bufptr = NULL;
        return EIO;
    }

    if (buf[n_read - 1] != '\n')
        buf[n_read++] = '\n';
    buf[n_read] = '\0';
    return 0;
}

void clearComments(char* str)
{
    static const char comment_start = ';';

    int clearing = 0;
    for (; *str != '\0'; ++str)
    {
        if (*str == comment_start)
            clearing = 1;
        else if (*str == '\n')
            clearing = 0;
        if (clearing)
            *str = ' ';
    }
}

void asmCtor(Asm* ase)
{
    #define ALLOC_BUF(var) ase->var = (char*)calloc(ase->buffer_size, sizeof(char))

    ase->buffer_size = 30;
    ase->code_size = 100;

    ALLOC_BUF(str_cmd);
    ALLOC_BUF(str_arg1);
    ALLOC_BUF(str_arg2);
    ALLOC_BUF(label);
    ALLOC_BUF(chr1);

    ase->ip = ase->n_args = ase->arg1 = ase->arg2 = 
    ase->str_code_pos = ase->pos_incr = 0;

    ase->code = (int*)calloc(ase->code_size, sizeof(int));
    ase->str_code = NULL;

    initLabelArray(&ase->la);
    initFixupTable(&ase->ft);

    #undef ALLOC_BUF
}
void asmDtor(Asm* ase)
{
    free(ase->str_code);
    free(ase->str_cmd);
    free(ase->str_arg1);
    free(ase->str_arg2);
    free(ase->label);
    free(ase->chr1);
    free(ase->code);
    labelArrayDtor(&ase->la);
    fixupTableDtor(&ase->ft);
}

void getRegNum(char* str_name, int* num)
{
    #define REG_CASE(name)                \
        if (strcmp(str_name, #name) == 0) \
        {                                 \
            *num = (int)name;             \
            return;                       \
        }

    REG_CASE(AX)
    REG_CASE(BX)
    REG_CASE(CX)
    REG_CASE(DX)

    *num = -1;

    #undef REG_CASE
}

void getArg(Asm* ase)
{
    /*
    push:
    ok:    001 010 011 101 110 111
    wrong: 000 100

    pop:
    ok:    010 101 110 111
    wrong: 000 001 011 100
    */

    ase->chr1 = strchr(ase->str_arg1, ']');
    if (ase->chr1 != NULL && sscanf(ase->str_arg1, "[%s", ase->str_arg2) == 1)
    {
        ase->code[ase->ip] |= MEM;
        *(ase->str_arg2 + (ase->chr1 - ase->str_arg1) - 1) = '\0'; // ] in str_arg2
        strcpy(ase->str_arg1, ase->str_arg2);
    }

    ase->chr1 = strchr(ase->str_arg1, '+');
    if (ase->chr1 != NULL && sscanf(ase->chr1 + 1, "%d", &ase->arg2) == 1)
    {
        ase->code[ase->ip] |= REG | IMM;
        *(ase->chr1) = '\0';
        getRegNum(ase->str_arg1, &ase->arg1);
        ase->code[ase->ip + 1] = ase->arg1;
        ase->code[ase->ip + 2] = ase->arg2;
        ase->ip += 3;
    }

    else if (sscanf(ase->str_arg1, "%d", &ase->arg1) == 1)
    {
        ase->code[ase->ip] |= IMM;
        ase->code[ase->ip + 1] = ase->arg1;
        ase->ip += 2;
    }

    else
    {
        ase->code[ase->ip] |= REG;
        sscanf(ase->str_arg1, "%s", ase->str_arg2);
        getRegNum(ase->str_arg2, &ase->arg2);
        ase->code[ase->ip + 1] = ase->arg2;
        ase->ip += 2;
    }
}

void runAsm(FILE* fin, FILE* fout)
{
    #define myScanf(fmt, ...)                               \
    (                                                       \
        scanf_res = sscanf(ase.str_code + ase.str_code_pos, \
        fmt "%n", __VA_ARGS__, &ase.pos_incr),              \
        ase.str_code_pos += ase.pos_incr,                   \
        scanf_res                                           \
    )

    #define ASM_CASE(command)                     \
        if (strcmp(ase.str_cmd, #command) == 0)   \
        {                                         \
            ase.code[ase.ip++] = CMD_ ## command; \
            continue;                             \
        }

    #define ASM_CASE_ARG(command)                 \
        if (strcmp(ase.str_cmd, #command) == 0)   \
        {                                         \
            ase.code[ase.ip++] = CMD_ ## command; \
            myScanf("%d", &ase.arg1);             \
            ase.code[ase.ip++] = ase.arg1;        \
            continue;                             \
        }

    #define ASM_CASE_COMPLEX_ARG(command)       \
        if (strcmp(ase.str_cmd, #command) == 0) \
        {                                       \
            ase.code[ase.ip] = CMD_ ## command; \
            myScanf("%s", ase.str_arg1);        \
            getArg(&ase);                       \
            continue;                           \
        }

    #define ASM_CASE_LABEL_ARG(command)                           \
        if (strcmp(ase.str_cmd, #command) == 0)                   \
        {                                                         \
            ase.code[ase.ip++] = CMD_ ## command;                 \
            myScanf("%s", ase.label);                             \
            if (ase.label[strlen(ase.label) - 1] == ':')          \
            {                                                     \
                getLabelAdr(&ase.la, ase.label, &ase.arg1);       \
                ase.code[ase.ip++] = ase.arg1;                    \
                if (ase.arg1 == -1)                               \
                {                                                 \
                    addFixup(&ase.ft, ase.ip - 1, ase.label);     \
                }                                                 \
            }                                                     \
            else                                                  \
            {                                                     \
                ase.code[ase.ip++] = strtol(ase.label, NULL, 10); \
            }                                                     \
            continue;                                             \
        }

    assert(fin != NULL && fout != NULL);

    int scanf_res = 0;
    Asm ase = {};
    asmCtor(&ase);

    readFile(fin, &ase.str_code);
    clearComments(ase.str_code);

    // printf("no comments:\n\n%s\n", ase.str_code);
    
    while (1)
    {
        if (myScanf("%s", ase.str_cmd) == EOF)
            break;

        if (ase.str_cmd[strlen(ase.str_cmd) - 1] == ':')
            insertLabel(&ase.la, ase.ip, ase.str_cmd);
        
        ASM_CASE(HLT)
        ASM_CASE(IN)
        ASM_CASE(OUT)
        ASM_CASE(ADD)
        ASM_CASE(SUB)
        ASM_CASE(MUL)
        ASM_CASE(DIV)
        ASM_CASE(DUMP)
        ASM_CASE(RET)
        ASM_CASE(DRAW)

        ASM_CASE_COMPLEX_ARG(PUSH)
        ASM_CASE_COMPLEX_ARG(POP)

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

    fixup(ase.code, &ase.ft, &ase.la);
    ase.code[ase.ip] = CMD_END;
    fwrite(ase.code, sizeof(int), ase.code_size, fout);
    asmDtor(&ase);

    #undef myScanf
    #undef ASM_CASE
    #undef ASM_CASE_ARG
    #undef ASM_CASE_COMPLEX_ARG
    #undef ASM_CASE_LABEL_ARG
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
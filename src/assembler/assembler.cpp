#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>

#include <assembler.h>
#include <common.h>
#include <error.h>
#include <utils.h>

const int MEM = 0x80, REG = 0x40, IMM = 0x20, max_label_len = 20;

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

ErrEnum asmCtor(Asm* ase)
{
    #define ALLOC_BUF(var)                                        \
        ase->var = (char*)calloc(ase->buffer_size, sizeof(char)); \
        if (ase->var == NULL) return ERR_MEM;

    ase->buffer_size = 30;
    ase->code_size = 100;

    ALLOC_BUF(str_cmd);
    ALLOC_BUF(str_arg1);
    ALLOC_BUF(str_arg2);
    ALLOC_BUF(label);

    ase->chr1 = NULL;
    ase->ip = ase->n_args = ase->arg1 = ase->arg2 = 
    ase->str_code_pos = ase->pos_incr = 0;

    ase->code = (int*)calloc(ase->code_size, sizeof(int));
    if (ase->code == NULL)
        return ERR_MEM;
    ase->str_code = NULL;

    labelArrayCtor(&ase->la);
    fixupTableCtor(&ase->ft);

    #undef ALLOC_BUF

    return OK;
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

    *num = INVAL_REG;

    #undef REG_CASE
}

ErrEnum getArg(Asm* ase)
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
    if (ase->chr1 != NULL)
    {
        if (sscanf(ase->str_arg1, "[%s", ase->str_arg2) != 1)
            return ERR_BRACKET;
        ase->code[ase->ip] |= MEM;
        *(ase->str_arg2 + (ase->chr1 - ase->str_arg1) - 1) = '\0'; // ] in str_arg2
        strcpy(ase->str_arg1, ase->str_arg2);
    }

    ase->chr1 = strchr(ase->str_arg1, '+');
    if (ase->chr1 != NULL)
    {
        if (sscanf(ase->chr1 + 1, "%d", &ase->arg2) != 1)
            return ERR_CMD_ARG_FMT;
        ase->code[ase->ip] |= REG | IMM;
        *(ase->chr1) = '\0';
        getRegNum(ase->str_arg1, &ase->arg1);
        if (ase->arg1 == INVAL_REG)
            return ERR_REG_NAME;
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
        if (ase->arg2 == INVAL_REG)
            return ERR_REG_NAME;
        ase->code[ase->ip + 1] = ase->arg2;
        ase->ip += 2;
    }

    return OK;
}

ErrEnum runAsm(FILE* fin, FILE* fout)
{
    #define myScanf(fmt, ...)                               \
    (                                                       \
        scanf_res = sscanf(ase.str_code + ase.str_code_pos, \
        fmt "%n", __VA_ARGS__, &ase.pos_incr),              \
        ase.str_code_pos += ase.pos_incr,                   \
        scanf_res                                           \
    )

    #define myScanfExp(exp_res, fmt, ...)         \
        if (myScanf(fmt, __VA_ARGS__) != exp_res) \
            return ERR_CMD_ARG_FMT;

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
            myScanfExp(1, "%d", &ase.arg1);       \
            ase.code[ase.ip++] = ase.arg1;        \
            continue;                             \
        }

    #define ASM_CASE_COMPLEX_ARG(command)       \
        if (strcmp(ase.str_cmd, #command) == 0) \
        {                                       \
            ase.code[ase.ip] = CMD_ ## command; \
            myScanfExp(1, "%s", ase.str_arg1);  \
            getArg(&ase);                       \
            continue;                           \
        }

    #define ASM_CASE_LABEL_ARG(command)                           \
        if (strcmp(ase.str_cmd, #command) == 0)                   \
        {                                                         \
            ase.code[ase.ip++] = CMD_ ## command;                 \
            myScanfExp(1, "%s", ase.label);                       \
            if (ase.label[strlen(ase.label) - 1] == ':')          \
            {                                                     \
                getLabelAdr(&ase.la, ase.label, &ase.arg1);       \
                ase.code[ase.ip++] = ase.arg1;                    \
                if (ase.arg1 == -1)                               \
                    addFixup(&ase.ft, ase.ip - 1, ase.label);     \
            }                                                     \
            else                                                  \
                ase.code[ase.ip++] = strtol(ase.label, NULL, 10); \
            continue;                                             \
        }

    myAssert(fin != NULL && fout != NULL);

    int scanf_res = 0;
    Asm ase = {};
    returnErr(asmCtor(&ase));

    returnErr(readFile(fin, &ase.str_code));
    clearComments(ase.str_code);

    // printf("no comments:\n\n%s\n", ase.str_code);
    
    while (1)
    {
        if (myScanf("%s", ase.str_cmd) == EOF)
            break;

        if (ase.str_cmd[strlen(ase.str_cmd) - 1] == ':')
        {
            addLabel(&ase.la, ase.ip, ase.str_cmd);
            continue;
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

        asmDtor(&ase);
        return ERR_INVAL_CMD;
    }

    fixup(ase.code, &ase.ft, &ase.la);
    ase.code[ase.ip] = CMD_END;

    if (fwrite(ase.code, sizeof(int), ase.code_size, fout) < ase.code_size)
        return ERR_IO;

    asmDtor(&ase);

    #undef myScanfExp
    #undef ASM_CASE
    #undef ASM_CASE_ARG
    #undef ASM_CASE_COMPLEX_ARG
    #undef ASM_CASE_LABEL_ARG

    return OK;
}

void labelCtor(Label* label)
{
    label->adr = -1;
    label->name = NULL;
}

ErrEnum labelArrayCtor(LabelArray* la)
{
    la->max_labels = 10;
    la->n_labels = 0;

    la->labels = (Label*)calloc(la->max_labels, sizeof(Label));
    la->name_buf = (char*)calloc(max_label_len * la->max_labels, sizeof(char));

    if (la->labels == NULL || la->name_buf == NULL)
        return ERR_MEM;

    for (int i = 0; i < la->max_labels; ++i)
        labelCtor(la->labels + i);

    return OK;
}

void labelArrayDtor(LabelArray* la)
{
    free(la->labels);
    free(la->name_buf);
}

void addLabel(LabelArray* la, int adr, char* name)
{
    la->labels[la->n_labels].adr = adr;

    la->labels[la->n_labels].name = la->name_buf + la->n_labels * max_label_len;
    strncpy(la->labels[la->n_labels].name, name, max_label_len);
    ++(la->n_labels);
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

void fixupElemCtor(FixupElem* fe)
{
    fe->ip = 0;
    fe->name = NULL;
}

ErrEnum fixupTableCtor(FixupTable* ft)
{
    ft->max_elems = 100;
    ft->n_fixups = 0;
    ft->table = (FixupElem*)calloc(ft->max_elems, sizeof(FixupElem));
    ft->name_buf = (char*)calloc(ft->max_elems * max_label_len, sizeof(char));
    if (ft->table == NULL || ft->name_buf == NULL)
        return ERR_MEM;

    return OK;
}

void fixupTableDtor(FixupTable* ft)
{
    free(ft->table);
    free(ft->name_buf);
}

void addFixup(FixupTable* ft, int ip, char* name)
{
    ft->table[ft->n_fixups].ip = ip;

    ft->table[ft->n_fixups].name = ft->name_buf + ft->n_fixups * max_label_len;
    strncpy(ft->table[ft->n_fixups].name, name, max_label_len);
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
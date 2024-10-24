#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#include <assembler.h>
#include <common.h>
#include <error.h>
#include <utils.h>

const int max_label_len = 20, buffer_size = 30;

void clearComments(char* str)
{
    const char comment_start = ';';

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
    ase->ip = ase->arg1 = ase->arg2 = ase->str_code_pos = 0;

    ase->str_cmd = (char*)calloc(buffer_size, sizeof(char));
    if (ase->str_cmd == NULL)
        return ERR_MEM;
    ase->code = (int*)calloc(max_cmds, sizeof(int));
    if (ase->code == NULL)
        return ERR_MEM;
    ase->str_code = NULL;

    labelArrayCtor(&ase->la);
    fixupTableCtor(&ase->ft);

    return ERR_OK;
}
void asmDtor(Asm* ase)
{
    free(ase->str_code);
    free(ase->str_cmd);
    free(ase->code);
    labelArrayDtor(&ase->la);
    fixupTableDtor(&ase->ft);
}

void getRegNum(char* str_name, int* num)
{
    if ((!isspace(str_name[2]) && str_name[2] != '\0') || 
    str_name[1] != 'X' || str_name[0] - 'A' < 0 || str_name[0] - 'A' >= n_regs)
    {
        *num = INVAL_REG;
        return;
    }
    *num = str_name[0] - 'A';
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

    char *str_ptr1 = ase->str_code + ase->str_code_pos;
    while (isspace(*str_ptr1))
        ++str_ptr1;
    if (*str_ptr1 == '\0')
        return ERR_CMD_NO_ARG;

    char *str_ptr2 = str_ptr1;
    while (!isspace(*str_ptr2) && *str_ptr2 != '\0')
        ++str_ptr2;
    if (*str_ptr2 == '\0')
        ase->str_code_pos = str_ptr2 - ase->str_code;
    else
    {
        *str_ptr2 = '\0';
        ase->str_code_pos = str_ptr2 - ase->str_code + 1;
    }

    char* chr1  = strchr(str_ptr1, ']');
    if (chr1 != NULL)
    {
        if (str_ptr1[0] != '[')
            return ERR_BRACKET;
        ase->code[ase->ip] |= MEM_BIT;
        ++str_ptr1;
        *chr1 = ' ';
    }

    chr1 = strchr(str_ptr1, '+');
    if (chr1 != NULL)
    {
        if (sscanf(chr1 + 1, "%d", ase->code + ase->ip + 2) != 1)
            return ERR_CMD_ARG_FMT;
        ase->code[ase->ip] |= REG_BIT | IMM_BIT;
        *(chr1) = ' ';
        getRegNum(str_ptr1, ase->code + ase->ip + 1);
        if (ase->code[ase->ip + 1] == INVAL_REG)
            return ERR_INVAL_REG_NAME;
        ase->ip += 3;
    }

    else if (sscanf(str_ptr1, "%d", ase->code + ase->ip + 1) == 1)
    {
        ase->code[ase->ip] |= IMM_BIT;
        ase->ip += 2;
    }

    else
    {
        ase->code[ase->ip] |= REG_BIT;
        getRegNum(str_ptr1, ase->code + ase->ip + 1);
        if (ase->code[ase->ip + 1] == INVAL_REG)
            return ERR_INVAL_REG_NAME;
        ase->ip += 2;
    }

    return ERR_OK;
}

ErrEnum runAsm(FILE* fin, FILE* fout)
{
    #define myScanf(fmt, ...)                               \
    (                                                       \
        scanf_res = sscanf(ase.str_code + ase.str_code_pos, \
        fmt "%n", __VA_ARGS__, &pos_incr),                  \
        ase.str_code_pos += pos_incr,                       \
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
            returnErr(getArg(&ase));            \
            continue;                           \
        }

    #define ASM_CASE_LABEL_ARG(command)                       \
        if (strcmp(ase.str_cmd, #command) == 0)               \
        {                                                     \
            ase.code[ase.ip++] = CMD_ ## command;             \
            myScanfExp(1, "%s", label);                       \
            if (label[strlen(label) - 1] == ':')              \
            {                                                 \
                getLabelAdr(&ase.la, label, &ase.arg1);       \
                ase.code[ase.ip++] = ase.arg1;                \
                if (ase.arg1 == -1)                           \
                    addFixup(&ase.ft, ase.ip - 1, label);     \
            }                                                 \
            else                                              \
                ase.code[ase.ip++] = strtol(label, NULL, 10); \
            continue;                                         \
        }

    myAssert(fin != NULL && fout != NULL);

    int scanf_res = 0, pos_incr = 0;
    char label[buffer_size] = "";
    Asm ase = {};
    returnErr(asmCtor(&ase));

    long str_code_size = 0;
    returnErr(fileSize(fin, &str_code_size));
    
    ase.str_code = (char*)calloc(str_code_size + 1, sizeof(char));
    if (ase.str_code == NULL) return ERR_MEM;
    fread(ase.str_code, sizeof(char), str_code_size, fin);

    clearComments(ase.str_code);
    
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
        ASM_CASE(SQRT)

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

    returnErr(fixup(ase.code, &ase.ft, &ase.la));
    if (fwrite(ase.code, sizeof(int), ase.ip, fout) != ase.ip)
        return ERR_IO;
    asmDtor(&ase);

    #undef myScanfExp
    #undef ASM_CASE
    #undef ASM_CASE_ARG
    #undef ASM_CASE_COMPLEX_ARG
    #undef ASM_CASE_LABEL_ARG

    return ERR_OK;
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

    return ERR_OK;
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

    return ERR_OK;
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

ErrEnum fixup(int* code, FixupTable* ft, LabelArray* la)
{
    int adr = -1;
    for (int fixup_n = 0; fixup_n < ft->n_fixups; ++fixup_n)
    {
        getLabelAdr(la, ft->table[fixup_n].name, &adr);
        if (adr == -1)
            return ERR_INVAL_LABEL;
        code[ft->table[fixup_n].ip] = adr;
    }
    return ERR_OK;
}
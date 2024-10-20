#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include <disassembler.h>
#include <common.h>
#include <error.h>

static const int MAX_CMDS = 100;
const int MEM = 0x80, REG = 0x40, IMM = 0x20, MASK_CMD = 0x1F, MASK_ARGT = 0xE0;

ErrEnum printRegName(RegEnum reg_num, FILE* fout)
{
    myAssert(fout != NULL);

    #define REG_CASE(enum_elem)                 \
        case enum_elem:                         \
            if (fputs(#enum_elem, fout) == EOF) \
                return ERR_IO;                  \
            break;

    switch (reg_num)
    {
        REG_CASE(AX)
        REG_CASE(BX)
        REG_CASE(CX)
        REG_CASE(DX)
        REG_CASE(WRONG_REG)

        default:
            if (fputs("WRING_REG", fout) == EOF)
                return ERR_IO;
            break;
    }

    return OK;

    #undef REG_CASE
}

ErrEnum disasmCtor(Disasm* dis)
{
    dis->ip = 0;
    dis->code = (int*)calloc(MAX_CMDS, sizeof(int));
    if (dis->code == NULL)
        return ERR_MEM;
    return OK;
}

void disasmDtor(Disasm* dis)
{
    free(dis->code);
}

ErrEnum printComplexArg(Disasm* dis, FILE* fout)
{
    /*
    push:
    ok:    001 010 011 101 110 111
    wrong: 000 100

    pop:
    ok:    010 101 110 111
    wrong: 000 001 011 100

    001 011     - arg1 (used for push)
    010         - reg
    101 110 111 - ram
    */

   int argt = dis->code[dis->ip - 1] & MASK_ARGT, mem = 0;

    if ((argt & (IMM | REG | MEM)) == 0 || (argt & (IMM | REG | MEM)) == MEM)
        return ERR_INSTR_ARG_FMT;

    if (argt & MEM)
    {
        mem = 1;
        putc('[', fout);
    }

    if (argt & REG)
        returnErr(printRegName((RegEnum)(dis->code[dis->ip++]), fout));

    if ((argt & REG) && (argt & IMM))
        putc('+', fout);

    if (argt & IMM)
        fprintf(fout, "%d", dis->code[dis->ip++]);

    if (mem)
        putc(']', fout);

    return OK;
}

ErrEnum runDisasm(FILE* fin, FILE* fout)
{
    #define DISASM_CASE(command)        \
        case CMD_ ## command:           \
            fputs(#command "\n", fout); \
            break;

    #define DISASM_CASE_ARG(command)        \
        case CMD_ ## command:               \
            fprintf(fout, #command " %d\n", \
            dis.code[dis.ip++]);            \
            break;

    #define DISASM_CASE_COMPLEX_ARG(command)      \
        case CMD_ ## command:                     \
            fputs(#command " ", fout);            \
            printComplexArg(&dis, fout);          \
            putc('\n', fout);                     \
            break;

    myAssert(fin != NULL && fout != NULL);

    Disasm dis = {};
    returnErr(disasmCtor(&dis));

    fread(dis.code, sizeof(int), MAX_CMDS, fin);

    while (1)
    {
        if (dis.ip >= MAX_CMDS)
            return ERR_IP_BOUNDARY;

        switch (dis.code[dis.ip++] & MASK_CMD)
        {
            case CMD_END:
                disasmDtor(&dis);
                return OK;

            DISASM_CASE(HLT)
            DISASM_CASE(IN)
            DISASM_CASE(OUT)
            DISASM_CASE(ADD)
            DISASM_CASE(SUB)
            DISASM_CASE(MUL)
            DISASM_CASE(DIV)
            DISASM_CASE(DUMP)
            DISASM_CASE(RET)
            DISASM_CASE(DRAW)

            DISASM_CASE_ARG(JMP)
            DISASM_CASE_ARG(JB)
            DISASM_CASE_ARG(JBE)
            DISASM_CASE_ARG(JA)
            DISASM_CASE_ARG(JAE)
            DISASM_CASE_ARG(JE)
            DISASM_CASE_ARG(JNE)
            DISASM_CASE_ARG(CALL)

            DISASM_CASE_COMPLEX_ARG(PUSH)
            DISASM_CASE_COMPLEX_ARG(POP)
            
            default:
                disasmDtor(&dis);
                return ERR_INVAL_INSTR;
        }
    }

    disasmDtor(&dis);
    return OK;

    #undef DISASM_CASE
    #undef DISASM_CASE_ARG
}
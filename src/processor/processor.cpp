#include <assert.h>
#include <stdlib.h>

#include <processor.h>
#include <common.h>
#include <colors.h>
#include <error.h>

static const int MAX_CMDS = 100, ram_size = 10;
const int MEM = 0x80, REG = 0x40, IMM = 0x20, MASK_CMD = 0x1F, MASK_ARGT = 0xE0;

ErrEnum procCtor(Proc* prc)
{
    stCtor(&prc->st, 0);
    stCtor(&prc->ret, 0);
    prc->ip = prc->cmd = prc->argt = prc->arg1 = prc->arg2 = 0;
    prc->code = (int*)calloc(MAX_CMDS, sizeof(int));
    prc->ram = (int*)calloc(ram_size * ram_size * 2, sizeof(int));

    if (prc->code == NULL || prc->ram == NULL)
        return ERR_MEM;

    initRam(prc);

    return OK;
}
void procDtor(Proc* prc)
{
    stDtor(&prc->st);
    stDtor(&prc->ret);
    free(prc->reg);
    free(prc->ram);
}

ErrEnum getPopDestination(Proc* prc, int** dest)
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

    if ((prc->argt & (IMM | REG | MEM)) == 0 || (prc->argt & (IMM | REG | MEM)) == MEM)
    {
        *dest = NULL;
        return ERR_INSTR_ARG_FMT;
    }

    if ((prc->argt & (IMM | REG | MEM)) == REG)
    {
        *dest = prc->reg + prc->code[prc->ip++];
        return OK;
    }

    int argv = 0;
    if (prc->argt & REG)
        argv += prc->reg[prc->code[prc->ip++]];
    if (prc->argt & IMM)
        argv += prc->code[prc->ip++];
    if (prc->argt & MEM)
    {
        *dest = prc->ram + argv;
        return OK;
    }

    // push
    prc->arg1 = argv;
    *dest = &prc->arg1;
    return OK;
}

ErrEnum runProc(FILE* fcode, FILE* fin, FILE* fout)
{
    myAssert(fin != NULL && fout != NULL);

    struct Proc prc = {};
    returnErr(procCtor(&prc));
    fread(prc.code, sizeof(int), MAX_CMDS, fcode);
    int* code = prc.code;

    while (1)
    {
        if (prc.ip < 0 || prc.ip >= MAX_CMDS)
            return ERR_IP_BOUNDARY;
         
        // printf("ip = %d\n", prc.ip);
        if (code[prc.ip] == CMD_END)
        {
            procDtor(&prc);
            return OK;
        }

        prc.cmd = code[prc.ip] & MASK_CMD;
        prc.argt = code[prc.ip++] & MASK_ARGT;
        switch(prc.cmd)
        {
            case CMD_HLT:
                procDtor(&prc);
                return OK;
            case CMD_IN:
                fscanf(fin, "%d", &prc.arg1);
                returnErr(stPush(&prc.st, prc.arg1));
                break;
            case CMD_OUT:
                returnErr(stPop(&prc.st, &prc.arg1));
                fprintf(fout, "%d\n", prc.arg1);
                break;

            #define CASE_ARITHM_OP(cmd, op)                           \
                case CMD_ ## cmd:                                     \
                    returnErr(stPop(&prc.st, &prc.arg1));             \
                    returnErr(stPop(&prc.st, &prc.arg2));             \
                    returnErr(stPush(&prc.st, prc.arg2 op prc.arg1)); \
                    break;

            CASE_ARITHM_OP(ADD, +)
            CASE_ARITHM_OP(SUB, -)
            CASE_ARITHM_OP(MUL, *)
            CASE_ARITHM_OP(DIV, /)

            case CMD_DUMP:
                prcDump(fout, &prc);
                break;
            case CMD_PUSH:
            {
                int *arg = NULL;
                returnErr(getPopDestination(&prc, &arg));
                returnErr(stPush(&prc.st, *arg));
                break;
            }
            case CMD_POP:
            {
                int *arg = NULL;
                returnErr(getPopDestination(&prc, &arg));
                returnErr(stPop(&prc.st, arg));
                break;
            }
            case CMD_DRAW:
                returnErr(drawRam(&prc, stdout));
                break;
            case CMD_JMP:
                prc.ip = code[prc.ip];
                break;

            #define CASE_JMP(jmp, op)                     \
                case CMD_ ## jmp:                         \
                    returnErr(stPop(&prc.st, &prc.arg2)); \
                    returnErr(stPop(&prc.st, &prc.arg1)); \
                    if (prc.arg1 op prc.arg2)             \
                        prc.ip = code[prc.ip];            \
                    else                                  \
                        ++prc.ip;                         \
                    break;

            CASE_JMP(JB, <)
            CASE_JMP(JBE, <=)
            CASE_JMP(JA, >)
            CASE_JMP(JAE, >=)
            CASE_JMP(JE, ==)
            CASE_JMP(JNE, !=)
            
            #undef CASE_JMP

            case CMD_CALL:
                returnErr(stPush(&prc.ret, prc.ip + 1));
                prc.ip = code[prc.ip];
                break;
            case CMD_RET:
                returnErr(stPop(&prc.ret, &prc.ip));
                break;
            default:
                fprintf(fout, "Invalid instruction: %d ip: %d\n", prc.code[prc.ip - 1], prc.ip - 1);
                procDtor(&prc);
                return ERR_INVAL_INSTR;
        }
    }

    procDtor(&prc);
    return OK;
}

void initRam(Proc* prc)
{
    myAssert(prc != NULL);

    int adr = 0;
    for (int i = 0; i < ram_size; ++i)
    {
        for (int j = 0; j < ram_size; ++j)
        {
            adr = 2 * (i * ram_size + j);
            prc->ram[adr] = '.';
            prc->ram[adr + 1] = 9;
        }
    }
    return;
}

ErrEnum drawRam(Proc* prc, FILE* fout)
{
    myAssert(prc != NULL && fout != NULL);

    int adr = 0;
    for (int i = 0; i < ram_size; ++i)
    {
        for (int j = 0; j < ram_size; ++j)
        {
            adr = 2 * (i * ram_size + j);
            fprintf(fout, "\x1b[3%cm%c%c\x1b[39m", '0' + prc->ram[adr + 1], prc->ram[adr], prc->ram[adr]);
        }
        putc('\n', fout);
    }

    return OK;
}

void prcDump(FILE* fout, Proc* prc)
{
    myAssert(prc != NULL || fout != NULL);

    fprintf(fout, "Processor [0x%p]\n\n", prc);
    fprintf(fout, "ip: %d\n\n", prc->ip);
    for (int reg_num = 0; reg_num < n_regs; ++reg_num)
    {
        printRegName((RegEnum)reg_num, fout);
        fprintf(fout, ": %d\n", prc->reg[reg_num]);
    }
    putc('\n', fout);
    stDump(fout, &prc->st);
    fputs("RAM:\n\n", fout);
    drawRam(prc, fout);
}
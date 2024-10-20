#include <assert.h>
#include <stdlib.h>

#include <processor.h>
#include <cmd.h>
#include <colors.h>

static const int MAX_CMDS = 100;
const int MEM = 0x80, REG = 0x40, IMM = 0x20, MASK_CMD = 0x1F, MASK_ARGT = 0xE0;

void procCtor(Proc* prc, int* code)
{
    prc->ram_size = 5;

    stCtor(&prc->st, 0);
    stCtor(&prc->ret, 0);
    prc->ip = prc->cmd = prc->argt = prc->arg1 = prc->arg2 = 0;
    prc->code = code;
    prc->ram = (int*)calloc(prc->ram_size * prc->ram_size * 2, sizeof(int));
}
void procDtor(Proc* prc)
{
    stDtor(&prc->st);
    stDtor(&prc->ret);
    free(prc->reg);
    free(prc->ram);
}

void getPopDestination(Proc* prc, int** dest)
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
        return;
    }

    if ((prc->argt & (IMM | REG | MEM)) == REG)
    {
        *dest = prc->reg + prc->code[prc->ip++];
        return;
    }

    int argv = 0;
    if (prc->argt & REG)
    {
        argv += prc->reg[prc->code[prc->ip++]];
    }
    if (prc->argt & IMM)
    {
        argv += prc->code[prc->ip++];
    }
    if (prc->argt & MEM)
    {
        *dest = prc->ram + argv;
        return;
    }

    // push
    prc->arg1 = argv;
    *dest = &prc->arg1;
    return;
}

void runProc(int* code, FILE* fin, FILE* fout)
{
    assert(fin != NULL && fout != NULL);

    struct Proc prc = {};
    procCtor(&prc, code);

    while (1)
    {
        if (prc.ip < 0 || prc.ip >=  MAX_CMDS || code[prc.ip] == CMD_END)
        {
            return;
        }
        prc.cmd = code[prc.ip] & MASK_CMD;
        prc.argt = code[prc.ip++] & MASK_ARGT;
        switch(prc.cmd)
        {
            case CMD_HLT:
                return;
            case CMD_IN:
                fscanf(fin, "%d", &prc.arg1);
                stPush(&prc.st, prc.arg1);
                break;
            case CMD_OUT:
                stPop(&prc.st, &prc.arg1);
                fprintf(fout, "%d\n", prc.arg1);
                break;
            case CMD_ADD:
                stPop(&prc.st, &prc.arg1);
                stPop(&prc.st, &prc.arg2);
                stPush(&prc.st, prc.arg1 + prc.arg2);
                break;
            case CMD_SUB:
                stPop(&prc.st, &prc.arg1);
                stPop(&prc.st, &prc.arg2);
                stPush(&prc.st, prc.arg2 - prc.arg1);
                break;
            case CMD_MUL:
                stPop(&prc.st, &prc.arg1);
                stPop(&prc.st, &prc.arg2);
                stPush(&prc.st, prc.arg1 * prc.arg2);
                break;
            case CMD_DIV:
                stPop(&prc.st, &prc.arg1);
                stPop(&prc.st, &prc.arg2);
                stPush(&prc.st, prc.arg2 / prc.arg1);
                break;
            case CMD_DUMP:
                stDump(fout, &prc.st);
                break;
            case CMD_PUSH:
            {
                int *arg = NULL;
                getPopDestination(&prc, &arg);
                if (arg == NULL)
                {
                    fprintf(fout, "Invalid argument type: %d\n", prc.argt);
                    break;
                }
                stPush(&prc.st, *arg);
                break;
            }
            case CMD_POP:
            {
                int *arg = NULL;
                getPopDestination(&prc, &arg);
                if (arg == NULL)
                {
                    fprintf(fout, "Invalid argument type: %d\n", prc.argt);
                    break;
                }
                stPop(&prc.st, arg);
                break;
            }
            case CMD_DRAW:
                drawRam(&prc);
                break;
            case CMD_JMP:
                prc.ip = code[prc.ip];
                break;

            #define CASE_JMP(jmp, op)          \
                case CMD_ ## jmp:              \
                    stPop(&prc.st, &prc.arg2); \
                    stPop(&prc.st, &prc.arg1); \
                    if (prc.arg1 op prc.arg2)  \
                        prc.ip = code[prc.ip]; \
                    else                       \
                        ++prc.ip;              \
                    break;

            CASE_JMP(JB, <)
            CASE_JMP(JBE, <=)
            CASE_JMP(JA, >)
            CASE_JMP(JAE, >=)
            CASE_JMP(JE, ==)
            CASE_JMP(JNE, !=)
            
            #undef CASE_JMP

            case CMD_CALL:
                stPush(&prc.ret, prc.ip + 1);
                prc.ip = code[prc.ip];
                break;
            case CMD_RET:
                stPop(&prc.ret, &prc.ip);
                break;
            default:
                fprintf(fout, "Invalid instruction: %d ip: %d\n", prc.code[prc.ip - 1], prc.ip);
                break;
        }
    }

    procDtor(&prc);
}

void drawRam(Proc* prc)
{
    for (int i = 0; i < prc->ram_size; ++i)
    {
        for (int j = 0; j < prc->ram_size; ++j)
        {
            printf("\x1b[3%cm%c%c\x1b[39m", '0' + prc->ram[(i * prc->ram_size + j) * 2 + 1], 
            prc->ram[(i * prc->ram_size + j) * 2], prc->ram[(i * prc->ram_size + j) * 2]);
        }
        putc('\n', stdout);
    }
}
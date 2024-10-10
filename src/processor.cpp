#include <assert.h>

#include <processor.h>
#include <cmd.h>
#include <stack.h>

static const int MAX_CMDS = 100;

void runProc(int* code, FILE* fin, FILE* fout)
{
    assert(fin != NULL && fout != NULL);

    Stack st = {};
    stCtor(&st, 0);
    int ip = 0, cmd = 0, arg1 = 0, arg2 = 0;
    while (1)
    {
        if (ip == MAX_CMDS)
        {
            return;
        }
        cmd = code[ip++];
        switch(cmd)
        {
            case CMD_HLT:
                return;
            case CMD_IN:
                fscanf(fin, "%d", &arg1);
                stPush(&st, arg1);
                break;
            case CMD_OUT:
                stPop(&st, &arg1);
                fprintf(fout, "%d\n", arg1);
                break;
            case CMD_PUSH:
                stPush(&st, code[ip++]);
                break;
            case CMD_ADD:
                stPop(&st, &arg1);
                stPop(&st, &arg2);
                stPush(&st, arg1 + arg2);
                break;
            case CMD_SUB:
                stPop(&st, &arg1);
                stPop(&st, &arg2);
                stPush(&st, arg2 - arg1);
                break;
            case CMD_MUL:
                stPop(&st, &arg1);
                stPop(&st, &arg2);
                stPush(&st, arg1 * arg2);
                break;
            case CMD_DIV:
                stPop(&st, &arg1);
                stPop(&st, &arg2);
                stPush(&st, arg2 / arg1);
                break;
            case CMD_DUMP:
                stDump(fout, &st);
                break;
            default:
                fprintf(fout, "Invalid instruction\n");
                break;
        }
    }
}
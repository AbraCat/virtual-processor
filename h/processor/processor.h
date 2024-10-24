#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include <stack.h>

#include <error.h>
#include <common.h>

struct Proc
{
    Stack st, ret;
    int ip, cmd, argt, arg1, arg2;
    int *code, *ram;
    int reg[n_regs];
};

ErrEnum procCtor(Proc* prc);
void procDtor(Proc* prc);

ErrEnum runProc(FILE* fcode, FILE* fin, FILE* fout);
ErrEnum getPopDestination(Proc* prc, int** dest);
void initRam(Proc* prc);
ErrEnum drawRam(FILE* fout, Proc* prc);
void prcDump(FILE* fout, Proc* prc);

#endif // PROCESSOR_H
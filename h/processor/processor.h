#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include <stack.h>

#include <error.h>

const int n_regs = 4;

struct Proc
{
    Stack st, ret;
    int ip, cmd, argt, arg1, arg2;
    int *code, *ram;
    int reg[n_regs];

    int ram_size;
};

ErrEnum procCtor(Proc* prc);
void procDtor(Proc* prc);

ErrEnum runProc(FILE* fcode, FILE* fin, FILE* fout);
ErrEnum getPopDestination(Proc* prc, int** dest);
ErrEnum drawRam(Proc* prc);

#endif // PROCESSOR_H
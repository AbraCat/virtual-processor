#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include <stack.h>

const int n_regs = 4;

struct Proc
{
    Stack st, ret;
    int ip, cmd, argt, arg1, arg2;
    int *code, *ram;
    int reg[n_regs];

    int ram_size;
};

void procCtor(Proc* prc, int* code);
void procDtor(Proc* prc);

void runProc(int* code, FILE* fin, FILE* fout);
void getPopDestination(Proc* prc, int** dest);
void drawRam(Proc* prc);

#endif // PROCESSOR_H
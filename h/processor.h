#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include <stack.h>

struct Proc
{
    Stack st, ret;
    int ip, cmd, argt, arg1, arg2;
    int *code, *reg, *ram;

    int ram_size;
};

void procCtor(Proc* prc, int* code);
void procDtor(Proc* prc);

void runProc(int* code, FILE* fin, FILE* fout);
void analArg(Proc* prc, int** arg);

#endif // PROCESSOR_H
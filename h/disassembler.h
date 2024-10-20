#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include <stdio.h>

#include <common.h>

struct Disasm
{
    int *code;
    int ip;
};

void printRegName(RegEnum reg_num, FILE* fout);

void disasmCtor(Disasm* dis);
void disasmDtor(Disasm* dis);

void printComplexArg(Disasm* dis, FILE* fout);

void runDisasm(FILE* fin, FILE* fout);

#endif // DISASSEMBLER_H
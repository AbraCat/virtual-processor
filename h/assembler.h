#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

#include <common.h>

struct Label
{
    int adr;
    char* name;
};
struct LabelArray
{
    struct Label* labels;
    int free, max_labels;
};

struct FixupElem
{
    int ip;
    char* name;
};
struct FixupTable
{
    FixupElem* table;
    int n_fixups, max_elems;
};

struct Asm
{
    char *str_code, *str_cmd, *str_arg1, *str_arg2, *label, *chr1;
    int *code;
    int ip, str_code_pos, pos_incr, n_args, arg1, arg2;
    LabelArray la;
    FixupTable ft;

    int buffer_size, code_size;
};


void runAsm(FILE* fin, FILE* fout);
void getRegNum(char* str_name, int* num);
void getArg(Asm*);

int fileSize(FILE *file, long *siz);
int readFile(FILE* file, char** bufptr);
void clearComments(char* str);


void asmCtor(Asm* ase);
void asmDtor(Asm* ase);

void initLabel(Label* label);
void initLabelArray(LabelArray* la);
void labelDtor(Label* label);
void labelArrayDtor(LabelArray* la);

void insertLabel(LabelArray* la, int adr, char* name);
void getLabelAdr(LabelArray* la, char* name, int* adr);


void initFixupElem(FixupElem* fe);
void initFixupTable(FixupTable* ft);
void fixupElemDtor(FixupElem* fe);
void fixupTableDtor(FixupTable* ft);


void addFixup(FixupTable* ft, int ip, char* name);
void fixup(int* code, FixupTable* ft, LabelArray* la);


#endif // ASSEMBLER_H
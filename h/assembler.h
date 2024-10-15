#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

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


void runAsm(FILE* fin, FILE* fout);


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
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>

#include <common.h>
#include <error.h>

struct Label
{
    int adr;
    char* name;
};
struct LabelArray
{
    struct Label* labels;
    int n_labels, max_labels;
    char* name_buf;
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
    char* name_buf;
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

ErrEnum runAsm(FILE* fin, FILE* fout);
void getRegNum(char* str_name, int* num);
ErrEnum getArg(Asm*);
void clearComments(char* str);

ErrEnum asmCtor(Asm* ase);
void asmDtor(Asm* ase);

void labelCtor(Label* label);
ErrEnum labelArrayCtor(LabelArray* la);
void labelArrayDtor(LabelArray* la);

void addLabel(LabelArray* la, int adr, char* name);
void getLabelAdr(LabelArray* la, char* name, int* adr);

void fixupElemCtor(FixupElem* fe);
ErrEnum fixupTableCtor(FixupTable* ft);
void fixupTableDtor(FixupTable* ft);

void addFixup(FixupTable* ft, int ip, char* name);
void fixup(int* code, FixupTable* ft, LabelArray* la);


#endif // ASSEMBLER_H
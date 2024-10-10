#ifndef SPU_IO
#define SPU_IO

#include <stdio.h>

int readCode(FILE* fin, int* code);
int writeCode(FILE* fout, int* code);

#endif // SPU_IO
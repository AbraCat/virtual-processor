#ifndef SPU_IO
#define SPU_IO

#include <stdio.h>

int readCode(FILE* fin, int* code, int max_cmds);
int writeCode(FILE* fout, int* code, int max_cmds);

#endif // SPU_IO
#include <stdio.h>

#include <processor.h>

int main()
{
    const int MAX_CMDS  = 100;

    FILE *fin = fopen("txt/code.txt", "rb"), *fout = fopen("txt/output.txt", "w");

    int code[MAX_CMDS] = {};
    fread(code, sizeof(int), MAX_CMDS, fin);
    runProc(code, stdin, fout);

    fclose(fin);
    fclose(fout);
    return 0;
}
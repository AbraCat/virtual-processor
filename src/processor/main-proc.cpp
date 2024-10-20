#include <stdio.h>

#include <processor.h>
#include <error.h>

int main()
{
    FILE *fcode = fopen("txt/code.txt", "rb"), *fout = fopen("txt/output.txt", "w");
    if (fcode == NULL || fout == NULL)
        handleErr(ERR_FILE);

    handleErr(runProc(fcode, stdin, fout));

    fclose(fcode);
    fclose(fout);
    return 0;
}
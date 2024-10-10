#include <assert.h>

#include <spuio.h>
#include <cmd.h>

int readCode(FILE* fin, int* code)
{
    assert(fin != NULL);

    for (int ip = 0; fscanf(fin, "%d", code + ip) != EOF; ++ip)
    {
        if (code[ip] == CMD_HLT) break;
    }

    fclose(fin);
    return 0;
}

int writeCode(FILE* fout, int* code)
{
    assert(fout != NULL);

    for (int ip = 0; 1; ++ip)
    {
        fprintf(fout, "%d ", code[ip]);
        if (code[ip] == CMD_HLT) break;
    }

    fclose(fout);
    return 0;
}
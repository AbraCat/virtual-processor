#include <assert.h>

#include <spuio.h>
#include <cmd.h>

int readCode(FILE* fin, int* code, int max_cmds)
{
    assert(fin != NULL);

    int ip = 0;
    for (ip = 0; ip < max_cmds - 1 && fscanf(fin, "%d", code + ip) != EOF; ++ip);
    code[ip] = CMD_END;

    fclose(fin);
    return 0;
}

int writeCode(FILE* fout, int* code, int max_cmds)
{
    assert(fout != NULL);

    for (int ip = 0; ip < max_cmds && code[ip] != CMD_END; ++ip)
    {
        fprintf(fout, "%d ", code[ip]);
    }

    fclose(fout);
    return 0;
}
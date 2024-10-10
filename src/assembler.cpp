#include <string.h>
#include <assert.h>

#include <assembler.h>
#include <cmd.h>
#include <spuio.h>

void runAsm(FILE* fin, FILE* fout)
{
    #define ASM_CASE(command)               \
        if (strcmp(str_cmd, #command) == 0) \
        {                                   \
            code[ip++] = CMD_ ## command;   \
            continue;                       \
        }

    assert(fin != NULL && fout != NULL);

    char str_cmd[50] = "";
    int code[100] = {};
    int ip = 0;
    
    while (1)
    {
        fscanf(fin, "%s", str_cmd);
        
        if (strcmp(str_cmd, "HLT") == 0)
        {
            code[ip++] = CMD_HLT;
            break;
        }
        if (strcmp(str_cmd, "PUSH") == 0)
        {
            code[ip++] = CMD_PUSH;
            int arg = 0;
            fscanf(fin, "%d", &arg);
            code[ip++] = arg;
            continue;
        }
        ASM_CASE(IN)
        ASM_CASE(OUT)
        ASM_CASE(ADD)
        ASM_CASE(SUB)
        ASM_CASE(MUL)
        ASM_CASE(DIV)
        ASM_CASE(DUMP)

        ++ip;
    }

    writeCode(fout, code);

    #undef ASM_CASE
}
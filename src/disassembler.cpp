#include <assert.h>
#include <string.h>

#include <disassembler.h>
#include <cmd.h>
#include <spuio.h>

void runDisasm(FILE* fin, FILE* fout)
{
    #define DISASM_CASE(command)    \
        case CMD_ ## command:       \
        fputs(#command "\n", fout); \
        break;

    assert(fin != NULL && fout != NULL);

    int code[100] = {};
    readCode(fin, code);
    int ip = 0;
    
    while (1)
    {
        switch (code[ip++])
        {
            case CMD_HLT:
                fputs("HLT", fout);
                return;
            case CMD_PUSH:
                fprintf(fout, "PUSH %d\n", code[ip++]);
                break;
            DISASM_CASE(IN)
            DISASM_CASE(OUT)
            DISASM_CASE(ADD)
            DISASM_CASE(SUB)
            DISASM_CASE(MUL)
            DISASM_CASE(DIV)
            DISASM_CASE(DUMP)
            default:
                break;
        }
    }

    #undef DISASM_CASE
}
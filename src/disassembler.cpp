#include <assert.h>
#include <string.h>

#include <disassembler.h>
#include <cmd.h>
#include <spuio.h>

void runDisasm(FILE* fin, FILE* fout)
{
    #define MAX_CMDS 100

    #define DISASM_CASE(command)        \
        case CMD_ ## command:           \
            fputs(#command "\n", fout); \
            break;

    #define DISASM_CASE_ARG(command)        \
        case CMD_ ## command:               \
            fprintf(fout, #command " %d\n", \
            code[ip++]);                    \
            break;

    #define DISASM_CASE_COMPLEX_ARG \
        case CMD ## COMMAND: \
            aaa \
            break;

    assert(fin != NULL && fout != NULL);

    int code[MAX_CMDS] = {};
    readCode(fin, code, MAX_CMDS);
    int ip = 0;
    
    while (1)
    {
        if (ip >= MAX_CMDS)
            return;
        switch (code[ip++])
        {
            case CMD_END:
                return;
            DISASM_CASE(HLT)
            DISASM_CASE(IN)
            DISASM_CASE(OUT)
            DISASM_CASE(ADD)
            DISASM_CASE(SUB)
            DISASM_CASE(MUL)
            DISASM_CASE(DIV)
            DISASM_CASE(DUMP)
            DISASM_CASE(RET)
            DISASM_CASE(DRAW)

            DISASM_CASE_ARG(JMP)
            DISASM_CASE_ARG(JB)
            DISASM_CASE_ARG(JBE)
            DISASM_CASE_ARG(JA)
            DISASM_CASE_ARG(JAE)
            DISASM_CASE_ARG(JE)
            DISASM_CASE_ARG(JNE)
            DISASM_CASE_ARG(CALL)

            // add push/pop
            
            default:
                break;
        }
    }

    #undef DISASM_CASE
    #undef DISASM_CASE_ARG
}
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#include <error.h>

const int n_regs = 4, MAX_CMDS = 200;

enum RegEnum
{
    INVAL_REG = -1,
    AX = 0,
    BX = 1,
    CX = 2,
    DX = 3,
};

enum CmdCode
{
    CMD_END   = 31, // used to mark end of code
    CMD_HLT   = 0,
    CMD_IN    = 1,
    CMD_OUT   = 2,
    CMD_PUSH  = 3,
    CMD_POP   = 4,
    CMD_ADD   = 5,
    CMD_SUB   = 6,
    CMD_MUL   = 7,
    CMD_DIV   = 8,
    CMD_DUMP  = 9,
    CMD_JMP   = 10,
    CMD_JB    = 11,
    CMD_JBE   = 12,
    CMD_JA    = 13,
    CMD_JAE   = 14,
    CMD_JE    = 15,
    CMD_JNE   = 16,
    CMD_CALL  = 17,
    CMD_RET   = 18,
    CMD_DRAW  = 19,
};

ErrEnum printRegName(RegEnum reg_num, FILE* fout);

#endif // COMMON_H
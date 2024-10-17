





enum CmdCode
{
    CMD_END   = 31, // used to mark end of code[] array, shouldn't be printed to file
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
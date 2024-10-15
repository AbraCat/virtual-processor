




enum CmdCode
{
    CMD_END   = -2, // used to mark end of code[] array, shouldn't be printed to file
    CMD_HLT   = -1,
    CMD_IN    = 0,
    CMD_OUT   = 1,
    CMD_PUSH  = 2,
    CMD_ADD   = 3,
    CMD_SUB   = 4,
    CMD_MUL   = 5,
    CMD_DIV   = 6,
    CMD_DUMP  = 7,
    CMD_PUSHR = 8,
    CMD_POP   = 9,
    CMD_JMP   = 10,
    CMD_JB    = 11,
    CMD_JBE   = 12,
    CMD_JA    = 13,
    CMD_JAE   = 14,
    CMD_JE    = 15,
    CMD_JNE   = 16,
    CMD_CALL  = 17,
    CMD_RET   = 18,
};
#ifndef ERROR_H
#define ERROR_H

// #define MY_NDEBUG

#define handleErr(err_num) handleErrFn(err_num, __FILE__, __LINE__, __FUNCTION__)
    
#define returnErr(expr)     \
{                           \
    ErrEnum err_num = expr; \
    if (err_num)            \
        return err_num;     \
}

#ifdef MY_NDEBUG
#define myAssert(expr)
#else
#define myAssert(expr) myAssertFn(expr, #expr, __FILE__, __LINE__, __FUNCTION__)
#endif


enum ErrEnum
{
    OK = 0,

    // stack
    ERR_ASSERT,
    ERR_STACK_UNDERFLOW,
    ERR_NULL_STACK,
    ERR_BAD_SIZE,
    ERR_NOMEM,
    ERR_POISON,
    ERR_BAD_CANARY,
    ERR_BAD_HASH,

    ERR_FILE,
    ERR_MEM,
    ERR_IO,
    ERR_BRACKET,
    ERR_CMD_ARG_FMT,
    ERR_INSTR_ARG_FMT,
    ERR_OPT_ARG_FMT,
    ERR_REG_NAME,
    ERR_INVAL_CMD,
    ERR_INVAL_INSTR,
    ERR_INVAL_OPT,
    ERR_IP_BOUNDARY,
};

struct ErrDescr
{
    ErrEnum num;
    char* descr;
};

void myAssertFn(int expr, const char* str_expr, const char* file, int line, const char* function);
void getErrDescr(ErrEnum num, char** descr);
void handleErrFn(ErrEnum num, const char* file, int line, const char* function);

#endif // ERROR_H
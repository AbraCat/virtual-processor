#include <stdio.h>
#include <stdlib.h>

#include <error.h>
#include <colors.h>

const int n_errs = 24;
ErrDescr err_arr[] = {{ERR_OK, "No error"}, 

{ERR_ASSERT, "ERR_ASSERT"}, 
{ERR_STACK_UNDERFLOW, "Stack underflow"}, 
{ERR_NULL_STACK, "ERR_NULL_STACK"},
{ERR_BAD_SIZE, "ERR_BAD_SIZE"},
{ERR_POISON, "ERR_POISON"},
{ERR_BAD_CANARY, "Bad canary"},
{ERR_BAD_HASH, "Bad hash"},

{ERR_FILE, "file error"},
{ERR_IO, "Input/output error"},
{ERR_MEM, "Couldn't allocate memory"},

{ERR_INVAL_OPT, "Invalid command line option"},
{ERR_INVAL_CMD, "Invalid assembly command"},
{ERR_INVAL_INSTR, "Invalid binary instruction"},

{ERR_OPT_ARG_FMT, "Invalid command line argument format"},
{ERR_CMD_ARG_FMT, "Invalid assembly argument format"},
{ERR_INSTR_ARG_FMT, "Invalid binary argument format"},
{ERR_CMD_NO_ARG, "Missing assembly argument"},
{ERR_BRACKET, "Invalid bracket sequence"},

{ERR_INVAL_REG_NAME, "Invalid register name"},
{ERR_INVAL_REG_NUM, "Invalid register number"},
{ERR_INVAL_LABEL, "Invalid label"},

{ERR_IP_BOUND, "Ip out of bounds"},
{ERR_INPUT, "Invalid input"}};

void myAssertFn(int expr, const char* str_expr, const char* file, int line, const char* function)
{
    if (expr)
        return;

    printf("%sAssertion failed: %s at %s:%d (%s)%s\n", RED_STR, str_expr, file, line, function, DEFAULT_STR);
    exit(expr);
}

void getErrDescr(ErrEnum num, const char** descr)
{
    for (int i = 0; i < n_errs; ++i)
    {
        if (err_arr[i].num == num)
        {
            *descr = err_arr[i].descr;
            return;
        }
    }
    *descr = NULL;
}

void handleErrFn(ErrEnum num, const char* file, int line, const char* function)
{
    if (num == ERR_OK)
        return;

    const char* descr = NULL;
    getErrDescr(num, &descr);

    if (descr == NULL)
        printf("%sUnknown error (%d) at %s:%d (%s)%s\n", RED_STR, (int)num, file, line, function, DEFAULT_STR);
    else
        printf("%sError: %s at %s:%d (%s)%s\n", RED_STR, descr, file, line, function, DEFAULT_STR);

    exit(num);
}
#include <stdio.h>
#include <stdlib.h>

#include <error.h>
#include <colors.h>

const int n_errs = 7;
ErrDescr err_arr[] = {{OK, "ok"}, {ERR_FILE, "ERR_FILE"}, {ERR_MEM, "MEM"}, {ERR_IO, "IO"}, {ERR_BRACKET, "BRACKET"}, 
{ERR_CMD_ARG_FMT, "CMD_ARG_FMT"}, {ERR_REG_NAME, "REG_NAME"}};

void myAssertFn(int expr, const char* str_expr, const char* file, int line, const char* function)
{
    if (expr)
        return;

    printf("%sAssertion failed: %s at %s:%d (%s)%s\n", RED_STR, str_expr, file, line, function, DEFAULT_STR);
    exit(expr);
}

void getErrDescr(ErrEnum num, char** descr)
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
    if (num == OK)
        return;

    char* descr = NULL;
    getErrDescr(num, &descr);

    if (descr == NULL)
        printf("%sUnknown error (%d) at %s:%d (%s)%s\n", RED_STR, (int)num, file, line, function, DEFAULT_STR);
    else
        printf("%sError: %s at %s:%d (%s)%s\n", RED_STR, descr, file, line, function, DEFAULT_STR);

    exit(num);
}
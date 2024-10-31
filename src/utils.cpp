#include <stdlib.h>
#include <ctype.h>

#include <utils.h>

ErrEnum fileSize(FILE *file, long *siz)
{
    myAssert(file != NULL && siz != NULL);

    if (fseek(file, 0L, SEEK_END))
    {
        *siz = -1L;
        return ERR_FILE;
    }

    *siz = ftell(file);
    if (*siz == -1L) return ERR_FILE;

    if (fseek(file, 0L, SEEK_SET)) return ERR_FILE;

    return ERR_OK;
}

ErrEnum strToPosInt(const char* str, int* ans)
{
    const int base = 10;
    *ans = 0;
    int digit = 0;

    if (isspace(*str) || *str == '\0') return ERR_STR_TO_INT;
    for (; !isspace(*str) && *str != '\0'; ++str)
    {
        digit = *str - '0';
        if (digit < 0 || digit > 9) return ERR_STR_TO_INT;
        *ans = *ans * base + digit;
    }

    return ERR_OK;
}
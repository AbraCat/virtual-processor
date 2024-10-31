#include <ctype.h>

#include <str.h>

void strlenToSpace(const char* str, int* len)
{
    *len = 0;
    while (!isspace(str[*len]) && str[*len] != '\0') ++*len;
}

void strncpyToSpace(char* dest, const char* src, int count)
{
    for (int i = 0; i < count && !isspace(src[i]) && src[i] != '\0'; ++i)
        dest[i] = src[i];
}

int strcmpToSpace(const char* lft, const char* rgt)
{
    while (!isspace(*lft) && *lft != '\0' && *lft == *rgt)
    {
        ++lft;
        ++rgt;
    }
    return *lft - *rgt;
}
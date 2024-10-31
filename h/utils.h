#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#include <error.h>

static int myMin(int a, int b)
{
    return a < b ? a : b;
}

static int myMax(int a, int b)
{
    return a > b ? a : b;
}

ErrEnum fileSize(FILE *file, long *siz);
ErrEnum strToPosInt(const char* str, int* ans);

#endif // UTILS_H
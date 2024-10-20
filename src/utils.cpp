#include <stdlib.h>

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

    if (*siz == -1L)
        return ERR_FILE;
    if (fseek(file, 0L, SEEK_SET))
        return ERR_FILE;

    return OK;
}

ErrEnum readFile(FILE* file, char** bufptr)
{
    myAssert(file != NULL && bufptr != NULL);

    long siz = 0;
    ErrEnum error = fileSize(file, &siz);

    if (error != OK)
    {
        *bufptr = NULL;
        return error;
    }

    char* buf = *bufptr = (char*)calloc(siz + 2, sizeof(char));
    if (buf == NULL)
        return ERR_MEM;

    int n_read = fread(buf, sizeof(char), siz, file);
    if (ferror(file))
    {
        free(buf);
        *bufptr = NULL;
        return ERR_IO;
    }

    if (buf[n_read - 1] != '\n')
        buf[n_read++] = '\n';
    buf[n_read] = '\0';
    return OK;
}
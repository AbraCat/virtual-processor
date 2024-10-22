#include <common.h>

ErrEnum printRegName(RegEnum reg_num, FILE* fout)
{
    myAssert(fout != NULL);

    #define REG_CASE(enum_elem)                 \
        case enum_elem:                         \
            if (fputs(#enum_elem, fout) == EOF) \
                return ERR_IO;                  \
            break;

    switch (reg_num)
    {
        REG_CASE(AX)
        REG_CASE(BX)
        REG_CASE(CX)
        REG_CASE(DX)
        REG_CASE(INVAL_REG)

        default:
            if (fputs("INVAL_REG", fout) == EOF)
                return ERR_IO;
            break;
    }

    return OK;

    #undef REG_CASE
}
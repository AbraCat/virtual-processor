#include <common.h>

ErrEnum printRegName(RegEnum reg_num, FILE* fout)
{
    myAssert(fout != NULL);
    if (reg_num < 0 || reg_num >= n_regs)
        return ERR_INVAL_REG_NUM;

    fprintf(fout, "%cX", 'A' + reg_num);
    return OK;
}
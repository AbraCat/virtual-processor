#include <stdio.h>

#include <options.h>
#include <assembler.h>
#include <disassembler.h>

int mainAsm();
int mainDisasm();

int main(int argc, const char* argv[])
{

    static const int n_opts = 1;
    Option opts[] = {{"-d", "--decompile"}};

    parseOpts(argc, argv, opts, n_opts);
    
    // testOpts(opts, n_opts);
    // return 0;

    if (optByName(opts, n_opts, "-d")->trig)
        return mainDisasm();

    return mainAsm();
}

int mainAsm()
{
    FILE *fin = fopen("txt/asm.txt", "r"), *fout = fopen("txt/code.txt", "wb");

    runAsm(fin, fout);

    fclose(fin);
    fclose(fout);
    return 0;
}

int mainDisasm()
{
    FILE *fin = fopen("txt/code.txt", "rb"), *fout = fopen("txt/asm.txt", "w");

    runDisasm(fin, fout);

    fclose(fin);
    fclose(fout);
    return 0;
}
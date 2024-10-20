#include <stdio.h>

#include <options.h>
#include <processor.h>
#include <assembler.h>
#include <disassembler.h>

int mainProc();
int mainAsm();
int mainDisasm();

int main(int argc, const char* argv[])
{

    static const int n_opts = 3;
    Option opts[] = {{"-r", "--run"}, {"-c", "--compile"}, {"-d", "--decompile"}};

    parseOpts(argc, argv, opts, n_opts);
    
    // testOpts(opts, n_opts);
    // return 0;

    if (optByName(opts, n_opts, "-d")->trig)
    {
        return mainDisasm();
    }

    if (optByName(opts, n_opts, "-c")->trig)
    {
        mainAsm();
        if (optByName(opts, n_opts, "-r")->trig)
        {
            mainProc();
        }
        return 0;
    }

    return mainProc();
}

int mainProc()
{
    const int MAX_CMDS  = 100;

    FILE *fin = fopen("txt/code.txt", "rb"), *fout = fopen("txt/output.txt", "w");

    int code[MAX_CMDS] = {};
    fread(code, sizeof(int), MAX_CMDS, fin);
    runProc(code, stdin, fout);

    fclose(fin);
    fclose(fout);
    return 0;
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
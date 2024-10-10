#ifndef OPTIONS_H
#define OPTIONS_H

struct Option
{
    const char *sh_name, *l_name;

    const char *str_arg;
    int int_arg;
    int trig;
    
    int l_name_len;
};

int optcmp(struct Option* opt1, struct Option* opt2);
int parseOpts(int argc, const char* argv[], struct Option* opts, int n_opts);
struct Option* optByName(struct Option* opts, int n_opts, const char* sh_name);
void testOpts(struct Option* opts, int n_opts);

#endif //OPTIONS_H
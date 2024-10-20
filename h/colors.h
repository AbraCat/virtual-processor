#ifndef COLORS_H
#define COLORS_H

static const char BLACK_STR[]   = "\x1b[30m";
static const char RED_STR[]     = "\x1b[31m";
static const char GREEN_STR[]   = "\x1b[32m";
static const char YELLOW_STR[]  = "\x1b[33m";
static const char BLUE_STR[]    = "\x1b[34m";
static const char MAGENTA_STR[] = "\x1b[35m";
static const char CYAN_STR[]    = "\x1b[36m";
static const char WHITE_STR[]   = "\x1b[37m";
static const char DEFAULT_STR[] = "\x1b[39m";

enum Colors
{
    BLACK   = 0,
    RED     = 1,
    GREEN   = 2,
    YELLOW  = 3,
    BLUE    = 4,
    MAGENTA = 5,
    CYAN    = 6,
    WHITE   = 7,
    DEFAULT = 9,
};

#endif // COLORS_H
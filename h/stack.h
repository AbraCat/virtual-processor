#ifndef STACK_H
#define STACK_H

#define ST_NDEBUG

#ifdef ST_NDEBUG
#undef ST_USE_CANARY
#undef ST_USE_HASH
#else
#define ST_USE_CANARY
#define ST_USE_HASH
#endif // ST_NDEBUG



#define handleErr(expr) handleErrFn(expr, __FILE__, __LINE__, __FUNCTION__)
#define stDump(file, st) stDumpFn(file, st, __FILE__, __LINE__, __FUNCTION__)

#define returnErr(expr)      \
{                            \
    stErrCode error = expr;  \
    if (error) return error; \
}



#ifdef ST_NDEBUG

#define ST_ON_RELEASE(...) __VA_ARGS__
#define ST_ON_DEBUG(...) ;
#define stAssert(expr) ;
#define stCtor(st, capacity) stCtorNDebug(st, capacity)

#else

#define ST_ON_RELEASE(...) ;
#define ST_ON_DEBUG(...) __VA_ARGS__
#define stAssert(expr) stAssertFn(expr, #expr, __FILE__, __LINE__, __FUNCTION__)
#define stCtor(st, capacity) stCtorDebug(st, capacity, __FILE__, __LINE__, __FUNCTION__)

#endif //ST_NDEBUG



#ifdef ST_USE_CANARY

#define ST_ON_CANARY(...) __VA_ARGS__
#define ST_ON_NO_CANARY(...) ;

#else

#define ST_ON_CANARY(...) ;
#define ST_ON_NO_CANARY(...) __VA_ARGS__

#endif //ST_USE_CANARY



#ifdef ST_USE_HASH
#define ST_ON_HASH(...)  __VA_ARGS__
#else
#define ST_ON_HASH(...) ;
#endif // ST_USE_HASH

#define DEBUG_DATA ST_ON_DEBUG(DebugData debug;)



typedef int StackElem;

struct DebugData
{
    const char *file_born, *func_born;
    int line_born;
};

struct Stack
{
    ST_ON_CANARY
    (
        StackElem left_st_canary;
    )

    ST_ON_HASH
    (
        int st_hash, data_hash;
    )

    DEBUG_DATA

    int size;
    int capacity;
    StackElem* data;

    ST_ON_CANARY
    (
        StackElem right_st_canary;
    )
};

enum stErrCode
{
    ERR_OK,
    ERR_ASSERT,
    ERR_STACK_UNDERFLOW,
    ERR_NULL_STACK,
    ERR_BAD_SIZE,
    ERR_NOMEM,
    ERR_POISON,
    ERR_BAD_CANARY,
    ERR_BAD_HASH
};



const char* stStrError(stErrCode error);
void handleErrFn(stErrCode error, const char* file, int line, const char* func);
void stAssertFn(int expr, const char* str_expr, const char* file, int line, const char* func);

stErrCode stCtorNDebug(Stack* st, int capacity);
stErrCode stCtorDebug(Stack* st, int capacity, const char* file_name, int line_born, const char* func_born);

void stDtor(Stack* st);

stErrCode stPush(Stack* st, StackElem elem);
stErrCode stPop(Stack* st, StackElem* elem);

stErrCode resize(Stack* st, int new_capacity);

stErrCode stErr(Stack* st);
void stDumpFn(FILE* file, Stack* st, const char* file_name, int line, const char* func_name);

#endif //STACK_H
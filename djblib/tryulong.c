#include <limits.h>

#if ULONG_MAX > 4294967295
#define WORDSIZE 64
#else
#define WORDSIZE 32
#endif

WORDSIZE

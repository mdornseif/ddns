#include <sys/types.h>
#include <unistd.h>
#include "seek.h"

#ifdef SEEK_SET
#define SET SEEK_SET
#else
#define SET 0 /* sigh */
#endif

int seek_set(int fd,seek_pos pos)
{ if (lseek(fd,(off_t) pos,SET) == -1) return -1; return 0; }

#include "byte.h"

char* byte_dup(char *source,unsigned int len) {
  char *tmp=(char *)malloc(len);
  if (!tmp) return 0;
  byte_copy(tmp,len,source);
  return tmp;
}

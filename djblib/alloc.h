#ifndef ALLOC_H
#define ALLOC_H

extern /*@null@*//*@out@*/char *alloc(unsigned int);
extern void alloc_free(void *);
extern int alloc_re(char **x,unsigned int m,unsigned int n);

extern char* byte_dup(char *source,unsigned int len);

#define str_dup(s) (byte_dup(s,str_len(s)+1))

#endif

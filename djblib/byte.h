#ifndef BYTE_H
#define BYTE_H

unsigned int byte_chr(const char* s,register unsigned int n,int c);
unsigned int byte_rchr(const char* s,register unsigned int n,int c);
void byte_copy(register void* to,
	       register unsigned int n,
	       register const void* from);
void byte_copyr(register void* to,
		register unsigned int n,
		register const void* from);
int byte_diff(register const void*,
	      register unsigned int,
	      register const void*);
void byte_zero(register void*,
	       register unsigned int);

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif

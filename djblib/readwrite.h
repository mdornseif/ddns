#ifndef READWRITE_H
#define READWRITE_H

extern int read(int fd,void *buf,unsigned int count);
extern int write(int fd,const void *buf,unsigned int count);
extern int close(int fd);

#endif

/*  $Id: scan_xlong.c,v 1.1 2000/05/02 04:34:49 drt Exp $

     from djblib 0.16                                                                                         
     20000331                                                                                            
     Copyright 2000                                                                                      
     SuperScript Technology, Inc.  
     
     $Log: scan_xlong.c,v $
     Revision 1.1  2000/05/02 04:34:49  drt
     importet from didentd

     Revision 1.2  2000/04/28 13:03:05  drt
     Compile fixes

     Revision 1.1.1.1  2000/04/12 16:07:11  drt
*/

#include "scan.h"

static char rcsid[] = "$Id: scan_xlong.c,v 1.1 2000/05/02 04:34:49 drt Exp $";

unsigned int scan_xlong(char *s,unsigned long *u)
{
  unsigned int pos; unsigned long result; unsigned long c;
  pos = 0; result = 0;
  while (((c = (unsigned long) (unsigned char) (s[pos] - '0')) < 10)
         ||(((c = (unsigned long) (unsigned char) (s[pos] - 'a')) < 6)
	    &&(c = c + 10))
	 ||(((c = (unsigned long) (unsigned char) (s[pos] - 'A')) < 6)
	    &&(c = c + 10))
	 ) /* XXX: this gets the job done */
    { result = result * 16 + c; ++pos; }
  *u = result; return pos;
}

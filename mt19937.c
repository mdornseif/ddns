// $Id: mt19937.c,v 1.1 2000/04/30 15:59:26 drt Exp $
//
// This is the ``Mersenne Twister'' random number generator MT19937, which
// generates pseudorandom integers uniformly distributed in 0..(2^32 - 1)
// starting from any odd seed in 0..(2^32 - 1).  This version is a recode
// by Shawn Cokus (Cokus@math.washington.edu) on March 8, 1998 of a version by
// Takuji Nishimura (who had suggestions from Topher Cooper and Marc Rieffel in
// July-August 1997).
//
// Effectiveness of the recoding (on Goedel2.math.washington.edu, a DEC Alpha
// running OSF/1) using GCC -O3 as a compiler: before recoding: 51.6 sec. to
// generate 300 million random numbers; after recoding: 24.0 sec. for the same
// (i.e., 46.5% of original time), so speed is now about 12.5 million random
// number generations per second on this machine.
//
// D. R. Tzeck <drt@ailis.de> added some function to generate blocks
// of random data.  Tests indivate that this ist about 55% faster on a
// PII for 1024 byte blocks than the inline function randomMT(void). I
// guess blockMT could be improved further for 64Bit CPUs.
//
// According to the URL <http://www.math.keio.ac.jp/~matumoto/emt.html>
// (and paraphrasing a bit in places), the Mersenne Twister is ``designed
// with consideration of the flaws of various existing generators,'' has
// a period of 2^19937 - 1, gives a sequence that is 623-dimensionally
// equidistributed, and ``has passed many stringent tests, including the
// die-hard test of G. Marsaglia and the load test of P. Hellekalek and
// S. Wegenkittl.''  It is efficient in memory usage (typically using 2506
// to 5012 bytes of static data, depending on data type sizes, and the code
// is quite short as well).  It generates random numbers in batches of 624
// at a time, so the caching and pipelining of modern systems is exploited.
// It is also divide- and mod-free.
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation (either version 2 of the License or, at your
// option, any later version).  This library is distributed in the hope that
// it will be useful, but WITHOUT ANY WARRANTY, without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU Library General Public License for more details.  You should have
// received a copy of the GNU Library General Public License along with this
// library; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA 02111-1307, USA.
//
// The code as Shawn received it included the following notice:
//
//   Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.  When
//   you use this, send an e-mail to <matumoto@math.keio.ac.jp> with
//   an appropriate reference to your work.
//
// It would be nice to CC: <Cokus@math.washington.edu> when you write.
//
// $Log: mt19937.c,v $
// Revision 1.1  2000/04/30 15:59:26  drt
// cleand up usage of djb stuff
//
// Revision 1.1.1.1  2000/04/16 17:46:05  drt
// initial ddns version
//
// Revision 1.3  2000/04/11 08:58:53  drt
// swapoff 0.01b
//
// Revision 1.2  2000/02/16 13:31:57  drt
// blockMT added to produce large blocks of random data
//
// Revision 1.1  2000/02/16 10:49:32  drt
// Initial revision
//

#ifdef TEST
#include <stdio.h>
#include <stdlib.h>
#endif

//
// uint32 must be an unsigned integer type capable of holding at least 32
// bits; exactly 32 should be fastest, but 64 is better on an Alpha with
// GCC at -O3 optimization so try your options and see what's best for you
//

#include "uint32.h"

#define N              (624)                 // length of state vector
#define M              (397)                 // a period parameter
#define K              (0x9908B0DFU)         // a magic constant
#define hiBit(u)       ((u) & 0x80000000U)   // mask all but highest   bit of u
#define loBit(u)       ((u) & 0x00000001U)   // mask all but lowest    bit of u
#define loBits(u)      ((u) & 0x7FFFFFFFU)   // mask     the highest   bit of u
#define mixBits(u, v)  (hiBit(u)|loBits(v))  // move hi bit of u to hi bit of v

static uint32   state[N+1];     // state vector + 1 extra to not violate ANSI C
static uint32   *next;          // next random value is computed from here
static int      left = -1;      // can *next++ this many times before reloading


void seedMT(uint32 seed)
 {
    register uint32 x = (seed | 1U) & 0xFFFFFFFFU, *s = state;
    register int    j;

    for(left=0, *s++=x, j=N; --j;
        *s++ = (x*=69069U) & 0xFFFFFFFFU);
 }


void reloadMT(void)
 {
    register uint32 *p0=state, *p2=state+2, *pM=state+M, s0, s1;
    register int    j;

    if(left < -1)
        seedMT(4357U);

    left=N-1, next=state+1;

    for(s0=state[0], s1=state[1], j=N-M+1; --j; s0=s1, s1=*p2++)
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

    for(pM=state, j=M; --j; s0=s1, s1=*p2++)
        *p0++ = *pM++ ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);

    s1=state[0], *p0 = *pM ^ (mixBits(s0, s1) >> 1) ^ (loBit(s1) ? K : 0U);
 }

unsigned long randomMT(void)
 {
    uint32 y;

    if(--left < 0)
      reloadMT();
    
    y  = *next++;
    y ^= (y >> 11);
    y ^= (y <<  7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    return(y ^ (y >> 18));
 }


// Fill a block at mem of len bytes with random Data
void blockMT(void * mem, unsigned int len)
{
  uint32 y;
  unsigned long *p, *end;

  for(p = mem, end = p + len/4; p < end; p++)
    {
      if(--left < 0)
        reloadMT();      

      y  = *next++;
      y ^= (y >> 11);
      y ^= (y <<  7) & 0x9D2C5680U;
      y ^= (y << 15) & 0xEFC60000U;
      *p = y ^ (y >> 18);
    }
}

// XOR a block at mem of len bytes with random Data
void blockMTxor(void * mem, unsigned int len)
{
  uint32 y;
  unsigned long *p, *end;

  for(p = mem, end = p + len/4; p < end; p++)
    {
      if(--left < 0)
        reloadMT();      

      y  = *next++;
      y ^= (y >> 11);
      y ^= (y <<  7) & 0x9D2C5680U;
      y ^= (y << 15) & 0xEFC60000U;
      *p ^= y ^ (y >> 18);
    }
}

#ifdef TEST
void testblock()
{
  long j;
  char m[4096]; 
  
  // generate 128MB of random Data
  for(j=1; j< 256*512; j++)
    {
      blockMT(m, 4096);
    }
}

void testsingle()
{
  long j;
  char m[1024]; 
  
    // generate 128MB (= 32M longs) of random Data
    for(j=1; j< 1024*1024*128; j++)
      {
	randomMT();
      }
}

int main(void)
 {
    long j;
    char m[1024]; 

    // you can seed with any uint32, but the best are odds in 0..(2^32 - 1)

    seedMT(4357U);
    
    testblock();
    testsingle();
 
    return(EXIT_SUCCESS);
 }
#endif

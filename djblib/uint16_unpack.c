#include "uint16.h"

void uint16_unpack(char *s,uint16 *u)
{
  uint16 result;

  result = (unsigned char) s[1];
  result <<= 8;
  result += (unsigned char) s[0];

  *u = result;
}

void uint16_unpack_big(char *s,uint16 *u)
{
  uint16 result;

  result = (unsigned char) s[0];
  result <<= 8;
  result += (unsigned char) s[1];

  *u = result;
}

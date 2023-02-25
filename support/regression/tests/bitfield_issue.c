/* Bitfield issue */
#include <testfwk.h>
#include <stdlib.h>
#include <stdint.h>

__data unsigned char * __data ptemp;
__data unsigned char temp;

struct {
  unsigned char b0 : 1;
  unsigned char b1 : 1;
  unsigned char b2 : 1;
  unsigned char b3 : 1;
  unsigned char b4_7 : 4;
} sb_bf;


void
test(void)
{
  ptemp=&temp;
  sb_bf.b4_7=2;
  ASSERT( sb_bf.b4_7==2 ); //PASS
  *ptemp=0xFF;
  ASSERT( *ptemp == 0xFF ); //PASS
  *ptemp=sb_bf.b4_7;
  ASSERT( *ptemp == 2 ); //PASS
  //ASSERT( *ptemp == 0x2F ); //PASS, but must fail
  *ptemp=(unsigned char) sb_bf.b4_7;
  ASSERT( *ptemp == 2 ); //PASS
  //ASSERT( *ptemp == 0x2F ); //PASS, but must fail
  temp=0xFF;
  ASSERT( temp == 0xFF ); //PASS
  temp=sb_bf.b4_7;
  ASSERT( temp == 2 ); //PASS
}


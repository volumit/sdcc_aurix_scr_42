/*
   bug3166208.c
 */

#include <testfwk.h>

// no need to call this, it generates compiler error:
//   SDCCval.c:1073: expected SPECIFIER, got null-link
volatile char a;

void bug3166208(void)
{
	if ((* (char __xdata *)0xDF53))
	{
		a = 2;
	}
}

// no need to call this, it generates compiler error:
//   SDCCloop.c:339: expected symbol, got value
// __data for *p should not work, because __data is direct addressing mode and unknown during compilation
//unsigned char __data *p;
unsigned char __idata *p;

void bug3150679(void)
{
	while(1)
	{
		*((unsigned char __data *)2) = *p;
	}
}

void
testBug (void)
{
	ASSERT (1);
}

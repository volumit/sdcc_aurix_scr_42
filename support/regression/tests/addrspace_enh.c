/** addrspace.c
*/

#include <testfwk.h>

int mem_id;

void set_mem1(void)
{
  mem_id=1;
}

void set_mem2(void)
{
  mem_id=2;
}

void set_mem3(void)
{
  mem_id=3;
}

void set_const(void)
{
  mem_id=4;
}

#if defined(__SDCC) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Weird issue in pdk14 / pdk15
__addressmod set_mem1 __mem1;
__addressmod set_mem2 __mem2;
__addressmod set_mem3 __mem3;
__addressmod set_const const __cmem4;
#else
#define b
#endif

int a0, a1, a2;
const __cmem4 int constx = 42;
__mem1 volatile int b0_mem1;
__mem2 volatile int x3_mem2;
__mem3 volatile int x4_mem3;
volatile int b0;
volatile int x3;
volatile int x4;
int b2, b1;
int x0,x1;

int f(void)
{
  ASSERT (x0==0);
      switch(x0)
        {
        case 0:
                ASSERT(a0==1);
                ASSERT(b0==1);
                if(a0 == b0)
                {
                    ASSERT (1);
                }
                else
                {
                    ASSERT(0);
                    x0++;
                }
                return(b2);
        default:
                x3+=1;
                return(x1);
        }
}

int faddr(void)
{
  ASSERT (x0==0);
      switch(x0)
        {
        case 0:
                ASSERT(a0==1);
                ASSERT(b0_mem1==1);
                if(a0 == b0_mem1)
                {
                    ASSERT (mem_id==1);
                    ASSERT (1);
                }
                else
                {
                    ASSERT(0);
                    x0++;
                }
                return(b2);
        default:
                x3_mem2+=1;
                ASSERT (mem_id==2);
                return(x1);
        }
}

int res;
void testBug(void)
{
a0=1;
b0=1;
ASSERT(b0==1);
x4=0;
ASSERT (mem_id!=3);
x0=0;
res=f();

a0=constx;
ASSERT(mem_id==4);
ASSERT(a0==42);
a0=1;
b0_mem1=1;
ASSERT(b0_mem1==1);
x4_mem3=0;
ASSERT (mem_id==3);
x0=0;
res=faddr();
}



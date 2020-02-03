#include "tea.h"

void encipher(unsigned long *xl, unsigned long *xr,
              const unsigned long * const key,
              unsigned long iter)
{
    register unsigned long       y=xl[0], z=xr[0], delta=0x9E3779B9, sum=0, n=iter;

    while(n-->0)
    {
        y += (((z << 4) ^ (z >> 5)) + z) ^ (sum + key[sum&3]);
        sum += delta;
        z += (((y << 4) ^ (y >> 5)) + y) ^ (sum + key[sum>>11 & 3]);
    }

    xl[0]=y; xr[0]=z;
}

void decipher(unsigned long *xl, unsigned long *xr,
              const unsigned long * const key,
              unsigned long iter)
{
    register unsigned long       y=xl[0], z=xr[0], delta=0x9E3779B9, sum=delta*iter, n=iter;

    while(n-->0)
    {
        z -= (((y << 4) ^ (y >> 5)) + y) ^ (sum + key[sum>>11 & 3]);
        sum -= delta;
        y -= (((z << 4) ^ (z >> 5)) + z) ^ (sum + key[sum&3]);
    }

    xl[0]=y; xr[0]=z;
}


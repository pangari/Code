#ifndef __TEA_H__
#define __TEA_H__

void encipher(unsigned long *xl, unsigned long *xr,
              const unsigned long * const key,
              unsigned long iter);

void decipher(unsigned long *xl, unsigned long *xr, 
              const unsigned long * const key,
              unsigned long iter);

#endif //__TEA_H__

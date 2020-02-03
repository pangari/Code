#include "lookup3.h"

#include <string.h>
#include <stdlib.h>

#undef min
#undef max

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

/*
--------------------------------------------------------------------
 This works on all machines.  To be useful, it requires
 -- that the key be an array of int's, and
 -- that all your machines have the same endianness, and
 -- that the length be the number of int's in the key

 The function hashword() is identical to hashlittle() on little-endian
 machines, and identical to hashbig() on big-endian machines,
 except that the length has to be measured in ints rather than in
 bytes.  hashlittle() is more complicated than hashword() only because
 hashlittle() has to dance around fitting the key bytes into registers.
--------------------------------------------------------------------
*/
int hashword(int* k, size_t length, int initval)
                         /* the key, an array of int values */
                         /* the length of the key, in ints */
                         /* the previous hash, or an arbitrary value */
{
  int a,b,c;

  /* Set up the internal state */
  a = b = c = 0xdeadbeef + (((int)length)<<2) + initval;

  /*------------------------------------------------- handle most of the key */
  while (length > 3)
  {
    a += k[0];
    b += k[1];
    c += k[2];
    mix(a,b,c);
    length -= 3;
    k += 3;
  }

  /*------------------------------------------- handle the last 3 int's */
  switch(length)                     /* all the case statements fall through */
  {
  case 3 : c+=k[2];
  case 2 : b+=k[1];
  case 1 : a+=k[0];
    final(a,b,c);
  case 0:     /* case 0: nothing left to add */
    break;
  }
  /*------------------------------------------------------ report the result */
  return c;
}

int hashstring(const char* str, const int keySize)
{
    int hash = 0;

    if(str)
    {
        int extraBytes = keySize % sizeof(int);
        hash = hashword((int*)str, keySize / sizeof(int), 536102);
        if(extraBytes)
        {
            int buff = 0;
            memcpy(&buff, &str[keySize-extraBytes], extraBytes);
            hash = hashword((int*)&buff, 1, hash);
        }
    }
    return hash;
}

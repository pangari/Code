#include <stdio.h>

#define  PLUS(td)     (td + 32)
#define  MOINS(td)    (td - 32)
#define  Q1(oct,bit)  (oct << bit)
#define  Q0(oct,bit)  ((oct + (oct << bit)) >> bit)
#define  P1(oct,bit)  ( oct >> bit )
#define  P0(oct,bit)  ( oct - (( oct >> bit ) << bit ))
#define  Max4Oct      469762047L

int DecodeI2( unsigned char *Source);
void To_2_oct(short l, unsigned char *tb);

int Write_Msg_Feed(FILE* file, char *Msg, short Lg, char TypCmpr, int flush);

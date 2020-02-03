#define  PLUS(td)     (td + 32)
#define  MOINS(td)    (td - 32)
#define  Q1(oct,bit)  (oct << bit)
#define  Q0(oct,bit)  ((oct + (oct << bit)) >> bit)
#define  P1(oct,bit)  ( oct >> bit )
#define  P0(oct,bit)  ( oct - (( oct >> bit ) << bit ))
#define  Max4Oct      469762047L

int DecodeI2( unsigned char *Source);

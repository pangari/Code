#include "codage.h"

int DecodeI2( unsigned char *Source)
{
	int EntierC;

	EntierC = Q1((short)MOINS( Source[1] ), 7);
	EntierC += Q0((short)MOINS( Source[0] ), 7);

	return EntierC;
}

void To_2_oct(short l, unsigned char *tb)
{
	tb[1] = (unsigned char)PLUS(P1(l, 7));
	tb[0] = (unsigned char)PLUS(P0(l, 7));
}

int Write_Msg_Feed(FILE* file, char *Msg, short Lg, char TypCmpr, int flush)
{
	char chaine[3];

	if(!file) return 0;

	To_2_oct(Lg+1, (unsigned char *)chaine);
	if (fprintf(file, "%-2.2s%c", chaine, TypCmpr) < 3)
	{
		return 0;
	}
	if(fwrite(Msg,1,Lg,file) < Lg)
	{
		return 0;
	}

	if(flush) fflush(file);

	return 1;
}

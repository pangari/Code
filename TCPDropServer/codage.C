#include "codage.H"

int DecodeI2( unsigned char *Source)
{
	int EntierC;

	EntierC = Q1((short)MOINS( Source[1] ), 7);
	EntierC += Q0((short)MOINS( Source[0] ), 7);

	return EntierC;
}

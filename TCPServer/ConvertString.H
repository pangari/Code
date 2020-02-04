#ifndef __CONVERT_STRING_H__
#define __CONVERT_STRING_H__

#include <string.h>
#include <stdio.h>

int ConvertStringForFullDisplay(char* szIn, char* szOut, int nCountIn);
int ConvertStringForDisplay(char* szIn, char* szOut, int nCountIn);
int ConvertString(char* szIn, char* szOut, int nCountIn);

int PrintStringForFullDisplay(char* szIn, FILE* output, int nCountIn);
int PrintStringForDisplay(char* szIn, FILE* output, int nCountIn);
int PrintString(char* szIn, FILE* output, int nCountIn);

char DecodeCString(char* szCHex, int* pCount);
size_t ConvertCString(char *str);

#endif /* __CONVERT_STRING_H__ */

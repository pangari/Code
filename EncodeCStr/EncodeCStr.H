#pragma once

#include <stdio.h>

unsigned int DecodeCStr(char *str);

unsigned int ConvertBufferAsCStr(char *szOut, const char *szIn, const int nCountIn);
unsigned int PrintBufferAsCStr(FILE *output, const char *szIn, const int nCountIn);

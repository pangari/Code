#include "EncodeCStr.H"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int /*argc*/, char ** /*argv*/)
{
    const char in[] = "Hello\tWorld\n";
    char out[sizeof(in) * 4];

    PrintBufferAsCStr(stdout, in, sizeof(in));

    ConvertBufferAsCStr(out, in, sizeof(in));
    fprintf(stdout, "\n[%s]\n", out);

    DecodeCStr(out);
    fprintf(stdout, "\n[%s]\n", out);

    return 0;
}

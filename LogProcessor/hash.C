#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

static void usage(void);

#undef min
#undef max

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)

#define rot(x,k) (((x)<<(k)) ^ ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
    a -= c;  a ^= rot(c, 4);  c += b; \
    b -= a;  b ^= rot(a, 6);  a += c; \
    c -= b;  c ^= rot(b, 8);  b += a; \
    a -= c;  a ^= rot(c,16);  c += b; \
    b -= a;  b ^= rot(a,19);  a += c; \
    c -= b;  c ^= rot(b, 4);  b += a; \
}
#define final(a,b,c) \
{ \
    c ^= b; c -= rot(b,14); \
    a ^= c; a -= rot(c,11); \
    b ^= a; b -= rot(a,25); \
    c ^= b; c -= rot(b,16); \
    a ^= c; a -= rot(c,4);  \
    b ^= a; b -= rot(a,14); \
    c ^= b; c -= rot(b,24); \
}

/* the key, an array of int values */
/* the length of the key, in ints */
/* the previous hash, or an arbitrary value */
static int hashword(int* k, size_t length, int initval)
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

int hash(const void* key, const int keySize)
{
    int hash = 0;

    if(key)
    {
        const char* str  = (const char*)key;
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

/* ******************************************************************
*   GetHashCode / GetHashCodeEx
*    Returns hash code from a string.
* ******************************************************************
*/
unsigned int GetHashCodeEx(const char * _String, size_t length, unsigned int _iHashKey)
{
    unsigned int uiCode = (unsigned int)hash((char*)_String, (int)length);
    uiCode %= _iHashKey;
    return uiCode;
}
unsigned int GetHashCode(const char * _String, unsigned int _iHashKey)
{
    return GetHashCodeEx(_String, strlen(_String), _iHashKey);
}

int HashMain(int argc, char **argv)
{
	if((argc == 2) && (argv[1][0] == '-') && ((argv[1][1] >= '0') && (argv[1][1] <= '9')))
	{
	    static int		name[1024*1024+1];
	    static char*	subject = (char*)&name[0];
		FILE*			file = stdin;
		char*			ptr;
		unsigned int	hashKey	= strtoul(&argv[1][1], &ptr, 10);
		unsigned int	hashCode;
		unsigned int	hashKeyLen = (ptr ? (ptr - &argv[1][1]) : 0);

        while(feof(file) != EOF)
        {
            if(fgets(subject, sizeof(name)-1, file))
            {
                unsigned int subject_length = (unsigned int)strlen(subject);

                while(subject_length)
                {
                    if((subject[subject_length-1] == '\r' || subject[subject_length-1] == '\n'))
                        subject[--subject_length] = 0;
                    else
                        break;
                }
				hashCode = GetHashCodeEx(subject, subject_length, hashKey);
				fprintf(stdout, "%0*d\t", hashKeyLen, hashCode);
				fwrite(subject, subject_length, 1, stdout);
				fputs("\n", stdout);
            }
            else
            {
                if(file && (file != stdin))  fclose(file);
                file = NULL;
                break;
            }
        }

		return 1;
	}
	else
	{
		usage();
		return 0;
	}
}

static void usage()
{
    fprintf(stderr, "usage: hash -SIZE\n");
    exit(2);
}

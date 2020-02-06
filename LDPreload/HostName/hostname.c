#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/utsname.h>
#include <dlfcn.h>

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *) -1L)
#endif

#define REAL_LIBC RTLD_NEXT

size_t get_file(const char* filename, char** data)
{
    if(!data || !filename)
        return 0;

    size_t dataLen = 0;
    FILE* file = fopen(filename, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        dataLen = (size_t)ftell(file);
        fseek(file, 0, SEEK_SET);

        (*data) = malloc(dataLen+1);
        if(*data)
        {
            dataLen = fread((*data), 1, dataLen, file);
            (*data)[dataLen] = 0;

            while(dataLen)
            {
                if(((*data)[dataLen-1] == '\r' || (*data)[dataLen-1] == '\n' || (*data)[dataLen-1] == ' '))
                    (*data)[--dataLen] = 0;
                else
                    break;
            }
        }

        fclose (file);
    }
    else
    {
        (*data) = strdup(filename);
    }
    return dataLen;
}

int (*real_uname)(struct utsname *buf) = 0;
int (*real_gethostname)(char *name, size_t len) = 0;
char* hostnameBuffer = 0;

static void init (void) __attribute__ ((constructor));
static void init (void)
{
    if(!real_uname)
    {
        real_uname = dlsym(REAL_LIBC, "uname");
        if(!real_uname)
        {
            fprintf(stderr, "missing symbol: uname");
            exit(1);
        }
    }

    if(!real_gethostname)
    {
        real_gethostname = dlsym(REAL_LIBC, "gethostname");
        if(!real_gethostname)
        {
            fprintf(stderr, "missing symbol: gethostname");
            exit(1);
        }
    }

    if(!hostnameBuffer)
        get_file(getenv("REAL_HOST"), &hostnameBuffer);
}
static int do_uname(struct utsname *buf, int (*uname_proc)(struct utsname *buf))
{
    return uname_proc(buf);
}
__attribute__ ((visibility("default"))) int uname(struct utsname *buf)
{
    init(); // we must always call init as constructor may not be called in some cases (such as loading 32bit pthread library)

    int retCode = do_uname(buf, real_uname);
    if(!retCode && hostnameBuffer)
    {
        strncpy(buf->nodename, hostnameBuffer, sizeof(buf->nodename));
        buf->nodename[sizeof(buf->nodename)-1] = 0;
    }
    return retCode;
}

static int do_gethostname(char *name, size_t len, int (*gethostname_proc)(char *name, size_t len))
{
    return gethostname_proc(name, len);
}
__attribute__ ((visibility("default"))) int gethostname(char *name, size_t len)
{
    init(); // we must always call init as constructor may not be called in some cases (such as loading 32bit pthread library)

    int retCode = do_gethostname(name, len, real_gethostname);
    if(!retCode && hostnameBuffer)
    {
        strncpy(name, hostnameBuffer, len);
        name[len-1] = 0;
    }
    return retCode;
}


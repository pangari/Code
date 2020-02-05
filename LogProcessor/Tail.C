#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <sys\stat.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#define BUFFER_SIZE (1024)

int TailMain(int argc, char** argv)
{
    int             ignore_top = 0;
    int             file = 0;
    char*           fileName = NULL;
    char            buffer[BUFFER_SIZE];
 
#ifdef WIN32
    struct  _stat   lastStat;
    struct  _stat   currStat;
    _off_t          newCount, readCount;
#else
    struct  stat   lastStat;
    struct  stat   currStat;
    size_t        newCount, readCount;
#endif

    memset(&lastStat, 0, sizeof(lastStat));

    while(argc > 1 && argv[1][0] == '-' && argv[1][1] == '-')
    {
        if(!strcmp("help", &argv[1][2]))                fprintf(stderr, "\nSyntax: Tail {--ignore-top} [Filename]\n");
        else if(!strcmp("ignore-top", &argv[1][2]))     ignore_top = 1;
        argc--;
        argv++;
    }

    if(argc == 2)
    {
        fileName = argv[1];
#ifdef WIN32
            file = open(fileName, _O_BINARY + _O_RDONLY);
#else
            file = open(fileName, O_RDONLY);
#endif
        if(file == -1) return 1;
    }
    else
        return 1;

    if(ignore_top) 
    {
#ifdef WIN32
        _stat(fileName, &lastStat);
        _lseek(file, lastStat.st_size, SEEK_SET);
#else
        stat(fileName, &lastStat);
        lseek(file, lastStat.st_size, SEEK_SET);
#endif
    }

    while(1)
    {
#ifdef WIN32
        _stat(fileName, &currStat);
#else
        stat(fileName, &currStat);
#endif
        newCount = currStat.st_size - lastStat.st_size;
        while(newCount > 0)
        {
            readCount = read(file, buffer, (newCount > BUFFER_SIZE) ? BUFFER_SIZE : newCount);
            newCount -= readCount;
            write(1, buffer, readCount);
        }
        lastStat = currStat;
#ifdef WIN32
        Sleep(100);
#else
        usleep(100 * 1000);
#endif
    }

    close(file);

    return 0;
}

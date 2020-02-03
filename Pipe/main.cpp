#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <fcntl.h>
#else
#define SOCKET int
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#endif

#include <pipe.hpp>

char DecodeCString(char* szCHex, int* pCount)
{
    char ch = 0;
    int count = 0;
    char szHex[3];
    char szOct[4];

    if(!szCHex)
    {
        if(pCount) (*pCount) = count;
        return ch;
    }

    count++;

    if(szCHex[0] != '\\')
    {
        if(pCount) (*pCount) = count;
        return szCHex[0];
    }

    switch(szCHex[1])
    {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        count += 1;
        szOct[0] = szCHex[1];
        szOct[1] = 0;
        if(szCHex[2] >= '0' && szCHex[2] <= '7')
        {
            count += 1;
            szOct[1] = szCHex[2];
            szOct[2] = 0;
            if(szCHex[3] >= '0' && szCHex[3] <= '7')
            {
                count += 1;
                szOct[2] = szCHex[3];
                szOct[3] = 0;
            }
        }
        ch = (char)strtoul(szOct, NULL, 8);
        break;
    case 'a':
        count++; ch = '\a'; break;
    case 'b':
        count++; ch = '\b'; break;
    case 't':
        count++; ch = '\t'; break;
    case 'n':
        count++; ch = '\n'; break;
    case 'v':
        count++; ch = '\v'; break;
    case 'f':
        count++; ch = '\f'; break;
    case 'r':
        count++; ch = '\r'; break;
    case '\"':
        count++; ch = '\"'; break;
    case '\'':
        count++; ch = '\''; break;
    case '\?':
        count++; ch = '\?'; break;
    case '\\':
        count++; ch = '\\'; break;
    case 'x':
        count += 3;
        szHex[0] = szCHex[2];
        szHex[1] = szCHex[3];
        szHex[2] = 0;
        ch = (char)strtoul(szHex, NULL, 16);
        break;
    }

    if(pCount) (*pCount) = count;

    return ch;
}

int recvFromFile (FILE* file, char * buf, int len)
{
    int idx = 0;
    int pos = 0;
    int inPos = 0;
    int outPos = 0;

    while(1)
    {
        fgets(buf, len, file);
        pos = (int)strlen(buf);

        while(pos)
        {
            if(buf[pos-1] == '\r' || buf[pos-1] == '\n')
            {
                buf[--pos] = 0;
            }
            else
                break;
        }

        if(!pos) break;

        while(inPos < pos)
        {
            buf[outPos++] = DecodeCString(&buf[inPos], &idx);
            inPos += idx;
        }

        break;
    }

    return outPos;
}

int PipeStopped = false;

#ifdef WIN32
DWORD WINAPI LogWriter(SOCKET* s)
{
#else
void* LogWriter(void* pVoid)
{
    SOCKET* s = (SOCKET*)pVoid;
#endif
    char        len;
    char        szBuffer[255+1];
    int         nCount;

#ifdef WIN32
    Sleep(1000*20);
#else
    usleep(1000*1000*20);
#endif

    while(pipe_read((int)*s, &len, 1) == 1)
    {
        nCount = len;
        nCount = pipe_read((int)*s, szBuffer, nCount);
        fprintf(stdout, "%.*s\n", nCount, szBuffer);
    }

    PipeStopped = true;

    return 0;
}

int main(int argc, char **argv)
{
#ifdef WIN32
    WORD        wVersionRequested = MAKEWORD(1,1);
    WSADATA     wsaData;
#endif
    int         nRet;
    char        szBuffer[255+1];
    int         nCount;
    int         fildes[2];

#ifdef WIN32
    HANDLE  thdHndle;
    DWORD   threadId;
#else
    pthread_t thread;
#endif

#ifdef WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif

#ifdef WIN32
    // Initialize WinSock and check version
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {
        fprintf(stderr,"\n Winsock Startup --> Wrong version, exiting\n");
        return 0;
    }
#endif

    if(pipe_create(fildes)) return 0;

#ifdef WIN32
    thdHndle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&LogWriter, (void*)&fildes[0], 0, &threadId);
    CloseHandle(thdHndle);
#else
    pthread_create(&thread, NULL, LogWriter, (void*)&fildes[0]);
    pthread_detach(thread);
#endif

    while(1)
    {
        char len;
        nCount = sizeof(szBuffer)-1;
        len = nRet = recvFromFile(stdin, szBuffer, nCount);
        if(nRet <= 0) break;
        pipe_write(fildes[1], &len, 1);
        pipe_write(fildes[1], szBuffer, nRet);
    }

    pipe_close(fildes);

    while(!PipeStopped);

    // Release WinSock
#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}

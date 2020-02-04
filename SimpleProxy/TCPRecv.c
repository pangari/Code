#include "Logging.h"

#include <curl/curl.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
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

// Thread structure
typedef struct _client_data
{
    SOCKET                  remoteSocket;
    struct sockaddr_in      remoteName;
    char                    szRemoteName[32];

    SOCKET                  hostSocket;
    struct sockaddr_in      hostName;

    char* sHostName;
    short nHostPort;
    short nServerPort;

    SERVER_FLAGS flags;

    char                    szInLog[64];
    char                    szOutLog[64];
    FILE*                   inLog;
    FILE*                   outLog;

    char                    szGLLog[64];
    FILE*                   glLog;

    int                     nSndBuf;
    int                     nRcvBuf;

} CLIENT_DATA, *PCLIENT_DATA;

// Function prototypes
void StreamServer(PSERVER_ARGS serverArgs);

#ifdef WIN32
DWORD WINAPI ClientIncomingThreadProc(PCLIENT_DATA data);
DWORD WINAPI ClientOutgoingThreadProc(PCLIENT_DATA data);
#else
void* ClientIncomingThreadProc(void* data);
void* ClientOutgoingThreadProc(void* data);
#endif

// Helper macro for displaying errors
#ifdef WIN32
#define PRINTERROR(s) \
    fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())
#else
#define PRINTERROR(s) \
    fprintf(stderr,"\n%s: %d\n", s, errno)
#endif

int AllocateBuffer(char** szBuffer, int nSize)
{
    if(!szBuffer) return 0;

    nSize *= 2;

    *szBuffer = (char*)realloc(*szBuffer, nSize);

    return (*szBuffer) ? nSize : 0;
}

int SendBytes(SOCKET socket, void* vBuffer, size_t bytesLeft)
{
    int nRet;
    int byteOffset = 0;
    char* buffer = (char*)vBuffer;

    while(bytesLeft > 0)
    {
        nRet = send(socket, &buffer[byteOffset], (int)bytesLeft, 0);
        if(nRet <= 0) break;
        byteOffset += nRet;
        bytesLeft -= nRet;
    }

    return (bytesLeft == 0) ? 1 : 0;
}

int main(int argc, char **argv)
{
#ifdef WIN32
    WORD wVersionRequested = MAKEWORD(1,1);
    WSADATA wsaData;
#endif
    int nRet;

    SERVER_ARGS serverArgs;

    memset(&serverArgs, 0, sizeof(serverArgs));
    serverArgs.nSndBuf = 1024*8;
    serverArgs.nRcvBuf = 1024*8;

    while(argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0')
    {
        char* opt = &argv[1][1];

        while(*opt)
        {
            switch(*opt)
            {
            case 'l':
                serverArgs.flags.RawLog = 1;
                break;
            case 'g':
                serverArgs.flags.GLLog = 1;
                break;
            case 'r':
                serverArgs.flags.replaceHostName = 1;
                break;
            case 's':
                serverArgs.flags.simpleTime = 1;
                break;
            case 'n':
                serverArgs.flags.noFlush = 1;
                break;
            }
            opt++;
        }

        argc--;
        argv++;
    }

    // Check for port argument
    if (argc < 4)
    {
        DisplayUsage(stderr);
        return 0;
    }

    serverArgs.nServerPort = atoi(argv[1]);
    serverArgs.sHostName = argv[2];
    serverArgs.nHostPort = atoi(argv[3]);
    if(argc > 4) serverArgs.nSndBuf = atoi(argv[4]);
    if(argc > 5) serverArgs.nRcvBuf = atoi(argv[5]);

#ifdef WIN32
    // Initialize WinSock and check version
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {
        fprintf(stderr,"\n Winsock Startup --> Wrong version, exiting\n");
        return 0;
    }
#endif

    // Do the stuff a stream server does
    StreamServer(&serverArgs);

    // Release WinSock
#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}

void StreamServer(PSERVER_ARGS serverArgs)
{
    // Create a TCP/IP stream socket to "listen" with
    SOCKET              listenSocket;
    struct sockaddr_in  saServer;
    struct sockaddr_in  saHost;
    struct hostent *    entHost;
    int                 nRet;

    int                 bNoDelay = 1;

    listenSocket = socket(AF_INET,  // Address family
        SOCK_STREAM,                // Socket type
        IPPROTO_TCP);               // Protocol
    if (listenSocket < 0)
    {
        PRINTERROR("socket()");
        return;
    }
    // Fill in the address structure
    saServer.sin_family = AF_INET;
    saServer.sin_addr.s_addr = INADDR_ANY;                  // Let WinSock supply address
    saServer.sin_port = htons(serverArgs->nServerPort);     // Use port from command line

    // bind the name to the socket
    nRet = bind(listenSocket,               // Socket
        (struct sockaddr*)&saServer,        // Our address
        sizeof(struct sockaddr));           // Size of address structure

    if (nRet)
    {
        PRINTERROR("bind()");

#ifdef WIN32
        shutdown(listenSocket, SD_BOTH);
        closesocket(listenSocket);
#else
        shutdown(listenSocket, SHUT_RDWR);
        close(listenSocket);
#endif
        return;
    }

    nRet = setsockopt(listenSocket,IPPROTO_TCP,TCP_NODELAY, (char *)&bNoDelay,sizeof(bNoDelay));
    nRet = setsockopt(listenSocket,SOL_SOCKET,SO_SNDBUF, (char *)&serverArgs->nSndBuf,sizeof(serverArgs->nSndBuf));
    nRet = setsockopt(listenSocket,SOL_SOCKET,SO_RCVBUF, (char *)&serverArgs->nRcvBuf,sizeof(serverArgs->nRcvBuf));

    DisplayServerStartUp(serverArgs, stderr);

    // Set the socket to listen
    nRet = listen(listenSocket, SOMAXCONN);
    if (nRet)
    {
        PRINTERROR("listen()");
#ifdef WIN32
        shutdown(listenSocket, SD_BOTH);
        closesocket(listenSocket);
#else
        shutdown(listenSocket, SHUT_RDWR);
        close(listenSocket);
#endif
        return;
    }

    // Wait for an incoming request
    while(1)
    {
        SOCKET remoteSocket;
        PCLIENT_DATA threadDataIn;
        PCLIENT_DATA threadDataOut;
        int bNoDelay = 1;
        int namelen = sizeof(threadDataIn->remoteName);

#ifdef WIN32
        HANDLE  thdHndle;
        DWORD   threadId;
#else
        pthread_t thread;
#endif

        remoteSocket = accept(listenSocket,     // Listening socket
            NULL,                               // Optional client address
            NULL);
        if (remoteSocket < 0)
        {
            PRINTERROR("accept()");
#ifdef WIN32
            shutdown(listenSocket, SD_BOTH);
            closesocket(listenSocket);
#else
            shutdown(listenSocket, SHUT_RDWR);
            close(listenSocket);
#endif
            return;
        }

        threadDataIn = (PCLIENT_DATA)malloc(sizeof(CLIENT_DATA));
        memset(threadDataIn, 0, sizeof(CLIENT_DATA));
        threadDataIn->remoteSocket = remoteSocket;

        nRet = setsockopt(threadDataIn->remoteSocket, IPPROTO_TCP,TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));
        nRet = setsockopt(threadDataIn->remoteSocket,SOL_SOCKET,SO_SNDBUF, (char *)&serverArgs->nSndBuf,sizeof(serverArgs->nSndBuf));
        nRet = setsockopt(threadDataIn->remoteSocket,SOL_SOCKET,SO_RCVBUF, (char *)&serverArgs->nRcvBuf,sizeof(serverArgs->nRcvBuf));

        getpeername(threadDataIn->remoteSocket, (struct sockaddr*)&threadDataIn->remoteName, &namelen);
        sprintf(threadDataIn->szRemoteName, "[%s][%d]", inet_ntoa(threadDataIn->remoteName.sin_addr), threadDataIn->remoteName.sin_port);
        if(serverArgs->flags.RawLog)
        {
            sprintf(threadDataIn->szInLog, "in%s.log", threadDataIn->szRemoteName);
            sprintf(threadDataIn->szOutLog, "out%s.log", threadDataIn->szRemoteName);
            threadDataIn->inLog = fopen(threadDataIn->szInLog, "wb");
            threadDataIn->outLog = fopen(threadDataIn->szOutLog, "wb");
        }
        if(serverArgs->flags.GLLog)
        {
            sprintf(threadDataIn->szGLLog, "feed%s.data", threadDataIn->szRemoteName);
            threadDataIn->glLog = fopen(threadDataIn->szGLLog, "wb");
        }
        threadDataIn->nSndBuf = serverArgs->nSndBuf;
        threadDataIn->nRcvBuf = serverArgs->nRcvBuf;

        threadDataIn->sHostName = serverArgs->sHostName;
        threadDataIn->nHostPort = serverArgs->nHostPort;
        threadDataIn->nServerPort = serverArgs->nServerPort;
        threadDataIn->flags = serverArgs->flags;

        threadDataIn->hostSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (remoteSocket < 0)
        {
            PRINTERROR("socket()");
#ifdef WIN32
            shutdown(threadDataIn->remoteSocket, SD_BOTH);
            closesocket(threadDataIn->remoteSocket);
#else
            shutdown(threadDataIn->remoteSocket, SHUT_RDWR);
            close(threadDataIn->remoteSocket);
#endif
            continue;
        }

        entHost = gethostbyname(serverArgs->sHostName);
        if(entHost == NULL) 
        {
            PRINTERROR("gethostbyname()");
#ifdef WIN32
            shutdown(threadDataIn->remoteSocket, SD_BOTH);
            closesocket(threadDataIn->remoteSocket);
#else
            shutdown(threadDataIn->remoteSocket, SHUT_RDWR);
            close(threadDataIn->remoteSocket);
#endif
            continue;
        }

        saHost.sin_family = AF_INET;
        memcpy(&saHost.sin_addr.s_addr, entHost->h_addr, entHost->h_length);
        saHost.sin_port = htons(serverArgs->nHostPort);

        nRet = setsockopt(threadDataIn->hostSocket, IPPROTO_TCP,TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));
        nRet = setsockopt(threadDataIn->hostSocket, SOL_SOCKET,SO_SNDBUF, (char *)&serverArgs->nSndBuf,sizeof(serverArgs->nSndBuf));
        nRet = setsockopt(threadDataIn->hostSocket, SOL_SOCKET,SO_RCVBUF, (char *)&serverArgs->nRcvBuf,sizeof(serverArgs->nRcvBuf));

        if(connect(threadDataIn->hostSocket, (struct sockaddr*)&saHost, sizeof(saHost)) < 0)
        {
            PRINTERROR("connect()");
#ifdef WIN32
            shutdown(threadDataIn->remoteSocket, SD_BOTH);
            closesocket(threadDataIn->remoteSocket);
#else
            shutdown(threadDataIn->remoteSocket, SHUT_RDWR);
            close(threadDataIn->remoteSocket);
#endif
            continue;
        }

        threadDataOut = (PCLIENT_DATA)malloc(sizeof(CLIENT_DATA));
        memcpy(threadDataOut, threadDataIn, sizeof(CLIENT_DATA));

#ifdef WIN32
        thdHndle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ClientIncomingThreadProc, threadDataIn, 0, &threadId);
        CloseHandle(thdHndle);

        thdHndle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ClientOutgoingThreadProc, threadDataOut, 0, &threadId);
        CloseHandle(thdHndle);
#else
        pthread_create(&thread, NULL, ClientIncomingThreadProc, threadDataIn);
        pthread_detach(thread);

        pthread_create(&thread, NULL, ClientOutgoingThreadProc, threadDataOut);
        pthread_detach(thread);
#endif

    }

#ifdef WIN32
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
#else
    shutdown(listenSocket, SHUT_RDWR);
    close(listenSocket);
#endif

    return;
}

#ifdef WIN32
DWORD WINAPI ClientIncomingThreadProc(PCLIENT_DATA data)
{
#else
void* ClientIncomingThreadProc(void* pVoid)
{
    PCLIENT_DATA data = (PCLIENT_DATA)pVoid;
#endif
    int DoLoop = 1;
    int nRet = 0;

    int nCount;
    char* szBuffer;
    char* szRcvBuffer;
    int nProcessCount, nRcvCount, nRcvOffset;
    char* ptr = NULL;

    int byteOffset = 0;
    int bytesLeft = 0;
    char* bytes = NULL;

    char ProxyDesc1[512];
    char HostDesc1[256];
    int diff1 = 0;

    char ProxyDesc2[512];
    char HostDesc2[256];
    int diff2 = 0;

    char URL[1024] = "";

    if(data->flags.replaceHostName)
    {
        sprintf(HostDesc1, "%s:%d", data->sHostName, data->nHostPort);
        if(gethostname(ProxyDesc1, sizeof(ProxyDesc1))) sprintf(ProxyDesc1, "127.0.0.1");
        sprintf(ProxyDesc1 + strlen(ProxyDesc1), ":%d", data->nServerPort);
        diff1 = ((int)strlen(HostDesc1) - (int)strlen(ProxyDesc1));
    }
    if(data->flags.replaceHostName)
    {
        sprintf(HostDesc2, "%s", data->sHostName);
        if(gethostname(ProxyDesc2, sizeof(ProxyDesc2))) sprintf(ProxyDesc2, "127.0.0.1");
        diff2 = ((int)strlen(HostDesc2) - (int)strlen(ProxyDesc2));
    }
    nCount = data->nRcvBuf * 4 + 1 + (diff1 > diff2 ? diff1 : diff2);

    szBuffer = (char*)malloc(nCount+1);
    szRcvBuffer = (char*)malloc(data->nRcvBuf+1);

    LogConnection(&data->flags, 1, data->szRemoteName, "INCOMING", stderr);

    nRcvOffset = 0;
    while(DoLoop)
    {
        nRcvCount = data->nRcvBuf-nRcvOffset;
        if(nRcvCount == 0) break;
        nRet = recv(data->remoteSocket, szRcvBuffer+nRcvOffset, nRcvCount, 0);
        if(nRet <= 0) break;

        LogRawData(&data->flags, szRcvBuffer+nRcvOffset, nRet, data->inLog, DataIn, szBuffer);

        nRcvOffset += nRet;
        nRet = nRcvOffset;
        szRcvBuffer[nRet] = 0;

        nRcvOffset = 0;
        if(data->flags.replaceHostName)
        {
            while(ptr = strstr(szRcvBuffer, ProxyDesc1))
            {
                int shift = nRet - (int)(ptr - szRcvBuffer);

                if(diff1 > 0)
                {
                    memmove((ptr + diff1), ptr, shift);
                }
                else if (diff1 < 0)
                {
                    memmove(ptr, (ptr + diff1), shift);
                }

                nRet += diff1;
                memcpy(ptr, HostDesc1, strlen(HostDesc1));
            }
            while(ptr = strstr(szRcvBuffer, ProxyDesc2))
            {
                int shift = nRet - (int)(ptr - szRcvBuffer);

                if(diff2 > 0)
                {
                    memmove((ptr + diff2), ptr, shift);
                }
                else if (diff2 < 0)
                {
                    memmove(ptr, (ptr + diff2), shift);
                }

                nRet += diff2;
                memcpy(ptr, HostDesc2, strlen(HostDesc2));
            }
        }

        nRcvCount = nRet;

        byteOffset = 0;
        bytesLeft = nRcvCount;
        bytes = szRcvBuffer;
        while(bytesLeft)
        {
            nRet = send(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
            if(nRet <= 0) break;
            byteOffset += nRet;
            bytesLeft -= nRet;
        }
        if(bytesLeft) break;
    }

    LogConnection(&data->flags, 0, data->szRemoteName, "INCOMING", stderr);

    if(data->inLog) fclose(data->inLog);

    // Close socket before exiting
#ifdef WIN32
    shutdown(data->remoteSocket, SD_BOTH);
    closesocket(data->remoteSocket);
    shutdown(data->hostSocket, SD_BOTH);
    closesocket(data->hostSocket);
#else
    shutdown(data->remoteSocket, SHUT_RDWR);
    close(data->remoteSocket);
    shutdown(data->hostSocket, SHUT_RDWR);
    close(data->hostSocket);
#endif

    free(szBuffer);
    free(szRcvBuffer);

    free(data);

    return 0;
}

#ifdef WIN32
DWORD WINAPI ClientOutgoingThreadProc(PCLIENT_DATA data)
{
#else
void* ClientOutgoingThreadProc(void* pVoid)
{
    PCLIENT_DATA data = (PCLIENT_DATA)pVoid;
#endif
    int nRet = 0;

    int nCount;
    char* szBuffer;
    char* szSndBuffer;
    int nSndCount;
    char* ptr = NULL;

    int byteOffset = 0;
    int bytesLeft = 0;
    char* bytes = NULL;

    char ProxyDesc1[512];
    char HostDesc1[256];
    int diff1 = 0;

    char ProxyDesc2[512];
    char HostDesc2[256];
    int diff2 = 0;

    if(data->flags.replaceHostName)
    {
        sprintf(HostDesc1, "%s:%d", data->sHostName, data->nHostPort);
        if(gethostname(ProxyDesc1, sizeof(ProxyDesc1))) sprintf(ProxyDesc1, "127.0.0.1");
        sprintf(ProxyDesc1 + strlen(ProxyDesc1), ":%d", data->nServerPort);
        diff1 = ((int)strlen(HostDesc1) - (int)strlen(ProxyDesc1));
    }
    if(data->flags.replaceHostName)
    {
        sprintf(HostDesc2, "%s", data->sHostName);
        if(gethostname(ProxyDesc2, sizeof(ProxyDesc2))) sprintf(ProxyDesc2, "127.0.0.1");
        diff2 = ((int)strlen(HostDesc2) - (int)strlen(ProxyDesc2));
    }
    nCount = data->nSndBuf * 4 + 1 + (diff1 > diff2 ? diff1 : diff2);

    szBuffer = (char*)malloc(nCount+1);
    szSndBuffer = (char*)malloc(data->nSndBuf+1);

    LogConnection(&data->flags, 1, data->szRemoteName, "OUTGOING", stderr);

    while(1)
    {
        nSndCount = data->nSndBuf;
        nRet = recv(data->hostSocket, szSndBuffer, nSndCount, 0);
        if(nRet <= 0) break;
        szSndBuffer[nRet] = 0;

        if(data->flags.replaceHostName)
        {
            /*
            {
                char* ProxyDesc1 = "f.transfer == \"[object]\"";
                char* HostDesc1  = "f.transfer              ";
                int diff1 = ((int)strlen(HostDesc1) - (int)strlen(ProxyDesc1));

                while(ptr = strstr(szSndBuffer, ProxyDesc1))
                {
                    int shift = nRet - (int)(ptr - szSndBuffer);

                    if(diff1 > 0)
                    {
                        memmove((ptr + diff1), ptr, shift);
                    }
                    else if (diff1 < 0)
                    {
                        memmove(ptr, (ptr - diff1), shift);
                    }

                    nRet += diff1;
                    memcpy(ptr, HostDesc1, strlen(HostDesc1));
                }
            }
            */

            /*
            while(ptr = strstr(szSndBuffer, HostDesc1))
            {
                int shift = nRet - (int)(ptr - szSndBuffer);

                if(diff1 < 0)
                {
                    memmove((ptr + diff1), ptr, shift);
                }
                else if (diff1 > 0)
                {
                    memmove(ptr, (ptr + diff1), shift);
                }

                nRet += diff1;
                memcpy(ptr, ProxyDesc1, strlen(ProxyDesc1));
            }
            while(ptr = strstr(szSndBuffer, HostDesc2))
            {
                int shift = nRet - (int)(ptr - szSndBuffer);

                if(diff2 < 0)
                {
                    memmove((ptr + diff2), ptr, shift);
                }
                else if (diff2 > 0)
                {
                    memmove(ptr, (ptr + diff2), shift);
                }

                nRet += diff2;
                memcpy(ptr, ProxyDesc2, strlen(ProxyDesc2));
            }
            */
        }

        LogRawData(&data->flags, szSndBuffer, nRet, data->outLog, DataOut, szBuffer);
        WriteGLFeed(nRet, szSndBuffer, data->glLog);

        nSndCount = nRet;

        byteOffset = 0;
        bytesLeft = nSndCount;
        bytes = szSndBuffer;
        while(bytesLeft)
        {
            nRet = send(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
            if(nRet <= 0) break;
            byteOffset += nRet;
            bytesLeft -= nRet;
        }
        if(bytesLeft) break;
    }

    LogConnection(&data->flags, 0, data->szRemoteName, "OUTGOING", stderr);

    if(data->outLog) fclose(data->outLog);
    if(data->glLog) fclose(data->glLog);

    // Close socket before exiting
#ifdef WIN32
    shutdown(data->hostSocket, SD_BOTH);
    closesocket(data->hostSocket);
    shutdown(data->remoteSocket, SD_BOTH);
    closesocket(data->remoteSocket);
#else
    shutdown(data->hostSocket, SHUT_RDWR);
    close(data->hostSocket);
    shutdown(data->remoteSocket, SHUT_RDWR);
    close(data->remoteSocket);
#endif

    free(szBuffer);
    free(szSndBuffer);

    free(data);

    return 0;
}

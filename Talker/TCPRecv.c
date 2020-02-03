#include "Logging.h"
#include "base64.h"

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

#include <time.h>
#include <signal.h>

// Thread structure
typedef struct _client_data
{
    SOCKET              remoteSocket;
    struct sockaddr_in  remoteName;
    char                szRemoteName[128];

    char                szInLog[64];
    char                szOutLog[64];
    FILE*               inLog;
    FILE*               outLog;

    char                szGLLog[64];

    int                 nSndBuf;
    int                 nRcvBuf;

    int                 id;

    SERVER_FLAGS flags;

    char* szBuffer;
    char* szCmpzBuffer;

} CLIENT_DATA, *PCLIENT_DATA;

typedef struct _server_args
{
    short nServerPort;
    char* szServerHost;
    int nSndBuf;
    int nRcvBuf;

    int maxActiveConnections;

    SERVER_FLAGS flags;

} SERVER_ARGS, *PSERVER_ARGS;

// Function prototypes
void StreamServer(PSERVER_ARGS serverArgs);

int recvFromConsole (char * buf, int len);
int recvFromConsoleBase64(char * buf, int len);
SOCKET ConnectToHostServer(char* szHostName, short nHostPort, int nSndBuf, int nRcvBuf);

#ifdef WIN32
DWORD WINAPI ClientIncomingThreadProc(PCLIENT_DATA data);
DWORD WINAPI ClientOutgoingThreadProc(PCLIENT_DATA data);
#else

#ifdef  __cplusplus
extern "C" {
#endif

    void* ClientIncomingThreadProc(void* data);
    void* ClientOutgoingThreadProc(void* data);
    static void proxy_signal_stat(int _signal);
    static void proxy_signal(int _signal);

#ifdef  __cplusplus
}
#endif

#endif

// Helper macro for displaying errors
#ifdef WIN32
#define PRINTERROR(s)   \
    fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())
#else
#define PRINTERROR(s)   \
    fprintf(stderr,"\n%s: %d\n", s, errno)
#endif

SOCKET  listenSocket;

int AllocateBuffer(char** szBuffer, int nSize)
{
    if(!szBuffer) return 0;

    nSize *= 2;

    *szBuffer = (char*)realloc(*szBuffer, nSize);

    return (*szBuffer) ? nSize : 0;
}

#ifdef WIN32
double GetElapsedTime()
{
    static DWORD InitialTick = 0;
    static DWORD ClocksPerSec = 1000;

    if(!InitialTick) InitialTick = GetTickCount();

    return ((double)(GetTickCount() - InitialTick) / (double)ClocksPerSec);
}
#else
double GetElapsedTime()
{
    static clock_t ClocksPerSec = 0;

    if(!ClocksPerSec) ClocksPerSec = (clock_t)sysconf(_SC_CLK_TCK);

    return ((double)clock() / (double)ClocksPerSec);
}
#endif

#ifdef WIN32
#define LockInit(lock)    InitializeCriticalSection(&lock);
#define LockDestroy(lock) DeleteCriticalSection(&lock);
#define LockLock(lock)    EnterCriticalSection(&lock);
#define LockUnlock(lock)  LeaveCriticalSection(&lock);
#else
#define LockInit(lock)    pthread_mutex_init(&lock, NULL)
#define LockDestroy(lock) pthread_mutex_destroy(&lock);
#define LockLock(lock)    pthread_mutex_lock(&lock);
#define LockUnlock(lock)  pthread_mutex_unlock(&lock);
#endif

#ifdef WIN32
CRITICAL_SECTION lock;
#else
pthread_mutex_t lock;
#endif

void CleanExit(int code)
{
#ifdef WIN32
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
#else
    shutdown(listenSocket, SHUT_RDWR);
    close(listenSocket);
#endif
    exit(code);
}

#ifdef WIN32
BOOL WINAPI proxy_signal_abrt(DWORD dwCtrlType)
{
    switch(dwCtrlType)
    {
    case CTRL_C_EVENT:
        {
            fputs("\n[CTRL_C_EVENT]\n", stderr);
            CleanExit(1);
            return FALSE;
        }
    case CTRL_BREAK_EVENT:
        {
            fputs("\n[CTRL_BREAK_EVENT]\n", stderr);
            CleanExit(1);
            return FALSE;
        }
    }

    return FALSE;
}
#else
static void proxy_signal(int _signal)
{
    switch(_signal)
    {
    case SIGINT:
        {
            fputs("\n[SIGINT]\n", stderr);
            signal(SIGINT, SIG_DFL);
            CleanExit(1);
            break;
        }
    }
}
#endif

int activeConnections = 0;

int main(int argc, char **argv)
{
#ifdef WIN32
    WORD         wVersionRequested = MAKEWORD(1,1);
    WSADATA      wsaData;
#endif
    int          nRet;
    int          flags = 0;
    int          reqArgs = 2;

    SERVER_ARGS  serverArgs;

#ifdef WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    memset(&serverArgs, 0, sizeof(serverArgs));

    serverArgs.nSndBuf = 8190+2+2+14;
    serverArgs.nRcvBuf = serverArgs.nSndBuf;
    serverArgs.maxActiveConnections = 1;

    while(argc > 1 && argv[1][0] == '-' && argv[1][1] == '-')
    {
        if(!strcmp("client", &argv[1][2]))          serverArgs.flags.ClientMode = 1;
        else if(!strcmp("rawLog", &argv[1][2]))     serverArgs.flags.RawLog = 1;
        else if(!strcmp("screenLog", &argv[1][2]))  serverArgs.flags.ScreenLog = 1;
        else if(!strcmp("base64", &argv[1][2]))     serverArgs.flags.Base64 = 1;
        else if(!strcmp("help", &argv[1][2]))       {DisplayDetailedUsage(stderr); return 0;}

            argc--;
            argv++;
    }

    if(serverArgs.flags.ClientMode)
    {
        reqArgs += 1;
        serverArgs.flags.RawLog = 0;
        serverArgs.flags.ScreenLog = 1;
    }

    if(serverArgs.flags.Base64)
    {
        serverArgs.flags.ScreenLog = 0;
    }

    // Check for port argument
    if (argc != reqArgs)
    {
        DisplayUsage(stderr);
        return 0;
    }

    serverArgs.nServerPort = atoi(argv[1]);

    if(serverArgs.flags.ClientMode) 
        serverArgs.szServerHost = argv[2];

#ifdef WIN32
    // Initialize WinSock and check version
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {
        fprintf(stderr,"\n Winsock Startup --> Wrong version, exiting\n");
        return 0;
    }
#endif

    LockInit(lock);

#ifdef WIN32
    SetConsoleCtrlHandler(proxy_signal_abrt, TRUE);
#else
    signal(SIGINT, proxy_signal);
#endif

    StreamServer(&serverArgs);

#ifdef WIN32
    SetConsoleCtrlHandler(proxy_signal_abrt, FALSE);
#else
    signal(SIGINT, SIG_DFL);
#endif

    LockDestroy(lock);

    // Release WinSock
#ifdef WIN32
    WSACleanup();
#endif

    return 0;
}

void StreamServer(PSERVER_ARGS serverArgs)
{
    // Create a TCP/IP stream socket to "listen" with
    struct sockaddr_in  saServer;
    int                 nRet;

    int                 bNoDelay = 1;

    if(serverArgs->flags.ClientMode)
    {
        if(listenSocket = ConnectToHostServer(serverArgs->szServerHost, serverArgs->nServerPort, serverArgs->nSndBuf, serverArgs->nRcvBuf))
        {
            CLIENT_DATA threadData;
            memset(&threadData, 0, sizeof(CLIENT_DATA));
            threadData.nSndBuf = serverArgs->nSndBuf;
            threadData.nRcvBuf = serverArgs->nRcvBuf;
            threadData.flags = serverArgs->flags;
            threadData.remoteSocket = listenSocket;
            sprintf(threadData.szRemoteName, "%s:%d", serverArgs->szServerHost, serverArgs->nServerPort);
            ClientIncomingThreadProc(&threadData);
        }
    }
    else
    {
        listenSocket = socket(AF_INET,      // Address family
            SOCK_STREAM,                    // Socket type
            IPPROTO_TCP);                   // Protocol
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
        nRet = bind(listenSocket,                               // Socket
            (struct sockaddr*)&saServer,                        // Our address
            sizeof(struct sockaddr));                           // Size of address structure

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

        if(!serverArgs->flags.ClientMode)
            DisplayServerStartUp(serverArgs->nServerPort, &serverArgs->flags, stderr);

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
            SOCKET          remoteSocket;
            PCLIENT_DATA    threadDataIn;
            int             bNoDelay = 1;
            socklen_t       namelen = sizeof(threadDataIn->remoteName);

    #ifdef WIN32
            HANDLE  thdHndle;
            DWORD   threadId;
    #else
            pthread_t thread;
    #endif

            remoteSocket = accept(listenSocket,     // Listening socket
                NULL,                               // Optional client address
                NULL);

            LockLock(lock);
            if(activeConnections >= serverArgs->maxActiveConnections)
            {
    #ifdef WIN32
                shutdown(remoteSocket, SD_BOTH);
                closesocket(remoteSocket);
    #else
                shutdown(remoteSocket, SHUT_RDWR);
                close(remoteSocket);
    #endif
                LockUnlock(lock);
                continue;
            }
            LockUnlock(lock);

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
            nRet = setsockopt(threadDataIn->remoteSocket, SOL_SOCKET,SO_SNDBUF, (char *)&serverArgs->nSndBuf,sizeof(serverArgs->nSndBuf));
            nRet = setsockopt(threadDataIn->remoteSocket, SOL_SOCKET,SO_RCVBUF, (char *)&serverArgs->nRcvBuf,sizeof(serverArgs->nRcvBuf));

            getpeername(threadDataIn->remoteSocket, (struct sockaddr*)&threadDataIn->remoteName, &namelen);
            sprintf(threadDataIn->szRemoteName, "[%s][%d]", inet_ntoa(threadDataIn->remoteName.sin_addr), threadDataIn->remoteName.sin_port);
            if(serverArgs->flags.RawLog)
            {
                sprintf(threadDataIn->szInLog, "in%s.log", threadDataIn->szRemoteName);
                sprintf(threadDataIn->szOutLog, "out%s.log", threadDataIn->szRemoteName);
                threadDataIn->inLog = fopen(threadDataIn->szInLog, "ab");
                threadDataIn->outLog = fopen(threadDataIn->szOutLog, "ab");
            }
            threadDataIn->nSndBuf = serverArgs->nSndBuf;
            threadDataIn->nRcvBuf = serverArgs->nRcvBuf;

            threadDataIn->flags = serverArgs->flags;

            LockLock(lock);
            threadDataIn->id = ++activeConnections;
            LockUnlock(lock);

    #ifdef WIN32
            thdHndle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ClientIncomingThreadProc, threadDataIn, 0, &threadId);
            CloseHandle(thdHndle);
    #else
            pthread_create(&thread, NULL, ClientIncomingThreadProc, threadDataIn);
            pthread_detach(thread);
    #endif
        }
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

SOCKET ConnectToHostServer(char* szHostName, short nHostPort, int nSndBuf, int nRcvBuf)
{
    SOCKET              hostSocket;
    int                 nRet;
    int                 bNoDelay = 1;
    struct hostent *    entHost;
    struct sockaddr_in  saHost;

    hostSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (hostSocket < 0)
    {
        PRINTERROR("socket()");
        return (SOCKET)NULL;
    }

    entHost = gethostbyname(szHostName);

    if(entHost == NULL) 
    {
        PRINTERROR("gethostbyname()");
        return (SOCKET)NULL;
    }

    saHost.sin_family = AF_INET;
    memcpy(&saHost.sin_addr.s_addr, entHost->h_addr, entHost->h_length);

    saHost.sin_port = htons(nHostPort);

    nRet = setsockopt(hostSocket, IPPROTO_TCP,TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));
    nRet = setsockopt(hostSocket, SOL_SOCKET,SO_SNDBUF, (char *)&nSndBuf,sizeof(nSndBuf));
    nRet = setsockopt(hostSocket, SOL_SOCKET,SO_RCVBUF, (char *)&nRcvBuf,sizeof(nRcvBuf));

    if(connect(hostSocket, (struct sockaddr*)&saHost, sizeof(saHost)) < 0)
    {
        PRINTERROR("connect()");
#ifdef WIN32
        shutdown(hostSocket, SD_BOTH);
        closesocket(hostSocket);
#else
        shutdown(hostSocket, SHUT_RDWR);
        close(hostSocket);
#endif
        return (SOCKET)NULL;
    }

    return hostSocket;
}

#ifdef WIN32
DWORD WINAPI ClientIncomingThreadProc(PCLIENT_DATA data)
{
#else
void* ClientIncomingThreadProc(void* pVoid)
{
    PCLIENT_DATA data = (PCLIENT_DATA)pVoid;
#endif
    int nRet = 0;

    int nCount = data->nRcvBuf * 4 + 1;
    int overhead = data->nRcvBuf / 16 + 64 + 3;
    char* szBuffer = (char*)malloc(nCount);
    char* szCmpzBuffer = (char*)malloc(data->nRcvBuf + overhead);
    char* szRcvBuffer = &szCmpzBuffer[overhead];
    int nRcvCount;

    PCLIENT_DATA threadDataOut = NULL;

    if(!data->flags.ClientMode)
    {
        LockLock(lock);
        LogConnection(1, data->szRemoteName, (char*)"INCOMING", data->id, activeConnections, stderr);
        LockUnlock(lock);
    }
    else
    {
        fprintf(stdout, "begin-base64 000 %s\n", data->szRemoteName);
    }

    do
    {
#ifdef WIN32
        HANDLE  thdHndle;
        DWORD   threadId;
#else
        pthread_t thread;
#endif

        threadDataOut = (PCLIENT_DATA)malloc(sizeof(CLIENT_DATA));
        memcpy(threadDataOut, data, sizeof(CLIENT_DATA));

        threadDataOut->szBuffer = (char*)malloc(data->nSndBuf * 4 + 1);
        threadDataOut->szCmpzBuffer = (char*)malloc(data->nSndBuf + (data->nSndBuf / 16 + 64 + 3));

#ifdef WIN32
        thdHndle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ClientOutgoingThreadProc, threadDataOut, 0, &threadId);
        CloseHandle(thdHndle);
#else
        pthread_create(&thread, NULL, ClientOutgoingThreadProc, threadDataOut);
        pthread_detach(thread);
#endif

        while(1)
        {
            nRcvCount = data->nRcvBuf;
            nRet = recv(data->remoteSocket, szRcvBuffer, nRcvCount, 0);
            if(nRet <= 0) break;
            nRcvCount = nRet;

            if(data->flags.RawLog) LogRawData(szRcvBuffer, nRcvCount, data->inLog, szBuffer);
            if(data->flags.ScreenLog) LogToScreen(szRcvBuffer, nRcvCount, stdout, szBuffer);
            if(data->flags.Base64) LogToScreenBase64(szRcvBuffer, nRcvCount, stdout, szBuffer);

            //nRet = send(data->hostSocket, szRcvBuffer, nRcvCount, 0);
            //if(nRet <= 0) break;
            //tmpStats.SendBytes = nRcvCount;
        }
    }
    while(0);

    if(!data->flags.ClientMode)
    {
        LockLock(lock);
        activeConnections--;
        LogConnection(0, data->szRemoteName, (char*)"INCOMING", data->id, activeConnections, stderr);
        LogConnection(0, data->szRemoteName, (char*)"OUTGOING", data->id, activeConnections, stderr);
        LockUnlock(lock);
    }

    // Close socket before exiting
#ifdef WIN32
    shutdown(data->remoteSocket, SD_BOTH);
    closesocket(data->remoteSocket);
#else
    shutdown(data->remoteSocket, SHUT_RDWR);
    close(data->remoteSocket);
#endif

    if(data->flags.ClientMode) fprintf(stdout, "====\n");

    _exit(0);

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

    int nCount   = data->nSndBuf * 4 + 1;
    int overhead = data->nSndBuf / 16 + 64 + 3;
    char* szBuffer = data->szBuffer;
    char* szCmpzBuffer = data->szCmpzBuffer;
    char* szSndBuffer = &szCmpzBuffer[overhead];
    int nSndCount;

    if(!data->flags.ClientMode)
    {
        LockLock(lock);
        LogConnection(1, data->szRemoteName, (char*)"OUTGOING", data->id, activeConnections, stderr);
        LockUnlock(lock);
    }

    do
    {
        while(1)
        {
            nSndCount = data->nSndBuf;
            if(data->flags.Base64)
                nRet = recvFromConsoleBase64(szSndBuffer, nSndCount);
            else
                nRet = recvFromConsole(szSndBuffer, nSndCount);
            if(nRet <= 0) break;
            nSndCount = nRet;

            if(data->flags.RawLog) LogRawData(szSndBuffer, nSndCount, data->outLog, szBuffer);

            nRet = send(data->remoteSocket , szSndBuffer, nSndCount, 0);
            if(nRet <= 0) break;
        }
    }
    while(0);

#ifdef WIN32
    shutdown(data->remoteSocket, SD_BOTH);
    closesocket(data->remoteSocket);
#else
    shutdown(data->remoteSocket, SHUT_RDWR);
    close(data->remoteSocket);
#endif

    return 0;
}

int recvFromConsole (char * buf, int len)
{
    int idx = 0;
    int pos = 0;
    int inPos = 0;
    int outPos = 0;

    while(1)
    {
        fgets(buf, len, stdin);
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

int recvFromConsoleBase64(char * buf, int len)
{
    int idx = 0;
    int pos = 0;
    int inPos = 0;
    int outPos = 0;
    char *inBuf, *outBuf;
    int nCount;

    while(1)
    {
        fgets(buf, len, stdin);
        pos = (int)strlen(buf);

        while(pos)
        {
            if(buf[pos-1] == '\r' || 
               buf[pos-1] == '\n' || 
               buf[pos-1] == '\t' || 
               buf[pos-1] == ' '
               )
            {
                buf[--pos] = 0;
            }
            else
                break;
        }

        inBuf = buf;
        outBuf = buf;

        while(*inBuf)
        {
            if((*inBuf) == '\r' || 
               (*inBuf) == '\n' || 
               (*inBuf) == '\t' || 
               (*inBuf) == ' '
               )
            {
                pos--;
                inBuf++;
            }
            else
                break;
        }

        nCount = outPos = pos;

        if(!nCount) break;
        if(nCount % 4) continue;

        while(nCount > 0)
        {
            decodeBase64((unsigned char*)inBuf, (unsigned char*)outBuf);
            nCount -= 4;
            inBuf += 4;
            outBuf += 3;
        }
        (*outBuf++) = '\0';

        nCount = (int)strlen(buf);

        if(!nCount) continue;

        break;
    }

    return nCount;
}

#include "Logging.h"
#include "GetPasswd.h"
#include "crypt.h"

#include <lzo/lzo.h>

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

#include <signal.h>

// Thread structure
typedef struct _client_data
{
    SOCKET              remoteSocket;
    struct sockaddr_in  remoteName;
    char                szRemoteName[128];

    SOCKET               hostSocket;
    struct sockaddr_in	hostName;
    char                szHostName[128];
    short               nHostPort;

    char                szProxyName[128];
    short               nProxyPort;

    char                szInLog[64];
    char                szOutLog[64];
    FILE*               inLog;
    FILE*               outLog;

    char                szGLLog[64];
    FILE*               glLog;

    int                 nSndBuf;
    int                 nRcvBuf;

    int                 id;

    SERVER_FLAGS    flags;
    SERVER_KEYS     keys;

} CLIENT_DATA, *PCLIENT_DATA;

typedef struct _server_args
{
    short nServerPort;
    char szHostName[128];
    short nHostPort;
    int nSndBuf;
    int nRcvBuf;

    char szProxyName[128];
    short nProxyPort;

    int maxActiveConnections;

    SERVER_FLAGS    flags;
    SERVER_KEYS     keys;

} SERVER_ARGS, *PSERVER_ARGS;

typedef unsigned long CRC;
typedef unsigned char uchar_t;
typedef unsigned long ulong_t;
CRC sum(CRC filecrc, uchar_t *bp, ulong_t n);

/* magic file header for compressed data */
static const unsigned char magic[10] = { 0xCA, 0xFE, 0xBA, 0xBE, 0xE9, 0x4C, 0x5A, 0x4F, 0xFF, 0x1A };

#pragma pack(1)
typedef struct _client_logon
{
    char  header[sizeof(magic)];
    short hostPort;
    char  hostLength;
    char  hostName[127];
    INITIAL_VECTOR Fish;
    INITIAL_VECTOR Ice;
    INITIAL_VECTOR Tea;

} CLIENT_LOGON, *PCLIENT_LOGON;
#pragma pack()

#pragma pack(1)
typedef struct _client_packet
{
    char  header[sizeof(magic)];
    unsigned short length;
    CRC checkSum;

} CLIENT_PACKET, *PCLIENT_PACKET;
#pragma pack()

enum ENCRYPTION
{
    ICE_ENCRYPTION,
    TEA_ENCRYPTION,
    FISH_ENCRYPTION
};

// Function prototypes
void StreamServer(PSERVER_ARGS serverArgs);

PCLIENT_DATA ConnectToHostServer(PCLIENT_DATA threadDataIn);

char* DoEncrypt(SERVER_FLAGS* flags, SERVER_KEYS* keys, char* szBuffer, char* szWorkBuf, int InCount, int* OutCount, int InOut, SERVER_STATS* stats);
char* DoDecrypt(SERVER_FLAGS* flags, SERVER_KEYS* keys, char* szBuffer, char* szWorkBuf, int InCount, int* OutCount, int InOut, SERVER_STATS* stats);

#ifdef WIN32
DWORD WINAPI ClientIncomingThreadProc(PCLIENT_DATA data);
DWORD WINAPI ClientOutgoingThreadProc(PCLIENT_DATA data);
#else
void* ClientIncomingThreadProc(void* data);
void* ClientOutgoingThreadProc(void* data);
#endif

// Helper macro for displaying errors
#ifdef WIN32
#define PRINTERROR(s)	\
    fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())
#else
#define PRINTERROR(s)	\
    fprintf(stderr,"\n%s: %d\n", s, errno)
#endif

int AllocateBuffer(char** szBuffer, int nSize)
{
    if(!szBuffer) return 0;

    nSize *= 2;

    *szBuffer = (char*)realloc(*szBuffer, nSize);

    return (*szBuffer) ? nSize : 0;
}

SERVER_STATS gStats;
SERVER_FLAGS gFlags;

#ifdef WIN32
CRITICAL_SECTION lock;
#define LockInit(lock)    InitializeCriticalSection(&lock);
#define LockDestroy(lock) DeleteCriticalSection(&lock);
#define LockLock(lock)    EnterCriticalSection(&lock);
#define LockUnlock(lock)  LeaveCriticalSection(&lock);
#else
pthread_mutex_t lock;
#define LockInit(lock)    pthread_mutex_init(&lock, NULL)
#define LockDestroy(lock) pthread_mutex_destroy(&lock);
#define LockLock(lock)    pthread_mutex_lock(&lock);
#define LockUnlock(lock)  pthread_mutex_unlock(&lock);
#endif

#ifdef WIN32
BOOL WINAPI proxy_signal(DWORD dwCtrlType)
{
    if(dwCtrlType == CTRL_C_EVENT)
    {
#else
static void proxy_signal(int _signal)
{
#endif
SERVER_STATS stats = gStats;
if(gFlags.Statistics)   LogSocketStatistics("GLOBAL", &stats, 0, stderr);
if(gFlags.LZO)          LogLzoStatistics("GLOBAL", &stats, 0, stderr);
if(gFlags.FISH)         LogFishStatistics("GLOBAL", &stats, 0, stderr);
if(gFlags.ICE)          LogIceStatistics("GLOBAL", &stats, 0, stderr);
if(gFlags.TEA)          LogTeaStatistics("GLOBAL", &stats, 0, stderr);
#ifdef WIN32
return TRUE;
    }
    return FALSE;
#else
#endif
}

int activeConnections = 0;

int main(int argc, char **argv)
{
#ifdef WIN32
    WORD         wVersionRequested = MAKEWORD(1,1);
    WSADATA      wsaData;
#endif
    int          nRet;
    int          flags = 0;
    int          reqArgs = 4;

    char         keyBufferA[1024+1];
    char         keyBufferB[1024+1];

    SERVER_ARGS  serverArgs;

#ifdef WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    memset(&gStats, 0, sizeof(gStats));
    memset(&serverArgs, 0, sizeof(serverArgs));
    memset(&gFlags, 0, sizeof(gFlags));

#ifdef _DEBUG1
    fprintf(stderr, "\n");
    {
        size_t count;
        void* key;
        INITIAL_VECTOR iv;
        long long StTime, EdTime;

        unsigned char in[34] = "_12345678123456781234567812345678_";
        unsigned char out[42];
        unsigned char back[42];

        key = fish_init(128, "HelloWorld");
        iv = build_iv();

        StTime = GetSimplePrecisionTime();
        count = fish_encrypt(key, iv, in, out, 34);
        count = fish_decrypt(key, iv, out, back, count);
        EdTime = GetSimplePrecisionTime() - StTime;

        fish_destroy(key);

        fprintf(stderr, "BlowFish: %s, Time(%lld)\n", (memcmp(in, back, 34) ? "FAIL" : "PASS"), EdTime);
    }
    {
        size_t count;
        void* key;
        INITIAL_VECTOR iv;
        long long StTime, EdTime;

        unsigned char in[34] = "_12345678123456781234567812345678_";
        unsigned char out[42];
        unsigned char back[42];

        key = ice_init(8, "HelloWorld");
        iv = build_iv();

        StTime = GetSimplePrecisionTime();
        count = ice_encrypt(key, iv, in, out, 34);
        count = ice_decrypt(key, iv, out, back, count);
        EdTime = GetSimplePrecisionTime() - StTime;

        ice_destroy(key);

        fprintf(stderr, "Ice8  : %s, Time(%lld)\n", (memcmp(in, back, 34) ? "FAIL" : "PASS"), EdTime);
    }
    {
        size_t count;
        void* key;
        INITIAL_VECTOR iv;
        long long StTime, EdTime;

        unsigned char in[34] = "_12345678123456781234567812345678_";
        unsigned char out[42];
        unsigned char back[42];

        key = ice_init(2, "HelloWorld");
        iv = build_iv();

        StTime = GetSimplePrecisionTime();
        count = ice_encrypt(key, iv, in, out, 34);
        count = ice_decrypt(key, iv, out, back, count);
        EdTime = GetSimplePrecisionTime() - StTime;

        ice_destroy(key);

        fprintf(stderr, "Ice2   : %s, Time(%lld)\n", (memcmp(in, back, 34) ? "FAIL" : "PASS"), EdTime);
    }
    {
        size_t count;
        void* key;
        INITIAL_VECTOR iv;
        long long StTime, EdTime;

        unsigned char in[34] = "_12345678123456781234567812345678_";
        unsigned char out[42];
        unsigned char back[42];

        key = tea_init(128, "HelloWorld");
        iv = build_iv();

        StTime = GetSimplePrecisionTime();
        count = tea_encrypt(key, iv, in, out, 34);
        count = tea_decrypt(key, iv, out, back, count);
        EdTime = GetSimplePrecisionTime() - StTime;

        tea_destroy(key);

        fprintf(stderr, "Tea128  : %s, Time(%lld)\n", (memcmp(in, back, 34) ? "FAIL" : "PASS"), EdTime);
    }
    {
        size_t count;
        void* key;
        INITIAL_VECTOR iv;
        long long StTime, EdTime;

        unsigned char in[34] = "_12345678123456781234567812345678_";
        unsigned char out[42];
        unsigned char back[42];

        key = tea_init(32, "HelloWorld");
        iv = build_iv();

        StTime = GetSimplePrecisionTime();
        count = tea_encrypt(key, iv, in, out, 34);
        count = tea_decrypt(key, iv, out, back, count);
        EdTime = GetSimplePrecisionTime() - StTime;

        tea_destroy(key);

        fprintf(stderr, "Tea32   : %s, Time(%lld)\n", (memcmp(in, back, 34) ? "FAIL" : "PASS"), EdTime);
    }
    {
        size_t count;
        void* key;
        INITIAL_VECTOR iv;
        long long StTime, EdTime;

        unsigned char in[34] = "_12345678123456781234567812345678_";
        unsigned char out[42];
        unsigned char back[42];

        key = aes_init(0, "HelloWorld");
        iv = build_iv();

        StTime = GetSimplePrecisionTime();
        count = aes_encrypt(key, iv, in, out, 34);
        count = aes_decrypt(key, iv, out, back, count);
        EdTime = GetSimplePrecisionTime() - StTime;

        aes_destroy(key);

        fprintf(stderr, "Aes     : %s, Time(%lld)\n", (memcmp(in, back, 34) ? "FAIL" : "PASS"), EdTime);
    }
    fprintf(stderr, "\n");
#endif

    serverArgs.nSndBuf = 8190+2+2+14;
    serverArgs.nRcvBuf = serverArgs.nSndBuf;
    serverArgs.maxActiveConnections = 16;

    while(argc > 1 && argv[1][0] == '-' && argv[1][1] == '-')
    {
        if(!strcmp("rawLog", &argv[1][2]))		serverArgs.flags.RawLog = 1;
        else
            if(!strcmp("GLLog", &argv[1][2]))		serverArgs.flags.GLLog = 1;
            else
                if(!strcmp("ProxyServer", &argv[1][2]))
                {
                    if(serverArgs.flags.ProxyClient)
                    {
                        serverArgs.flags.ProxyClient = 0; reqArgs -= 2;
                    }

                    serverArgs.flags.ProxyServer = 1; reqArgs -= 2;
                    while(1)
                    {
                        if((GetPasswd("SERVER Key -  ", keyBufferA, sizeof(keyBufferA)) ==
                            GetPasswd("\nSERVER Key -  ", keyBufferB, sizeof(keyBufferB)))
                            && !memcmp(keyBufferA, keyBufferB, sizeof(keyBufferB)))
                        {
                            memcpy(serverArgs.keys.ServerKey, keyBufferB, sizeof(keyBufferB));
                            break;
                        }
                        else
                        {
                            fputs("\n*MIS-MATCH*\n", stderr);
                        }
                    }
#if defined(_DEBUG) && defined(WIN32)
                    fputs("\n", stderr);
#endif
                }
                else if(!strcmp("ProxyClient", &argv[1][2]))
                {
                    if(serverArgs.flags.ProxyServer)
                    {
                        serverArgs.flags.ProxyServer = 0; reqArgs += 2;
                    }

                    serverArgs.flags.ProxyClient = 1; reqArgs += 2;
                    while(1)
                    {
                        if((GetPasswd("SERVER Key -  ", keyBufferA, sizeof(keyBufferA)) ==
                            GetPasswd("\nSERVER Key -  ", keyBufferB, sizeof(keyBufferB)))
                            && !memcmp(keyBufferA, keyBufferB, sizeof(keyBufferB)))
                        {
                            memcpy(serverArgs.keys.ServerKey, keyBufferB, sizeof(keyBufferB));
                            break;
                        }
                        else
                        {
                            fputs("\n*MIS-MATCH*\n", stderr);
                        }
                    }
#if defined(_DEBUG) && defined(WIN32)
                    fputs("\n", stderr);
#endif
                }
                else if(!strcmp("lzo", &argv[1][2]))
                {
                    serverArgs.flags.LZO = 1;
                    if (lzo_init() != LZO_E_OK)
                    {
                        fputs("internal error - lzo_init() failed !!!\n", stderr);
                        fputs("(this usually indicates a compiler bug - try recompiling\nwithout optimizations, and enable `-DLZO_DEBUG' for diagnostics)\n", stderr);
                        return 1;
                    }
                }
                else if((argv[1][2] == 'i') &&
                        (argv[1][3] == 'c') &&
                        (argv[1][4] == 'e'))
                {
                    serverArgs.keys.Ice.flag = strtoul(&argv[1][5], NULL, 10);
                    if((serverArgs.keys.Ice.flag == 0) || (serverArgs.keys.Ice.flag > 128)) serverArgs.keys.Ice.flag = 4;
                    serverArgs.flags.ICE = ++flags;
                    while(1)
                    {
                        if((GetPasswd("ICE Key    -  ", keyBufferA, sizeof(keyBufferA)) ==
                            GetPasswd("\nICE Key    -  ", keyBufferB, sizeof(keyBufferB)))
                            && !memcmp(keyBufferA, keyBufferB, sizeof(keyBufferB)))
                        {
                            memcpy(serverArgs.keys.Ice.key, keyBufferB, sizeof(keyBufferB));
                            break;
                        }
                        else
                        {
                            fputs("\n*MIS-MATCH*\n", stderr);
                        }
                    }
#if defined(_DEBUG) && defined(WIN32)
                    fputs("\n", stderr);
#endif
                }
                else if((argv[1][2] == 't') &&
                        (argv[1][3] == 'e') &&
                        (argv[1][4] == 'a'))
                {
                    serverArgs.keys.Tea.flag = strtoul(&argv[1][5], NULL, 10);
                    if((serverArgs.keys.Tea.flag < 8) || (serverArgs.keys.Tea.flag > 128)) serverArgs.keys.Tea.flag = 32;
                    serverArgs.flags.TEA = ++flags;
                    while(1)
                    {
                        if((GetPasswd("TEA Key    -  ", keyBufferA, sizeof(keyBufferA)) ==
                            GetPasswd("\nTEA Key    -  ", keyBufferB, sizeof(keyBufferB)))
                            && !memcmp(keyBufferA, keyBufferB, sizeof(keyBufferB)))
                        {
                            memcpy(serverArgs.keys.Tea.key, keyBufferB, sizeof(keyBufferB));
                            break;
                        }
                        else
                        {
                            fputs("\n*MIS-MATCH*\n", stderr);
                        }
                    }
#if defined(_DEBUG) && defined(WIN32)
                    fputs("\n", stderr);
#endif
                }
                else if((argv[1][2] == 'f') &&
                        (argv[1][3] == 'i') &&
                        (argv[1][4] == 's') &&
                        (argv[1][5] == 'h') )
                {
                    serverArgs.keys.Fish.flag = 0;
                    serverArgs.flags.FISH = ++flags;
                    while(1)
                    {
                        if((GetPasswd("FISH Key   -  ", keyBufferA, sizeof(keyBufferA)) ==
                            GetPasswd("\nFISH Key   -  ", keyBufferB, sizeof(keyBufferB)))
                            && !memcmp(keyBufferA, keyBufferB, sizeof(keyBufferB)))
                        {
                            memcpy(serverArgs.keys.Fish.key, keyBufferB, sizeof(keyBufferB));
                            break;
                        }
                        else
                        {
                            fputs("\n*MIS-MATCH*\n", stderr);
                        }
                    }
#if defined(_DEBUG) && defined(WIN32)
                    fputs("\n", stderr);
#endif
                }
                else if(!strcmp("statistics", &argv[1][2]))
                {
                    serverArgs.flags.Statistics = 1;
                }
                else if(!strcmp("help", &argv[1][2]))	   {DisplayDetailedUsage(stderr); return 0;}

                argc--;
                argv++;
    }

#ifdef _DEBUG
    if(serverArgs.flags.ProxyClient || serverArgs.flags.ProxyServer)
        fprintf(stderr, "ServerKey : %s\n", serverArgs.keys.ServerKey);
    if(serverArgs.flags.ICE)
        fprintf(stderr, "IceKey    : %s\n", serverArgs.keys.Ice.key);
    if(serverArgs.flags.TEA)
        fprintf(stderr, "TeaKey    : %s\n", serverArgs.keys.Tea.key);
    if(serverArgs.flags.FISH)
        fprintf(stderr, "FishKey   : %s\n", serverArgs.keys.Fish.key);
#endif

#ifdef _DEBUG
    {
        SERVER_FLAGS    flags = serverArgs.flags;
        SERVER_KEYS     keys = serverArgs.keys;

        keys.Fish.iv = build_iv();
        keys.Ice.iv = build_iv();
        keys.Tea.iv = build_iv();

        if(flags.FISH)
        {
            keys.Fish.ctx = fish_init(keys.Fish.flag, keys.Fish.key);
            if(!keys.Fish.ctx) flags.FISH = 0;
        }
        if(flags.ICE)
        {
            keys.Ice.ctx = ice_init(keys.Ice.flag, keys.Ice.key);
            if(!keys.Ice.ctx) flags.ICE = 0;
        }
        if(flags.TEA)
        {
            keys.Tea.ctx = tea_init(keys.Tea.flag, keys.Tea.key);
            if(!keys.Tea.ctx) flags.TEA = 0;
        }

        {
            char* szBuffer = (char*)malloc(1024*8);
            char* szWorkBuf = (char*)malloc(1024*8);
            char* szTmpBuf;
            int InCount, OutCount;
            long long StTime, EdTime;

            strcpy(szBuffer, "_12345678123456781234567812345678_");
            InCount = (int)strlen(szBuffer);

            StTime = GetSimplePrecisionTime();
            {
                szTmpBuf = DoEncrypt(&flags, &keys, szBuffer, szWorkBuf, InCount, &OutCount, 0, NULL);

                InCount = OutCount;
                memcpy(szBuffer, szTmpBuf, InCount);

                szTmpBuf = DoDecrypt(&flags, &keys, szBuffer, szWorkBuf, InCount, &OutCount, 0, NULL);
            }
            EdTime = GetSimplePrecisionTime() - StTime;

            fprintf(stderr, "\nTriple  : %s, Time(%lld)\n\n", (memcmp(szTmpBuf, "_12345678123456781234567812345678_", 34) ? "FAIL" : "PASS"), EdTime);
        }

        if(flags.FISH)
        {
            fish_destroy(keys.Fish.ctx);
        }
        if(flags.ICE)
        {
            ice_destroy(keys.Ice.ctx);
        }
        if(flags.TEA)
        {
            tea_destroy(keys.Tea.ctx);
        }

    }
#endif

    memset(keyBufferA, 0, sizeof(keyBufferA));
    memset(keyBufferB, 0, sizeof(keyBufferB));

    // Check for port argument
    if (argc != reqArgs)
    {
        DisplayUsage(stderr);
        return 0;
    }

    serverArgs.nServerPort = atoi(argv[1]);
    if(argc > 2)
    {
        strncpy(serverArgs.szHostName, argv[2], sizeof(serverArgs.szHostName) - 1);
        serverArgs.szHostName[sizeof(serverArgs.szHostName) - 1] = '\0';
        serverArgs.nHostPort = atoi(argv[3]);
    }
    if(argc > 4)
    {
        strncpy(serverArgs.szProxyName, argv[4], sizeof(serverArgs.szProxyName) - 1);
        serverArgs.szProxyName[sizeof(serverArgs.szProxyName) - 1] = '\0';
        serverArgs.nProxyPort = atoi(argv[5]);
    }

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
    SetConsoleCtrlHandler(proxy_signal, TRUE);
#else
    signal(SIGINT, proxy_signal);
#endif

    gFlags = serverArgs.flags;
    StreamServer(&serverArgs);

#ifdef WIN32
    SetConsoleCtrlHandler(proxy_signal, FALSE);
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
    SOCKET				listenSocket;
    struct sockaddr_in	saServer;
    int                 nRet;

    int                 bNoDelay = 1;

    listenSocket = socket(AF_INET,			// Address family
        SOCK_STREAM,		// Socket type
        IPPROTO_TCP);		// Protocol
    if (listenSocket < 0)
    {
        PRINTERROR("socket()");
        return;
    }
    // Fill in the address structure
    saServer.sin_family = AF_INET;
    saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock supply address
    saServer.sin_port = htons(serverArgs->nServerPort);		// Use port from command line

    // bind the name to the socket
    nRet = bind(listenSocket,				// Socket
        (struct sockaddr*)&saServer,		// Our address
        sizeof(struct sockaddr));	// Size of address structure

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

    DisplayServerStartUp(serverArgs->nServerPort, serverArgs->szHostName, serverArgs->nHostPort, serverArgs->szProxyName, serverArgs->nProxyPort, &serverArgs->flags, &serverArgs->keys, stderr);

    // Set the socket to listen
    nRet = listen(listenSocket,	SOMAXCONN);
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
        SOCKET	remoteSocket;
        PCLIENT_DATA threadDataIn;
        int bNoDelay = 1;
        int namelen = sizeof(threadDataIn->remoteName);

#ifdef WIN32
        HANDLE  thdHndle;
        DWORD   threadId;
#else
        pthread_t thread;
#endif

        remoteSocket = accept(listenSocket,			// Listening socket
            NULL,					// Optional client address
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
        if(serverArgs->flags.RawLog || serverArgs->flags.Statistics)
        {
            sprintf(threadDataIn->szInLog, "in%s.log", threadDataIn->szRemoteName);
            sprintf(threadDataIn->szOutLog, "out%s.log", threadDataIn->szRemoteName);
            threadDataIn->inLog = fopen(threadDataIn->szInLog, "ab");
            threadDataIn->outLog = fopen(threadDataIn->szOutLog, "ab");
        }
        if(serverArgs->flags.GLLog)
        {
            sprintf(threadDataIn->szGLLog, "feed%s.data", threadDataIn->szRemoteName);
            threadDataIn->glLog = fopen(threadDataIn->szGLLog, "ab");
        }
        threadDataIn->nSndBuf = serverArgs->nSndBuf;
        threadDataIn->nRcvBuf = serverArgs->nRcvBuf;

        memcpy(threadDataIn->szHostName, serverArgs->szHostName, sizeof(threadDataIn->szHostName));
        threadDataIn->nHostPort = serverArgs->nHostPort;

        memcpy(threadDataIn->szProxyName, serverArgs->szProxyName, sizeof(threadDataIn->szProxyName));
        threadDataIn->nProxyPort = serverArgs->nProxyPort;

        threadDataIn->flags = serverArgs->flags;
        threadDataIn->keys = serverArgs->keys;

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

#ifdef WIN32
    shutdown(listenSocket, SD_BOTH);
    closesocket(listenSocket);
#else
    shutdown(listenSocket, SHUT_RDWR);
    close(listenSocket);
#endif

    return;
}

PCLIENT_DATA ConnectToHostServer(PCLIENT_DATA threadData)
{
    int                 nRet;
    int                 bNoDelay = 1;
    struct hostent *	entHost;
    struct sockaddr_in	saHost;

    PCLIENT_DATA threadDataCpy = NULL;

    threadData->hostSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (threadData->hostSocket < 0)
    {
        PRINTERROR("socket()");
#ifdef WIN32
        shutdown(threadData->remoteSocket, SD_BOTH);
        closesocket(threadData->remoteSocket);
#else
        shutdown(threadData->remoteSocket, SHUT_RDWR);
        close(threadData->remoteSocket);
#endif
        return threadDataCpy;
    }

    if(threadData->flags.ProxyClient)
        entHost = gethostbyname(threadData->szProxyName);
    else
        entHost = gethostbyname(threadData->szHostName);

    if(entHost == NULL)
    {
        PRINTERROR("gethostbyname()");
#ifdef WIN32
        shutdown(threadData->remoteSocket, SD_BOTH);
        closesocket(threadData->remoteSocket);
#else
        shutdown(threadData->remoteSocket, SHUT_RDWR);
        close(threadData->remoteSocket);
#endif
        return threadDataCpy;
    }

    saHost.sin_family = AF_INET;
    memcpy(&saHost.sin_addr.s_addr, entHost->h_addr, entHost->h_length);

    if(threadData->flags.ProxyClient)
        saHost.sin_port = htons(threadData->nProxyPort);
    else
        saHost.sin_port = htons(threadData->nHostPort);

    nRet = setsockopt(threadData->hostSocket, IPPROTO_TCP,TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay));
    nRet = setsockopt(threadData->hostSocket, SOL_SOCKET,SO_SNDBUF, (char *)&threadData->nSndBuf,sizeof(threadData->nSndBuf));
    nRet = setsockopt(threadData->hostSocket, SOL_SOCKET,SO_RCVBUF, (char *)&threadData->nRcvBuf,sizeof(threadData->nRcvBuf));

    if(connect(threadData->hostSocket, (struct sockaddr*)&saHost, sizeof(saHost)) < 0)
    {
        PRINTERROR("connect()");
#ifdef WIN32
        shutdown(threadData->remoteSocket, SD_BOTH);
        closesocket(threadData->remoteSocket);
#else
        shutdown(threadData->remoteSocket, SHUT_RDWR);
        close(threadData->remoteSocket);
#endif
        return threadDataCpy;
    }

    threadDataCpy = (PCLIENT_DATA)malloc(sizeof(CLIENT_DATA));

    memcpy(threadDataCpy, threadData, sizeof(CLIENT_DATA));

    return threadDataCpy;
}

void AddStatistics(PSERVER_STATS to, PSERVER_STATS from)
{
    if(to && from)
    {
        to->LzoIn += from->LzoIn;
        to->LzoOut += from->LzoOut;

        to->FishIn += from->FishIn;
        to->FishOut += from->FishOut;

        to->IceIn += from->IceIn;
        to->IceOut += from->IceOut;

        to->TeaIn += from->TeaIn;
        to->TeaOut += from->TeaOut;

        to->RecvBytes += from->RecvBytes;
        to->SendBytes += from->SendBytes;
    }
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
    int lineBreak = 0;
    int err = 0;
    CRC checkSum;
    int byteOffset = 0;
    int bytesLeft = 0;
    char* bytes = NULL;
    char* szTmpBuf = NULL;
    SERVER_STATS stats;

    int nCount = data->nRcvBuf * 4 + 1;
    int overhead = data->nRcvBuf; //data->nRcvBuf / 16 + 64 + 3;
    char* szBuffer = (char*)malloc(nCount);
    char* szCmpzBuffer = (char*)malloc(data->nRcvBuf + overhead + 1024);
    char* szRcvBuffer = &szCmpzBuffer[overhead + 512];
    lzo_bytep wrkmem = NULL;
    int nRcvCount;
    int nCmpzCount;

    PCLIENT_DATA threadDataOut = NULL;

    if(data->flags.LZO) wrkmem = malloc(LZO1X_1_MEM_COMPRESS);

    memset(&stats, 0, sizeof(stats));

    LockLock(lock);
    LogConnection(1, data->szRemoteName, "INCOMING", data->id, activeConnections, stderr);
    LockUnlock(lock);

    if(data->flags.FISH)
    {
        data->keys.Fish.ctx = fish_init(data->keys.Fish.flag, data->keys.Fish.key);
        if(!data->keys.Fish.ctx) data->flags.FISH = 0;
    }
    if(data->flags.ICE)
    {
        data->keys.Ice.ctx = ice_init(data->keys.Ice.flag, data->keys.Ice.key);
        if(!data->keys.Ice.ctx) data->flags.ICE = 0;
    }
    if(data->flags.TEA)
    {
        data->keys.Tea.ctx = tea_init(data->keys.Tea.flag, data->keys.Tea.key);
        if(!data->keys.Tea.ctx) data->flags.TEA = 0;
    }

    do
    {
        if(data->flags.ProxyServer)
        {
            CLIENT_LOGON logon;
            char         logonBuff[sizeof(logon)+8];

            nRcvCount = 4;

            byteOffset = 0;
            bytesLeft = nRcvCount;
            bytes = (char*)&logon;
            while(bytesLeft)
            {
                nRet = recv(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                err = errno;
                if(nRet <= 0) {lineBreak = __LINE__; break;}
                byteOffset += nRet;
                bytesLeft -= nRet;
            }
            if(bytesLeft) {lineBreak = __LINE__; break;}

            if(memcmp(logon.header, magic, 4))
            {
                break;
            }

            nRcvCount = (sizeof(logon) + 4);

            byteOffset = 0;
            bytesLeft = nRcvCount;
            bytes = logonBuff;
            while(bytesLeft)
            {
                nRet = recv(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                err = errno;
                if(nRet <= 0) {lineBreak = __LINE__; break;}
                byteOffset += nRet;
                bytesLeft -= nRet;
            }
            if(bytesLeft) {lineBreak = __LINE__; break;}

            {
                INITIAL_VECTOR iv;

                void* key = tea_init(128, data->keys.ServerKey);
                memset(&iv, 0, sizeof(iv));

                nRcvCount = (int)tea_decrypt(key, iv, logonBuff, &logon.header[4], (sizeof(logon) + 4));

                tea_destroy(key);
            }

            if(!memcmp(&logon.header[4], &magic[4], (sizeof(magic) - 4)) && logon.hostLength > 0)
            {
                data->nHostPort = ntohs(logon.hostPort);
                memcpy(data->szHostName, logon.hostName, logon.hostLength);
                data->szHostName[logon.hostLength] = 0;

                data->keys.Fish.iv.u.low = ntohl(logon.Fish.u.low);
                data->keys.Fish.iv.u.high = ntohl(logon.Fish.u.high);

                data->keys.Ice.iv.u.low = ntohl(logon.Ice.u.low);
                data->keys.Ice.iv.u.high = ntohl(logon.Ice.u.high);

                data->keys.Tea.iv.u.low = ntohl(logon.Tea.u.low);
                data->keys.Tea.iv.u.high = ntohl(logon.Tea.u.high);
            }
            else
            {
                break;
            }

            LockLock(lock);
            LogProxy(data->szRemoteName, "ProxyConnectionRecv", data->szHostName, data->nHostPort, data->id, stderr);
            LockUnlock(lock);
        }

        if(threadDataOut = ConnectToHostServer(data))
        {
#ifdef WIN32
            HANDLE  thdHndle;
            DWORD   threadId;
#else
            pthread_t thread;
#endif

            if(data->flags.ProxyClient)
            {
                CLIENT_LOGON    logon;
                char            logonBuff[sizeof(logon)+8];
                INITIAL_VECTOR  iv;

                memcpy(logon.header, magic, sizeof(magic));
                logon.hostPort = htons(data->nHostPort);
                logon.hostLength = (char)strlen(data->szHostName);
                memset(logon.hostName, 0, sizeof(logon.hostName));
                memcpy(logon.hostName, data->szHostName, logon.hostLength);

                iv = data->keys.Fish.iv = threadDataOut->keys.Fish.iv = build_iv();
                logon.Fish.u.low = htonl(iv.u.low);
                logon.Fish.u.high = htonl(iv.u.high);

                iv = data->keys.Ice.iv = threadDataOut->keys.Ice.iv = build_iv();
                logon.Ice.u.low = htonl(iv.u.low);
                logon.Ice.u.high = htonl(iv.u.high);

                iv = data->keys.Tea.iv = threadDataOut->keys.Tea.iv = build_iv();
                logon.Tea.u.low = htonl(iv.u.low);
                logon.Tea.u.high = htonl(iv.u.high);

                {
                    INITIAL_VECTOR iv;

                    void* key = tea_init(128, data->keys.ServerKey);
                    memset(&iv, 0, sizeof(iv));

                    memcpy(logonBuff, &logon, 4);
                    nRcvCount = 4;

                    nRcvCount += (int)tea_encrypt(key, iv, &logon.header[4], &logonBuff[4], (sizeof(logon) - 4));

                    tea_destroy(key);
                }

                byteOffset = 0;
                bytesLeft = nRcvCount;
                bytes = logonBuff;
                while(bytesLeft)
                {
                    nRet = send(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
                    err = errno;
                    if(nRet <= 0) {lineBreak = __LINE__; break;}
                    byteOffset += nRet;
                    bytesLeft -= nRet;
                }
                if(bytesLeft) {lineBreak = __LINE__; break;}

                LockLock(lock);
                LogProxy(data->szRemoteName, "ProxyConnectionSent", data->szHostName, data->nHostPort, data->id, stderr);
                LockUnlock(lock);
            }


#ifdef WIN32
            thdHndle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&ClientOutgoingThreadProc, threadDataOut, 0, &threadId);
            CloseHandle(thdHndle);
#else
            pthread_create(&thread, NULL, ClientOutgoingThreadProc, threadDataOut);
            pthread_detach(thread);
#endif
        }
        else
        {
            break;
        }

        while(1)
        {
            CLIENT_PACKET packet;
            SERVER_STATS tmpStats;
            memset(&tmpStats, 0, sizeof(tmpStats));

            if(data->flags.ProxyServer && (data->flags.LZO || data->flags.FISH || data->flags.ICE || data->flags.TEA))
            {
                nRcvCount = sizeof(packet);

                byteOffset = 0;
                bytesLeft = nRcvCount;
                bytes = (char*)&packet;
                while(bytesLeft)
                {
                    nRet = recv(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                    err = errno;
                    if(nRet <= 0) {lineBreak = __LINE__; break;}
                    byteOffset += nRet;
                    bytesLeft -= nRet;
                }
                if(bytesLeft) {lineBreak = __LINE__; break;}

                nRcvCount = ntohs(packet.length);
                checkSum = ntohl(packet.checkSum);

                byteOffset = 0;
                bytesLeft = nRcvCount;
                bytes = szRcvBuffer;
                while(bytesLeft)
                {
                    nRet = recv(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                    err = errno;
                    if(nRet <= 0) {lineBreak = __LINE__; break;}
                    byteOffset += nRet;
                    bytesLeft -= nRet;
                }
                if(bytesLeft) {lineBreak = __LINE__; break;}

                tmpStats.RecvBytes = byteOffset;

                if(data->flags.LZO)
                {
                    szTmpBuf = DoDecrypt(&data->flags, &data->keys, szRcvBuffer, szCmpzBuffer, nRcvCount, &bytesLeft, 1, &tmpStats);

                    nCmpzCount = data->nRcvBuf;

                    if(data->flags.Statistics) tmpStats.StTime = GetSimplePrecisionTime();
                    if(szTmpBuf == szRcvBuffer)
                    {
                        lzo1x_decompress_safe(szRcvBuffer, bytesLeft, szCmpzBuffer, (lzo_uint*)&nCmpzCount, wrkmem);
                        szTmpBuf = szCmpzBuffer;
                        bytesLeft = nCmpzCount;
                    }
                    else
                    {
                        lzo1x_decompress_safe(szCmpzBuffer, bytesLeft, szRcvBuffer, (lzo_uint*)&nCmpzCount, wrkmem);
                        szTmpBuf = szRcvBuffer;
                        bytesLeft = nCmpzCount;
                    }
                    if(data->flags.Statistics) tmpStats.LzoIn = GetSimplePrecisionTime() - tmpStats.StTime;

                    if(data->flags.RawLog) LogRawData(szTmpBuf, bytesLeft, data->inLog, szBuffer);
                }
                else
                {
                    szTmpBuf = DoDecrypt(&data->flags, &data->keys, szRcvBuffer, szCmpzBuffer, nRcvCount, &bytesLeft, 1, &tmpStats);
                    if(data->flags.RawLog) LogRawData(szTmpBuf, bytesLeft, data->inLog, szBuffer);
                }

                if(checkSum != sum(0, szTmpBuf, bytesLeft))
                {
                    break;
                }

                byteOffset = 0;
                //bytesLeft = [See Above];
                bytes = szTmpBuf;
                while(bytesLeft)
                {
                    nRet = send(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
                    err = errno;
                    if(nRet <= 0) {lineBreak = __LINE__; break;}
                    byteOffset += nRet;
                    bytesLeft -= nRet;
                }
                if(bytesLeft) {lineBreak = __LINE__; break;}

                tmpStats.SendBytes = byteOffset;
            }
            else
            {
                nRcvCount = data->nRcvBuf;
                nRet = recv(data->remoteSocket, szRcvBuffer, nRcvCount, 0);
                err = errno;
                if(nRet <= 0) break;
                nRcvCount = nRet;
                tmpStats.RecvBytes = nRcvCount;

                if(data->flags.RawLog) LogRawData(szRcvBuffer, nRcvCount, data->inLog, szBuffer);

                if(data->flags.ProxyClient && (data->flags.LZO || data->flags.FISH || data->flags.ICE || data->flags.TEA))
                {
                    packet.checkSum = htonl(sum(0, szRcvBuffer, nRcvCount));

                    if(data->flags.LZO)
                    {
                        if(data->flags.Statistics) tmpStats.StTime = GetSimplePrecisionTime();
                        lzo1x_1_compress(szRcvBuffer, nRcvCount, szCmpzBuffer, (lzo_uint*)&nCmpzCount, wrkmem);
                        if(data->flags.Statistics) tmpStats.LzoIn = GetSimplePrecisionTime() - tmpStats.StTime;

                        szTmpBuf = DoEncrypt(&data->flags, &data->keys, szCmpzBuffer, szRcvBuffer, nCmpzCount, &nCmpzCount, 1, &tmpStats);
                    }
                    else
                    {
                        szTmpBuf = DoEncrypt(&data->flags, &data->keys, szRcvBuffer, szCmpzBuffer, nRcvCount, &nCmpzCount, 1, &tmpStats);
                    }

                    memcpy(packet.header, magic, sizeof(magic));
                    packet.length = htons(nCmpzCount);

                    byteOffset = 0;
                    bytesLeft = sizeof(packet);
                    bytes = (char*)&packet;
                    while(bytesLeft)
                    {
                        nRet = send(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
                        err = errno;
                        if(nRet <= 0) {lineBreak = __LINE__; break;}
                        byteOffset += nRet;
                        bytesLeft -= nRet;
                    }
                    if(bytesLeft) {lineBreak = __LINE__; break;}

                    byteOffset = 0;
                    bytesLeft = nCmpzCount;
                    bytes = szTmpBuf;
                    while(bytesLeft)
                    {
                        nRet = send(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
                        err = errno;
                        if(nRet <= 0) {lineBreak = __LINE__; break;}
                        byteOffset += nRet;
                        bytesLeft -= nRet;
                    }
                    if(bytesLeft) {lineBreak = __LINE__; break;}

                    tmpStats.SendBytes = byteOffset;
                }
                else
                {
                    byteOffset = 0;
                    bytesLeft = nRcvCount;
                    bytes = szRcvBuffer;
                    while(bytesLeft)
                    {
                        nRet = send(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
                        err = errno;
                        if(nRet <= 0) {lineBreak = __LINE__; break;}
                        byteOffset += nRet;
                        bytesLeft -= nRet;
                    }
                    if(bytesLeft) {lineBreak = __LINE__; break;}

                    tmpStats.SendBytes = byteOffset;
                }
            }
            if(data->flags.Statistics)
            {
                AddStatistics(&stats, &tmpStats);
                LockLock(lock);
                AddStatistics(&gStats, &tmpStats);
                LockUnlock(lock);
            }
        }
    }
    while(0);

    LockLock(lock);
    activeConnections--;
    LogConnection(0, data->szRemoteName, "INCOMING", data->id, activeConnections, stderr);
    if(data->flags.Statistics) LogSocketStatistics(data->szRemoteName, &stats, data->id, data->inLog);
    if(data->flags.Statistics && data->flags.LZO) LogLzoStatistics(data->szRemoteName, &stats, data->id, data->inLog);
    if(data->flags.Statistics && data->flags.FISH) LogFishStatistics(data->szRemoteName, &stats, data->id, data->inLog);
    if(data->flags.Statistics && data->flags.ICE) LogIceStatistics(data->szRemoteName, &stats, data->id, data->inLog);
    if(data->flags.Statistics && data->flags.TEA) LogTeaStatistics(data->szRemoteName, &stats, data->id, data->inLog);
    LockUnlock(lock);

    if(data->inLog) fclose(data->inLog);

    if(data->flags.FISH)
    {
        fish_destroy(data->keys.Fish.ctx);
    }
    if(data->flags.ICE)
    {
        ice_destroy(data->keys.Ice.ctx);
    }
    if(data->flags.TEA)
    {
        tea_destroy(data->keys.Tea.ctx);
    }

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
    free(szCmpzBuffer);

    if(data->flags.LZO) free(wrkmem);

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
    int lineBreak = 0;
    int err = 0;
    CRC checkSum;
    int byteOffset = 0;
    int bytesLeft = 0;
    char* bytes = NULL;
    char* szTmpBuf = NULL;
    SERVER_STATS stats;

    int nCount   = data->nSndBuf * 4 + 1;
    int overhead = data->nSndBuf; //data->nSndBuf / 16 + 64 + 3;
    char* szBuffer = (char*)malloc(nCount);
    char* szCmpzBuffer = (char*)malloc(data->nSndBuf + overhead + 1024);
    char* szSndBuffer = &szCmpzBuffer[overhead + 512];
    lzo_bytep wrkmem = NULL;
    int nSndCount;
    int nCmpzCount;

    if(data->flags.LZO) wrkmem = malloc(LZO1X_1_MEM_COMPRESS);

    memset(&stats, 0, sizeof(stats));

    LockLock(lock);
    LogConnection(1, data->szRemoteName, "OUTGOING", data->id, activeConnections, stderr);
    LockUnlock(lock);

    if(data->flags.FISH)
    {
        data->keys.Fish.ctx = fish_init(data->keys.Fish.flag, data->keys.Fish.key);
        if(!data->keys.Fish.ctx) data->flags.FISH = 0;
    }
    if(data->flags.ICE)
    {
        data->keys.Ice.ctx = ice_init(data->keys.Ice.flag, data->keys.Ice.key);
        if(!data->keys.Ice.ctx) data->flags.ICE = 0;
    }
    if(data->flags.TEA)
    {
        data->keys.Tea.ctx = tea_init(data->keys.Tea.flag, data->keys.Tea.key);
        if(!data->keys.Tea.ctx) data->flags.TEA = 0;
    }

    do
    {
        while(1)
        {
            CLIENT_PACKET packet;
            SERVER_STATS tmpStats;
            memset(&tmpStats, 0, sizeof(tmpStats));

            if(data->flags.ProxyClient && (data->flags.LZO || data->flags.FISH || data->flags.ICE || data->flags.TEA))
            {
                nSndCount = sizeof(packet);

                byteOffset = 0;
                bytesLeft = nSndCount;
                bytes = (char*)&packet;
                while(bytesLeft)
                {
                    nRet = recv(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
                    err = errno;
                    if(nRet <= 0) {lineBreak = __LINE__; break;}
                    byteOffset += nRet;
                    bytesLeft -= nRet;
                }
                if(bytesLeft) {lineBreak = __LINE__; break;}

                nSndCount = ntohs(packet.length);
                checkSum = ntohl(packet.checkSum);

                byteOffset = 0;
                bytesLeft = nSndCount;
                bytes = szSndBuffer;
                while(bytesLeft)
                {
                    nRet = recv(data->hostSocket, &bytes[byteOffset], bytesLeft, 0);
                    err = errno;
                    if(nRet <= 0) {lineBreak = __LINE__; break;}
                    byteOffset += nRet;
                    bytesLeft -= nRet;
                }
                if(bytesLeft) {lineBreak = __LINE__; break;}

                tmpStats.RecvBytes = byteOffset;

                if(data->flags.LZO)
                {
                    szTmpBuf = DoDecrypt(&data->flags, &data->keys, szSndBuffer, szCmpzBuffer, nSndCount, &bytesLeft, -1, &tmpStats);

                    nCmpzCount = data->nSndBuf;

                    if(data->flags.Statistics) tmpStats.StTime = GetSimplePrecisionTime();
                    if(szTmpBuf == szSndBuffer)
                    {
                        lzo1x_decompress_safe(szSndBuffer, bytesLeft, szCmpzBuffer, (lzo_uint*)&nCmpzCount, wrkmem);
                        szTmpBuf = szCmpzBuffer;
                        bytesLeft = nCmpzCount;
                    }
                    else
                    {
                        lzo1x_decompress_safe(szCmpzBuffer, bytesLeft, szSndBuffer, (lzo_uint*)&nCmpzCount, wrkmem);
                        szTmpBuf = szSndBuffer;
                        bytesLeft = nCmpzCount;
                    }
                    if(data->flags.Statistics) tmpStats.LzoOut = GetSimplePrecisionTime() - tmpStats.StTime;

                    if(data->flags.RawLog) LogRawData(szTmpBuf, bytesLeft, data->outLog, szBuffer);
                }
                else
                {
                    szTmpBuf = DoDecrypt(&data->flags, &data->keys, szSndBuffer, szCmpzBuffer, nSndCount, &bytesLeft, -1, &tmpStats);
                    if(data->flags.RawLog) LogRawData(szTmpBuf, bytesLeft, data->outLog, szBuffer);
                }

                if(checkSum != sum(0, szTmpBuf, bytesLeft))
                {
                    break;
                }

                byteOffset = 0;
                //bytesLeft = [See Above];
                bytes = szTmpBuf;
                while(bytesLeft)
                {
                    nRet = send(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                    err = errno;
                    if(nRet <= 0) {lineBreak = __LINE__; break;}
                    byteOffset += nRet;
                    bytesLeft -= nRet;
                }
                if(bytesLeft) {lineBreak = __LINE__; break;}

                tmpStats.SendBytes = byteOffset;
            }
            else
            {
                nSndCount = data->nSndBuf;
                nRet = recv(data->hostSocket, szSndBuffer, nSndCount, 0);
                err = errno;
                if(nRet <= 0) break;
                nSndCount = nRet;
                tmpStats.RecvBytes = nSndCount;

                if(data->flags.RawLog) LogRawData(szSndBuffer, nSndCount, data->outLog, szBuffer);
                WriteGLFeed(nSndCount, szSndBuffer, data->glLog);

                if(data->flags.ProxyServer && (data->flags.LZO || data->flags.FISH || data->flags.ICE || data->flags.TEA))
                {
                    packet.checkSum = htonl(sum(0, szSndBuffer, nSndCount));

                    if(data->flags.LZO)
                    {
                        if(data->flags.Statistics) tmpStats.StTime = GetSimplePrecisionTime();
                        lzo1x_1_compress(szSndBuffer, nSndCount, szCmpzBuffer, (lzo_uint*)&nCmpzCount, wrkmem);
                        if(data->flags.Statistics) tmpStats.LzoOut = GetSimplePrecisionTime() - tmpStats.StTime;

                        szTmpBuf = DoEncrypt(&data->flags, &data->keys, szCmpzBuffer, szSndBuffer, nCmpzCount, &nCmpzCount, -1, &tmpStats);
                    }
                    else
                    {
                        szTmpBuf = DoEncrypt(&data->flags, &data->keys, szSndBuffer, szCmpzBuffer, nSndCount, &nCmpzCount, -1, &tmpStats);
                    }

                    memcpy(packet.header, magic, sizeof(magic));
                    packet.length = htons(nCmpzCount);

                    byteOffset = 0;
                    bytesLeft = sizeof(packet);
                    bytes = (char*)&packet;
                    while(bytesLeft)
                    {
                        nRet = send(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                        err = errno;
                        if(nRet <= 0) {lineBreak = __LINE__; break;}
                        byteOffset += nRet;
                        bytesLeft -= nRet;
                    }
                    if(bytesLeft) {lineBreak = __LINE__; break;}

                    byteOffset = 0;
                    bytesLeft = nCmpzCount;
                    bytes = szTmpBuf;
                    while(bytesLeft)
                    {
                        nRet = send(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                        err = errno;
                        if(nRet <= 0) {lineBreak = __LINE__; break;}
                        byteOffset += nRet;
                        bytesLeft -= nRet;
                    }
                    if(bytesLeft) {lineBreak = __LINE__; break;}

                    tmpStats.SendBytes = byteOffset;
                }
                else
                {
                    byteOffset = 0;
                    bytesLeft = nSndCount;
                    bytes = szSndBuffer;
                    while(bytesLeft)
                    {
                        nRet = send(data->remoteSocket, &bytes[byteOffset], bytesLeft, 0);
                        err = errno;
                        if(nRet <= 0) {lineBreak = __LINE__; break;}
                        byteOffset += nRet;
                        bytesLeft -= nRet;
                    }
                    if(bytesLeft) {lineBreak = __LINE__; break;}

                    tmpStats.SendBytes = byteOffset;
                }
            }
            if(data->flags.Statistics)
            {
                AddStatistics(&stats, &tmpStats);
                LockLock(lock);
                AddStatistics(&gStats, &tmpStats);
                LockUnlock(lock);
            }
        }
    }
    while(0);

    LockLock(lock);
    LogConnection(0, data->szRemoteName, "OUTGOING", data->id, activeConnections, stderr);
    if(data->flags.Statistics) LogSocketStatistics(data->szRemoteName, &stats, data->id, data->outLog);
    if(data->flags.Statistics && data->flags.LZO) LogLzoStatistics(data->szRemoteName, &stats, data->id, data->outLog);
    if(data->flags.Statistics && data->flags.FISH) LogFishStatistics(data->szRemoteName, &stats, data->id, data->outLog);
    if(data->flags.Statistics && data->flags.ICE) LogIceStatistics(data->szRemoteName, &stats, data->id, data->outLog);
    if(data->flags.Statistics && data->flags.TEA) LogTeaStatistics(data->szRemoteName, &stats, data->id, data->outLog);
    LockUnlock(lock);

    if(data->outLog) fclose(data->outLog);
    if(data->glLog) fclose(data->glLog);

    if(data->flags.FISH)
    {
        fish_destroy(data->keys.Fish.ctx);
    }
    if(data->flags.ICE)
    {
        ice_destroy(data->keys.Ice.ctx);
    }
    if(data->flags.TEA)
    {
        tea_destroy(data->keys.Tea.ctx);
    }

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
    free(szCmpzBuffer);

    if(data->flags.LZO) free(wrkmem);

    free(data);

    return 0;
}

char* DoDecrypt(SERVER_FLAGS* flags, SERVER_KEYS* keys, char* szBuffer, char* szWorkBuf, int InCount, int* OutCount, int InOut, SERVER_STATS* stats)
{
    int idx;
    char* szTmpBuf;
    int count;
    long long StTime;

    if(!flags || !keys || !szBuffer || !szWorkBuf) return NULL;

    count = InCount;

    for(idx = 3; idx >= 1; idx--)
    {
        if(flags->FISH == idx)
        {
            if(flags->Statistics && stats && InOut) StTime = GetSimplePrecisionTime();
            count = (int)fish_decrypt(keys->Fish.ctx, keys->Fish.iv, szBuffer, szWorkBuf, count);
            if(flags->Statistics && stats && InOut)
            {
                if(InOut > 0)
                    stats->FishIn = GetSimplePrecisionTime() - StTime;
                else if(InOut < 0)
                    stats->FishOut = GetSimplePrecisionTime() - StTime;
            }
            szTmpBuf = szWorkBuf;
            szWorkBuf = szBuffer;
            szBuffer = szTmpBuf;
        }
        else if(flags->ICE == idx)
        {
            if(flags->Statistics && stats && InOut) StTime = GetSimplePrecisionTime();
            count = (int)ice_decrypt(keys->Ice.ctx, keys->Ice.iv, szBuffer, szWorkBuf, count);
            if(flags->Statistics && stats && InOut)
            {
                if(InOut > 0)
                    stats->IceIn = GetSimplePrecisionTime() - StTime;
                else if(InOut < 0)
                    stats->IceOut = GetSimplePrecisionTime() - StTime;
            }
            szTmpBuf = szWorkBuf;
            szWorkBuf = szBuffer;
            szBuffer = szTmpBuf;
        }
        else if(flags->TEA == idx)
        {
            if(flags->Statistics && stats && InOut) StTime = GetSimplePrecisionTime();
            count = (int)tea_decrypt(keys->Tea.ctx, keys->Tea.iv, szBuffer, szWorkBuf, count);
            if(flags->Statistics && stats && InOut)
            {
                if(InOut > 0)
                    stats->TeaIn = GetSimplePrecisionTime() - StTime;
                else if(InOut < 0)
                    stats->TeaOut = GetSimplePrecisionTime() - StTime;
            }
            szTmpBuf = szWorkBuf;
            szWorkBuf = szBuffer;
            szBuffer = szTmpBuf;
        }
    }

    if(OutCount) (*OutCount) = count;

    return szBuffer;
}

char* DoEncrypt(SERVER_FLAGS* flags, SERVER_KEYS* keys, char* szBuffer, char* szWorkBuf, int InCount, int* OutCount, int InOut, SERVER_STATS* stats)
{
    int idx;
    char* szTmpBuf;
    int count;
    long long StTime;

    if(!flags || !keys || !szBuffer || !szWorkBuf) return NULL;

    count = InCount;

    for(idx = 1; idx <= 3; idx++)
    {
        if(flags->FISH == idx)
        {
            if(flags->Statistics && stats && InOut) StTime = GetSimplePrecisionTime();
            count = (int)fish_encrypt(keys->Fish.ctx, keys->Fish.iv, szBuffer, szWorkBuf, count);
            if(flags->Statistics && stats && InOut)
            {
                if(InOut > 0)
                    stats->FishIn = GetSimplePrecisionTime() - StTime;
                else if(InOut < 0)
                    stats->FishOut = GetSimplePrecisionTime() - StTime;
            }
            szTmpBuf = szWorkBuf;
            szWorkBuf = szBuffer;
            szBuffer = szTmpBuf;
        }
        else if(flags->ICE == idx)
        {
            if(flags->Statistics && stats && InOut) StTime = GetSimplePrecisionTime();
            count = (int)ice_encrypt(keys->Ice.ctx, keys->Ice.iv, szBuffer, szWorkBuf, count);
            if(flags->Statistics && stats && InOut)
            {
                if(InOut > 0)
                    stats->IceIn = GetSimplePrecisionTime() - StTime;
                else if(InOut < 0)
                    stats->IceOut = GetSimplePrecisionTime() - StTime;
            }
            szTmpBuf = szWorkBuf;
            szWorkBuf = szBuffer;
            szBuffer = szTmpBuf;
        }
        else if(flags->TEA == idx)
        {
            if(flags->Statistics && stats && InOut) StTime = GetSimplePrecisionTime();
            count = (int)tea_encrypt(keys->Tea.ctx, keys->Tea.iv, szBuffer, szWorkBuf, count);
            if(flags->Statistics && stats && InOut)
            {
                if(InOut > 0)
                    stats->TeaIn = GetSimplePrecisionTime() - StTime;
                else if(InOut < 0)
                    stats->TeaOut = GetSimplePrecisionTime() - StTime;
            }
            szTmpBuf = szWorkBuf;
            szWorkBuf = szBuffer;
            szBuffer = szTmpBuf;
        }
    }

    if(OutCount) (*OutCount) = count;

    return szBuffer;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <fcntl.h>
#include <time.h>
#else
#define SOCKET int
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#endif

#include <codage.h>
#include <ConvertString.h>

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

int activeConnections = 0;
int maxActiveConnections = 16;

typedef struct _stream_client_thread_data
{
    SOCKET remoteSocket;
    char* szDataFile;
    int nTempo;
    int nRecord;
    int nBinary;
} STREAM_CLIENT_THREAD_DATA, *PSTREAM_CLIENT_THREAD_DATA;

// Function prototype
void StreamServer(short nPort, char* szDataFile, int nSndBuf, int nRcvBuf, int nTempo, int nRecord, int nBinary);
void StreamClient(SOCKET remoteSocket, char* szDataFile, int nTempo, int nRecord, int nBinary);
void GetLocalTimeAsString(char* szBuffer);
SOCKET ConnectToHostServer(char* szHostName, short nHostPort, int nSndBuf, int nRcvBuf);

#ifdef WIN32
DWORD WINAPI StreamClientThreadProc(PSTREAM_CLIENT_THREAD_DATA lpThreadData);
#else

#ifdef  __cplusplus
extern "C" {
#endif

void* StreamClientThreadProc(void* data);

#ifdef  __cplusplus
}
#endif

#endif

// Helper macro for displaying errors
#ifdef WIN32
#define PRINTERROR(s)	\
    fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())
#else
#define PRINTERROR(s)	\
    fprintf(stderr,"\n%s: %d\n", s, errno)
#endif

#define BREAKERROR_1(s1)	\
		{fprintf(stderr,s1); break;}
#define BREAKERROR_2(s1, s2)	\
		{fprintf(stderr,s1, s2); break;}

////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
#ifdef WIN32
    WORD         wVersionRequested = MAKEWORD(1,1);
    WSADATA      wsaData;
#endif
	int nRet;
	int nSndBuf = 1024;
	int nRcvBuf = 1024;
    int nTempo = 0;
	int nRecord = 0;
	int nBinary = 0;
	short nPort;
	char* szDataFile;
	char* szHostName;

#ifdef WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stderr), _O_BINARY);
#else
	signal(SIGPIPE, SIG_IGN);
#endif

	if((argc > 1) &&
	   (argv[1][0] == '-') &&
	   (argv[1][1] == '-') &&
	   (argv[1][2] == 'r') &&
	   (argv[1][3] == 'e') &&
	   (argv[1][4] == 'c') &&
	   (argv[1][5] == 'o') &&
	   (argv[1][6] == 'r') &&
	   (argv[1][7] == 'd') &&
	   (argv[1][8] ==  0))
	{
		nRecord = 1;
		argc--;
		argv++;
	}
	if((argc > 1) &&
	   (argv[1][0] == '-') &&
	   (argv[1][1] == '-') &&
	   (argv[1][2] == 'b') &&
	   (argv[1][3] == 'i') &&
	   (argv[1][4] == 'n') &&
	   (argv[1][5] == 'a') &&
	   (argv[1][6] == 'r') &&
	   (argv[1][7] == 'y') &&
	   (argv[1][8] ==  0))
	{
		nBinary = 1;
		argc--;
		argv++;
	}

	//
	// Check for port argument
	//
	if (argc < 3)
	{
		fprintf(stderr,"\nUsage: TCPServer.exe {--record} {--binary} [PORT_NUMBER] {HOST_NAME} [DATA_FILE] {SO_SNDBUF} {SO_RCVBUF} {TEMPO}\n");
		return 0;
	}

	nPort = atoi(argv[1]);

	if(nRecord)
	{
		szHostName = argv[2];
		argc--;
		argv++;
	}

	szDataFile = argv[2];
    if(argc > 3) nSndBuf = atoi(argv[3]);
    if(argc > 4) nRcvBuf = atoi(argv[4]);
    if(argc > 5) nTempo = atoi(argv[5]);

	if(szDataFile[0] == '-' && szDataFile[1] == '\0') maxActiveConnections = 1;

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

	//
	// Do the stuff a stream server does
	//
	if(nRecord)
	{
		SOCKET remoteSocket;

		printf("\nClient[%s:%d]; SndBuf[%d]; RcvBuf[%d]; DataFile[%s][%d];\n", szHostName, nPort, nSndBuf, nRcvBuf, ((szDataFile[0] == '-' && szDataFile[1] == '\0') ? (nRecord ? "stdout" : "stdin") : szDataFile), nTempo);

		if(remoteSocket = ConnectToHostServer(szHostName, nPort, nSndBuf, nRcvBuf))
		{
			StreamClient(remoteSocket, szDataFile, nTempo, nRecord, nBinary);
		}
	}
	else
	{
		StreamServer(nPort, szDataFile, nSndBuf, nRcvBuf, nTempo, nRecord, nBinary);
	}

	LockDestroy(lock);

    // Release WinSock
#ifdef WIN32
    WSACleanup();
#endif

	return 0;
}

int AllocateBuffer(char** szBuffer, size_t nSize)
{
    if(!szBuffer) return 0;

	nSize *= 2;

    *szBuffer = (char*)realloc(*szBuffer, nSize);

    return (*szBuffer) ? nSize : 0;
}

////////////////////////////////////////////////////////////

void StreamServer(short nPort, char* szDataFile, int nSndBuf, int nRcvBuf, int nTempo, int nRecord, int nBinary)
{
	//
	// Create a TCP/IP stream socket to "listen" with
	//
	SOCKET	listenSocket;

	listenSocket = socket(AF_INET,			// Address family
						  SOCK_STREAM,		// Socket type
						  IPPROTO_TCP);		// Protocol
	if (listenSocket < 0)
	{
		PRINTERROR("socket()");
		return;
	}


	//
	// Fill in the address structure
	//
	struct sockaddr_in saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock supply address
	saServer.sin_port = htons(nPort);		// Use port from command line

	//
	// bind the name to the socket
	//
	int nRet;

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

    int bNoDelay = 1;
    setsockopt(listenSocket,IPPROTO_TCP,TCP_NODELAY, (char *)&bNoDelay,sizeof(bNoDelay));
    setsockopt(listenSocket,SOL_SOCKET,SO_SNDBUF, (char *)&nSndBuf,sizeof(nSndBuf));
    setsockopt(listenSocket,SOL_SOCKET,SO_RCVBUF, (char *)&nRcvBuf,sizeof(nRcvBuf));


	//
	// This isn't normally done or required, but in this
	// example we're printing out where the server is waiting
	// so that you can connect the example client.
	//
	char szBuf[256];

	nRet = gethostname(szBuf, sizeof(szBuf));
	if (nRet)
	{
		PRINTERROR("gethostname()");
#ifdef WIN32
        shutdown(listenSocket, SD_BOTH);
        closesocket(listenSocket);
#else
        shutdown(listenSocket, SHUT_RDWR);
        close(listenSocket);
#endif
		return;
	}

	//
	// Show the server name and port number
	//
	printf("\nServer[%s:%d]; SndBuf[%d]; RcvBuf[%d]; DataFile[%s][%d];\n", szBuf, nPort, nSndBuf, nRcvBuf, ((szDataFile[0] == '-' && szDataFile[1] == '\0') ? (nRecord ? "stdout" : "stdin") : szDataFile), nTempo);

	//
	// Set the socket to listen
	//

	nRet = listen(listenSocket,					// Bound socket
				  SOMAXCONN);					// Number of connection request queue
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

	//
	// Wait for an incoming request
	//

    while(true)
    {
        SOCKET	remoteSocket;

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
		if(activeConnections >= maxActiveConnections)
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

        STREAM_CLIENT_THREAD_DATA* threadData = (STREAM_CLIENT_THREAD_DATA*)malloc(sizeof(STREAM_CLIENT_THREAD_DATA));

        threadData->remoteSocket = remoteSocket;
        threadData->szDataFile = szDataFile;
        threadData->nTempo = nTempo;
        threadData->nRecord = nRecord;
        threadData->nBinary = nBinary;

		LockLock(lock);
		++activeConnections;
		LockUnlock(lock);

#ifdef WIN32
		thdHndle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&StreamClientThreadProc, threadData, 0, &threadId);
		CloseHandle(thdHndle);
#else
		pthread_create(&thread, NULL, StreamClientThreadProc, threadData);
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

    printf("\n\n");

	return;
}

#ifdef WIN32
DWORD WINAPI StreamClientThreadProc(PSTREAM_CLIENT_THREAD_DATA lpThreadData)
{
#else
void* StreamClientThreadProc(void* pVoid)
{
    PSTREAM_CLIENT_THREAD_DATA lpThreadData = (PSTREAM_CLIENT_THREAD_DATA)pVoid;
#endif

    if(lpThreadData)
    {
		StreamClient(lpThreadData->remoteSocket, lpThreadData->szDataFile, lpThreadData->nTempo, lpThreadData->nRecord, lpThreadData->nBinary);
		free(lpThreadData);
	}

    return 0;
}

void StreamClient(SOCKET remoteSocket, char* szDataFile, int nTempo, int nRecord, int nBinary)
{
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    char szRemoteName[128];
	char szCurrentTime[32];

	int nRet;
    int bNoDelay = 1;
    nRet = setsockopt(remoteSocket,IPPROTO_TCP,TCP_NODELAY, (char *)&bNoDelay,sizeof(bNoDelay));
    nRet = getpeername(remoteSocket, (struct sockaddr*)&name, &namelen);
	sprintf(szRemoteName, "[%s][%d]", inet_ntoa(name.sin_addr), name.sin_port);

	GetLocalTimeAsString(szCurrentTime);

	if(nRecord)
	{
		size_t nRecvCount = 0;

		printf("\n%s : Server%s-->> {Connection Open}\n", szCurrentTime, szRemoteName);

		do
		{
			size_t nBufSize = 1024*8;
			char* szBuffer = 0;
			size_t nCount = 0;

			FILE* fpDataFile = (szDataFile[0] == '-' && szDataFile[1] == '\0') ? stdout : fopen(szDataFile,"wb");

			if (!fpDataFile) BREAKERROR_2("\nInvalid filename[%s]\n", szDataFile);

			nBufSize = AllocateBuffer(&szBuffer, nBufSize);

			while(nBufSize)
			{
				nRet = recv(remoteSocket, szBuffer, nBufSize, 0);
				if(nRet <= 0)
				{
					nBufSize = 0; nTempo= 0;
					GetLocalTimeAsString(szCurrentTime);
					printf("\n%s : Server%s<<-- {Connection Closed}", szCurrentTime, szRemoteName);
					break;
				}
				nCount = nRet;
				nRecvCount++;

				if(fpDataFile == stdout)
				{
					if(nBinary)
					{
						PrintString(szBuffer, stdout, nCount);
					}
					else
					{
						fwrite(szBuffer, 1, nCount, stdout);
					}
					fputs("\n", stdout);
				}
				else
				{
					Write_Msg_Feed(fpDataFile, szBuffer, (short)nCount, ' ', 1);
				}

	#ifdef WIN32
				if(nTempo) Sleep(nTempo);
	#else
				if(nTempo) usleep(nTempo * 1000);
	#endif
			}

			AllocateBuffer(&szBuffer, 0);

			if(fpDataFile != stdout) fclose(fpDataFile);
		}
		while(false);

		printf("\nRecvFromCount[%d]\n", nRecvCount);
	}
	else
	{
		size_t nSendCount = 0;

		printf("\n%s : Client%s-->> {Connection Open}", szCurrentTime, szRemoteName);

		do
		{
			size_t nBufSize = 1024*8;
			char* szBuffer = 0;
			char* szTmpBuffer = 0;
			size_t nCount = 0;

			FILE* fpDataFile = (szDataFile[0] == '-' && szDataFile[1] == '\0') ? stdin : fopen(szDataFile,"rb");

			if (!fpDataFile) BREAKERROR_2("\nInvalid filename[%s]\n", szDataFile);

			if(fpDataFile == stdin) fputs("\n", stdout);

			nBufSize = AllocateBuffer(&szBuffer, nBufSize);
			while(nBufSize)
			{
				if(fpDataFile == stdin)
				{
					if(!fgets(szBuffer, nBufSize, stdin))
						break;

					nCount = strlen(szBuffer);
					if(!nCount)
						break;

					while(nCount && (szBuffer[nCount-1] == '\r' || szBuffer[nCount-1] == '\n')) szBuffer[--nCount] = 0;

					if(nBinary) nCount = ConvertCString(szBuffer);

					szTmpBuffer = szBuffer;
				}
				else
				{
					nCount = 2;
					if(fread(&nCount, nCount, 1, fpDataFile) != 1)
					{
						printf("\nEnd Of File");
						break;
					}

					nCount = DecodeI2((unsigned char *)&nCount);

					if(nCount > 1024 * 32) BREAKERROR_2("\nInvalid run length in file[%s]\n", szDataFile);

					if(nCount > nBufSize)
						if(!(nBufSize = AllocateBuffer(&szBuffer, nCount)))
							BREAKERROR_1("\nFailed to allocate memory\n");

					if(fread(szBuffer, nCount, 1, fpDataFile) != 1) BREAKERROR_2("\nInvalid data format in file[%s]\n", szDataFile);

					szTmpBuffer = szBuffer + 1; nCount--;
				}

				while(nCount > 0)
				{
					nRet = send(remoteSocket, szTmpBuffer, nCount, 0);
					if(nRet < 0)
					{
						nBufSize = 0; nTempo= 0;
						GetLocalTimeAsString(szCurrentTime);
						printf("\n%s : Client%s<<-- {Connection Closed}", szCurrentTime, szRemoteName);
						break;
					}
					szTmpBuffer += nRet;
					nCount -= nRet;
					nSendCount++;
				}

	#ifdef WIN32
				if(nTempo) Sleep(nTempo);
	#else
				if(nTempo) usleep(nTempo * 1000);
	#endif
			}

			AllocateBuffer(&szBuffer, 0);
			if(fpDataFile != stdin) fclose(fpDataFile);
		}
		while(false);

		printf("\nSendToCount[%d]\n", nSendCount);
	}

	LockLock(lock);
	--activeConnections;
	LockUnlock(lock);

	//
	// Close BOTH sockets before exiting
	//
#ifdef WIN32
	shutdown(remoteSocket, SD_BOTH);
	closesocket(remoteSocket);
#else
	shutdown(remoteSocket, SHUT_RDWR);
	close(remoteSocket);
#endif

	return;
}

void GetLocalTimeAsString(char* szBuffer)
{
    struct tm localTime;
    int milliseconds;
#ifdef WIN32
    SYSTEMTIME      v_tpcur;
    GetLocalTime( &v_tpcur );
    milliseconds = v_tpcur.wMilliseconds;
    localTime.tm_sec = v_tpcur.wSecond;
    localTime.tm_min = v_tpcur.wMinute;
    localTime.tm_hour = v_tpcur.wHour;
    localTime.tm_mday = v_tpcur.wDay;
    localTime.tm_mon = v_tpcur.wMonth;
    localTime.tm_year = v_tpcur.wYear;
#else
    struct timeval tpcur;
    time_t  sysTime;
    gettimeofday(&tpcur,0);
    milliseconds=tpcur.tv_usec / 1000;
    sysTime = tpcur.tv_sec;
    localtime_r(&sysTime, &localTime);
#endif

    sprintf(szBuffer, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d.%.3d",
        localTime.tm_year+1900,
        localTime.tm_mon+1,
        localTime.tm_mday,
        localTime.tm_hour,
        localTime.tm_min,
        localTime.tm_sec,
        milliseconds);
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
        return (SOCKET)0;
    }

    entHost = gethostbyname(szHostName);

    if(entHost == NULL)
    {
        PRINTERROR("gethostbyname()");
        return (SOCKET)0;
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
        return (SOCKET)0;
    }

    return hostSocket;
}

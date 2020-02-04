#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
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
#endif

#include <codage.H>

// Function prototype
void StreamServer(short nPort, int nTempo);
void GetLocalTimeAsString(char* szBuffer);

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
    int nTempo = 0;
	short nPort;

	//
	// Check for port argument
	//
	if (argc < 3)
	{
		fprintf(stderr,"\nUsage: TCPDropServer.exe {PORT_NUMBER} [TEMPO]\n");
		return 0;
	}

	nPort = atoi(argv[1]);
    if(argc > 2) nTempo = atoi(argv[5]);

#ifdef WIN32
    // Initialize WinSock and check version
    nRet = WSAStartup(wVersionRequested, &wsaData);
    if (wsaData.wVersion != wVersionRequested)
    {
        fprintf(stderr,"\n Winsock Startup --> Wrong version, exiting\n");
        return 0;
    }
#endif

	//
	// Do the stuff a stream server does
	//
	StreamServer(nPort, nTempo);

    // Release WinSock
#ifdef WIN32
    WSACleanup();
#endif

	return 0;
}

////////////////////////////////////////////////////////////

void StreamServer(short nPort, int nTempo)
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
	printf("\nServer[%s:%d][%d];\n", szBuf, nPort, nTempo);

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

        remoteSocket = accept(listenSocket,			// Listening socket
                              NULL,					// Optional client address
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


		struct sockaddr_in name;
	    char szRemoteName[128];

		socklen_t namelen = sizeof(name);
		int nRet;
		char szCurrentTime[32];

		nRet = getpeername(remoteSocket, (struct sockaddr*)&name, &namelen);
		sprintf(szRemoteName, "[%s][%d]", inet_ntoa(name.sin_addr), name.sin_port);

		GetLocalTimeAsString(szCurrentTime);
		printf("\n%s : Client%s-->> {Connection Open}", szCurrentTime, szRemoteName);
#ifdef WIN32
		if(nTempo) Sleep(nTempo);
        shutdown(remoteSocket, SD_BOTH);
        closesocket(remoteSocket);
#else
		if(nTempo) usleep(nTempo * 1000);
        shutdown(remoteSocket, SHUT_RDWR);
        close(remoteSocket);
#endif

		GetLocalTimeAsString(szCurrentTime);
		printf("\n%s : Client%s-->> {Connection Closed}", szCurrentTime, szRemoteName);
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

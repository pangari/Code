#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include <codage.h>
#include <ConvertString.h>

// Function prototype
void DatagramServer(char* szBroadcastAddres, short nPort, char* szDataFile, int nTempo, int nRecord, int nBinary);

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
	WORD		wVersionRequested = MAKEWORD(1,1);
	WSADATA		wsaData;
#endif
	int nRet;
	char* szBroadcastAddress;
	int nTempo = 0;
	int nRecord = 0;
	int nBinary = 0;
	short nPort;
	char* szDataFile;

#ifdef WIN32
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stderr), _O_BINARY);
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
	if (argc < 4)
	{
		fprintf(stderr,"\nUsage: UDPServer.exe {--record} {--binary} [BRAODCAST_ADRESS] [PORT_NUMBER] [DATA_FILE] {TEMPO}\n");
		return 0;
	}

	szBroadcastAddress = argv[1];
	nPort = atoi(argv[2]);
	szDataFile = argv[3];
	if(argc > 4) nTempo = atoi(argv[4]);

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
	// Do all the stuff a datagram server does
	//
	DatagramServer(szBroadcastAddress, nPort, szDataFile, nTempo, nRecord, nBinary);

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

void DatagramServer(char* szBroadcastAddress, short nPort, char* szDataFile, int nTempo, int nRecord, int nBinary)
{
	//
	// Create a UDP/IP datagram socket
	//
	SOCKET theSocket;

	theSocket = socket(AF_INET,		// Address family
						SOCK_DGRAM,  // Socket type
						IPPROTO_UDP);// Protocol
	if (theSocket < 0)
	{
		PRINTERROR("socket()");
		return;
	}

	int opt = 1;
	if(setsockopt(theSocket, SOL_SOCKET, SO_BROADCAST, (char *) &opt, sizeof(opt)) < 0)
	{
		PRINTERROR("setsockopt(SO_BROADCAST)");

#ifdef WIN32
		shutdown(theSocket, SD_BOTH);
		closesocket(theSocket);
#else
		shutdown(theSocket, SHUT_RDWR);
		close(theSocket);
#endif
		return;
	}
	if(setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0)
	{
		PRINTERROR("setsockopt(SO_REUSEADDR)");

#ifdef WIN32
		shutdown(theSocket, SD_BOTH);
		closesocket(theSocket);
#else
		shutdown(theSocket, SHUT_RDWR);
		close(theSocket);
#endif
		return;
	}


	//
	// Fill in the address structure
	//
	struct sockaddr_in	saServer;

	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY; // Let WinSock assign address
	saServer.sin_port = htons(nPort);	   // Use port passed from user


	//
	// bind the name to the socket
	//
	int nRet;

	nRet = bind(theSocket,				// Socket descriptor
				(struct sockaddr*)&saServer,  // Address to bind to
				sizeof(struct sockaddr)	// Size of address
				);
	if (nRet)
	{
		PRINTERROR("bind()");

#ifdef WIN32
		shutdown(theSocket, SD_BOTH);
		closesocket(theSocket);
#else
		shutdown(theSocket, SHUT_RDWR);
		close(theSocket);
#endif
		return;
	}

	//
	// Show the server name and port number
	//
#ifdef WIN32
	int nMsgSize;
	int optlen;
	optlen = sizeof(nMsgSize);
	getsockopt(theSocket,SOL_SOCKET,SO_MAX_MSG_SIZE, (char *)&nMsgSize, &optlen);

	printf("\n%s[%s:%d]; MsgSize[%d]; DataFile[%s][%d];\n", (nRecord ? "Client" : "Server"), szBroadcastAddress, nPort, nMsgSize, ((szDataFile[0] == '-' && szDataFile[1] == '\0') ? (nRecord ? "stdout" : "stdin") : szDataFile), nTempo);
#else
	printf("\n%s[%s:%d]; DataFile[%s][%d];\n", (nRecord ? "Client" : "Server"), szBroadcastAddress, nPort, ((szDataFile[0] == '-' && szDataFile[1] == '\0') ? (nRecord ? "stdout" : "stdin") : szDataFile), nTempo);
#endif


	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(nPort);
	sa.sin_addr.s_addr = inet_addr(szBroadcastAddress);

	if(nRecord)
	{
		size_t nRecvCount = 0;

		do
		{
#ifdef WIN32
			size_t nBufSize = nMsgSize;
#else
			size_t nBufSize = 1024*8;
#endif
			char* szBuffer = 0;
			size_t nCount = 0;

			FILE* fpDataFile = (szDataFile[0] == '-' && szDataFile[1] == '\0') ? stdout : fopen(szDataFile,"wb");

			if (!fpDataFile) BREAKERROR_2("\nInvalid filename[%s]\n", szDataFile);

			nBufSize = AllocateBuffer(&szBuffer, nBufSize);

			while(nBufSize)
			{
				nRet = recvfrom(theSocket, szBuffer, nBufSize, 0, NULL, NULL);
				if(nRet < 0)
				{
					nBufSize = 0; nTempo = 0;
					PRINTERROR("recvfrom()");
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
		do
		{
#ifdef WIN32
			size_t nBufSize = nMsgSize * 4;
#else
			size_t nBufSize = 1024*8*4;
#endif
			char* szBuffer = 0;
			char* szTmpBuffer = 0;
			size_t nCount = 0;

			FILE* fpDataFile = (szDataFile[0] == '-' && szDataFile[1] == '\0') ? stdin : fopen(szDataFile,"rb");

			if (!fpDataFile) BREAKERROR_2("\nInvalid filename[%s]\n", szDataFile);

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

					if((nCount * 4) > nBufSize)
						if(!(nBufSize = AllocateBuffer(&szBuffer, nCount * 4)))
							BREAKERROR_1("\nFailed to allocate memory\n");

					if(fread(szBuffer, nCount, 1, fpDataFile) != 1) BREAKERROR_2("\nInvalid data format in file[%s]\n", szDataFile);

					szTmpBuffer = szBuffer + 1; nCount--;
				}

				while(nCount > 0)
				{
					nRet = sendto(theSocket, szTmpBuffer, nCount, 0, (struct sockaddr *)&sa, sizeof(sa));
					if(nRet < 0)
					{
						nBufSize = 0; nTempo= 0;
						PRINTERROR("sendto()");
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

#ifdef WIN32
	shutdown(theSocket, SD_BOTH);
	closesocket(theSocket);
#else
	shutdown(theSocket, SHUT_RDWR);
	close(theSocket);
#endif
	return;
}

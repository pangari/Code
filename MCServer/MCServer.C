#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

#include <codage.H>
#include <ConvertString.H>

// Function prototype
void MulticastServer(char* szBroadcastAddres, char* szDataFile, int nSndBuf, int nRcvBuf, int nTempo, int nRecord, int nBinary);

// Helper macro for displaying errors
#ifdef WIN32
#define PRINTERROR(s)   \
    fprintf(stderr,"\n%s: %d\n", s, WSAGetLastError())
#else
#define PRINTERROR(s)   \
    fprintf(stderr,"\n%s: %d\n", s, errno)
#endif

#define BREAKERROR_1(s1)    \
{fprintf(stderr,s1); goto CLEANUP;}
#define BREAKERROR_2(s1, s2)    \
{fprintf(stderr,s1, s2); goto CLEANUP;}

////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
#ifdef WIN32
    WORD        wVersionRequested = MAKEWORD(2,2);
    WSADATA     wsaData;
#endif
    int nRet;
    char* szMulticastAddress;
    int nTempo = 0;
    int nRecord = 0;
    int nBinary = 0;
    char* szDataFile;
    int nSndBuf = 1024*128;
    int nRcvBuf = 1024*128;

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
    if (argc < 3)
    {
        fprintf(stderr,"\nUsage: MCServer {--record} {--binary} [MULTICAST_ADRESS:PORT{-INTERFACE},...] [DATA_FILE] {SO_SNDBUF} {SO_RCVBUF} {TEMPO}\n\n");
        return 0;
    }

    szMulticastAddress = argv[1];
    szDataFile = argv[2];
    if(argc > 3) nSndBuf = atoi(argv[3]);
    if(argc > 4) nRcvBuf = atoi(argv[4]);
    if(argc > 5) nTempo = atoi(argv[5]);

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
    // Do all the stuff a Multicast server does
    //
    MulticastServer(szMulticastAddress, szDataFile, nSndBuf, nRcvBuf, nTempo, nRecord, nBinary);

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
    return (*szBuffer) ? (int)nSize : 0;
}

////////////////////////////////////////////////////////////

int replaceByte(char* buffer, size_t size, char in, char out)
{
    int count = 0;

    while(size--)
    {
        if(buffer[size] == in)
        {
            buffer[size] = out;
            count++;
        }
    }
    return count;
}

SOCKET CreateAndBindSocket(unsigned short nPort, int nSndBuf, int nRcvBuf, char* DestInter)
{
    //
    // Create a UDP/IP Multicast socket
    //
    SOCKET newSock;
    int nRet;

    newSock = socket(AF_INET,     // Address family
        SOCK_DGRAM,  // Socket type
        0);// Protocol
    if (newSock < 0)
    {
        PRINTERROR("socket()");
        return (SOCKET)0;
    }

    int opt = 1;
    if(setsockopt(newSock, SOL_SOCKET, SO_BROADCAST, (char *) &opt, sizeof(opt)) < 0)
    {
        PRINTERROR("setsockopt(SO_BROADCAST)");

#ifdef WIN32
        shutdown(newSock, SD_BOTH);
        closesocket(newSock);
#else
        shutdown(newSock, SHUT_RDWR);
        close(newSock);
#endif
        return (SOCKET)0;
    }
    if(setsockopt(newSock, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0)
    {
        PRINTERROR("setsockopt(SO_REUSEADDR)");

#ifdef WIN32
        shutdown(newSock, SD_BOTH);
        closesocket(newSock);
#else
        shutdown(newSock, SHUT_RDWR);
        close(newSock);
#endif
        return (SOCKET)0;
    }

    setsockopt(newSock, SOL_SOCKET, SO_SNDBUF, (char *)&nSndBuf, sizeof(nSndBuf));
    setsockopt(newSock, SOL_SOCKET, SO_RCVBUF, (char *)&nRcvBuf, sizeof(nRcvBuf));

    //
    // Fill in the address structure
    //
    struct sockaddr_in saServer;
    saServer.sin_family = AF_INET;
    saServer.sin_addr.s_addr = (DestInter && DestInter[0] ? inet_addr(DestInter) : INADDR_ANY);
    saServer.sin_port = htons(nPort); // Use port passed from user

    //
    // bind the name to the socket
    //

    nRet = bind(newSock,              // Socket descriptor
        (struct sockaddr*)&saServer,    // Address to bind to
        sizeof(struct sockaddr)         // Size of address
        );

    if (nRet)
    {
        PRINTERROR("bind()");

#ifdef WIN32
        shutdown(newSock, SD_BOTH);
        closesocket(newSock);
#else
        shutdown(newSock, SHUT_RDWR);
        close(newSock);
#endif
        return (SOCKET)0;
    }

    return newSock;
}

////////////////////////////////////////////////////////////

void MulticastServer(char* szMulticastAddress, char* szDataFile, int nSndBuf, int nRcvBuf, int nTempo, int nRecord, int nBinary)
{
    //
    // Create a UDP/IP Multicast socket
    //
    int                 nRet;
    char*               ptr;

    SOCKET*             SocketList;
    struct  ip_mreq*    MCastAddr;
    sockaddr_in*        AddrList;
    char**              DescAddr;
    char**              DescInter;
    unsigned short*     PortList;
    int                 Index, AddrCount;

    SOCKET              DestSocket = (SOCKET)0;
    sockaddr_in         DestAddr;
    char*               DestInter = NULL;
    unsigned short      DestPort = 0;

    if(ptr = strchr(szDataFile, ':'))
    {
        DestInter = strchr(szDataFile, '-');
        (*ptr) = 0;
        DestPort = atoi(&ptr[1]);
    }

    AddrCount = 0;
    {
        char seps[]   = "|;,\t\r\n";
        char *token;
        char *port;
        size_t len;

        len = strlen(szMulticastAddress);
        token = strtok(szMulticastAddress, seps);
        while(token != NULL)
        {
            port = strchr(token, ':');
            if(!port)
            {
                PRINTERROR("\nWarning: No port specified");
                return;
            }
            AddrCount++;
            token = strtok(NULL, seps);
        }
        replaceByte(szMulticastAddress, len, '\0', ',');
    }

    if(AddrCount)
    {
        SocketList = (SOCKET*)malloc(sizeof(SOCKET) * AddrCount);
        MCastAddr = (struct  ip_mreq*)malloc(sizeof(struct  ip_mreq) * AddrCount);
        AddrList = (struct  sockaddr_in*)malloc(sizeof(sockaddr_in) * AddrCount);
        DescAddr = (char**)malloc(sizeof(char*) * AddrCount);
        DescInter = (char**)malloc(sizeof(char*) * AddrCount);
        PortList = (unsigned short*)malloc(sizeof(unsigned short) * AddrCount);
    }
    else
    {
        PRINTERROR("\nWarning: No address specified");
        return;
    }

    int MaxFd = 0;
    fd_set ReadMask0;
    FD_ZERO(&ReadMask0);

#define MAX_SOCKET() (MaxFd)
#define ADD_SOCKET(sock) \
    { \
        FD_SET((unsigned int)sock, &ReadMask0); \
        if(MaxFd < (int)sock) \
        { \
            MaxFd = (int)sock; \
        } \
    }

    Index = 0;
    {
        char seps[]   = "|;,\t";
        char *token;
        char *port;
        char *inter;
        size_t len;

        len = strlen(szMulticastAddress);
        token = strtok(szMulticastAddress, seps);
        while(token != NULL)
        {
            DescAddr[Index] = (char*)malloc(strlen(token) + 1);
            DescInter[Index] = (char*)malloc(strlen(token) + 1);
            strcpy(DescAddr[Index], token);
            port = strchr(DescAddr[Index], ':');
            inter = strchr(DescAddr[Index], '-');

            (*port++) = 0; // no check needed, already validated
            PortList[Index] = atoi(port);

            if(inter)
            {
                (*inter++) = 0;
                strcpy(DescInter[Index], inter);
                MCastAddr[Index].imr_interface.s_addr = inet_addr(DescInter[Index]);
            }
            else
            {
                strcpy(DescInter[Index], "");
                MCastAddr[Index].imr_interface.s_addr = INADDR_ANY;
            }
            MCastAddr[Index].imr_multiaddr.s_addr = inet_addr(DescAddr[Index]);

            AddrList[Index].sin_family = AF_INET;
            AddrList[Index].sin_port = htons(PortList[Index]);
            AddrList[Index].sin_addr.s_addr = inet_addr(DescAddr[Index]);

            SocketList[Index] = CreateAndBindSocket((nRecord ? PortList[Index] : 0), nSndBuf, nRcvBuf, DescInter[Index]);
            if(SocketList[Index])
            {
                ADD_SOCKET(SocketList[Index]);
            }
            else
            {
                goto CLEANUP;
            }

            Index++;
            token = strtok(NULL, seps);
        }
        replaceByte(szMulticastAddress, len, '\0', ',');
    }

    if(nRecord)
    {
        for(Index = 0; Index < AddrCount; Index++)
        {
            // Add address to multicast group
            if (setsockopt(SocketList[Index], IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &MCastAddr[Index], sizeof(struct  ip_mreq))==-1)
            {
                PRINTERROR("\nWarning: Could not add address to MCast group");
                goto CLEANUP;
            }
        }

        if(DestPort)
        {
            DestSocket = CreateAndBindSocket(0, nSndBuf, nRcvBuf, DestInter ? &DestInter[1] : NULL);
            if(!DestSocket)
            {
                goto CLEANUP;
            }

            DestAddr.sin_family = AF_INET;
            DestAddr.sin_port = htons(DestPort);
            DestAddr.sin_addr.s_addr = inet_addr(szDataFile);

            // Setting default IP address for multicast
            struct  in_addr   MCastDefAddr;

            MCastDefAddr.s_addr = INADDR_ANY;
            MCastAddr[0].imr_multiaddr.s_addr = INADDR_ANY;
            MCastAddr[Index].imr_interface.s_addr = DestInter ? inet_addr(&DestInter[1]) : INADDR_ANY;

            if (setsockopt(DestSocket, IPPROTO_IP, IP_MULTICAST_IF, (char *) &MCastDefAddr, sizeof(struct in_addr))==-1)
            {
                PRINTERROR("\nWarning: Could not set default MCast interface");
                goto CLEANUP;
            }
        }
    }
    else
    {
        for(Index = 0; Index < AddrCount; Index++)
        {
            // Setting default IP address for multicast
            struct  in_addr   MCastDefAddr;

            MCastDefAddr.s_addr = INADDR_ANY;
            MCastAddr[0].imr_multiaddr.s_addr = INADDR_ANY;
            MCastAddr[0].imr_interface.s_addr = DescInter[Index][0] ? inet_addr(DescInter[Index]) : INADDR_ANY;

            if (setsockopt(SocketList[Index], IPPROTO_IP, IP_MULTICAST_IF, (char *) &MCastDefAddr, sizeof(struct in_addr))==-1)
            {
                PRINTERROR("\nWarning: Could not set default MCast interface");
                goto CLEANUP;
            }
        }
    }

    //
    // Show the server name and port number
    //

    fprintf(stderr, "\n");

    for(Index = 0; Index < AddrCount; Index++)
    {
        fprintf(stdout, "%s - Group[%s]; Interface[%s]; Port[%d];\n", (nRecord ? "Client" : "Server"), DescAddr[Index], (DescInter[Index][0] ? DescInter[Index] : "INADDR_ANY"), PortList[Index]);
    }

    if(DestPort)
    {
        fprintf(stderr, "SndBuf[%d]; RcvBuf[%d]; Destination[%s:%d] Interface[%s]; Tempo[%d];", nSndBuf, nRcvBuf, szDataFile, (int)DestPort, (DestInter ? &DestInter[1] : "INADDR_ANY"), nTempo);
    }
    else
    {
        fprintf(stderr, "SndBuf[%d]; RcvBuf[%d]; DataFile[%s]; Tempo[%d];", nSndBuf, nRcvBuf, ((szDataFile[0] == '-' && szDataFile[1] == '\0') ? (nRecord ? "stdout" : "stdin") : szDataFile), nTempo);
    }

#ifdef WIN32
    int nMsgSize;
    int optlen;
    optlen = sizeof(nMsgSize);
    getsockopt(SocketList[0], SOL_SOCKET, SO_MAX_MSG_SIZE, (char *)&nMsgSize, &optlen);

    fprintf(stderr, " MsgSize[%d];", nMsgSize);
#endif

    fprintf(stderr, "\n\n");

    if(nRecord)
    {
        size_t nRecvCount = 0;

        do
        {
            size_t nBufSize = nRcvBuf;
            char* szBuffer = 0;
            size_t nCount = 0;
            FILE* fpDataFile = NULL;

            if(!DestPort)
            {
                fpDataFile = (szDataFile[0] == '-' && szDataFile[1] == '\0') ? stdout : fopen(szDataFile,"wb");
                if (!fpDataFile) BREAKERROR_2("\nInvalid filename[%s]\n", szDataFile);
            }

            nBufSize = AllocateBuffer(&szBuffer, nBufSize);

            while(nBufSize)
            {
                fd_set readmask = ReadMask0;
                struct timeval timer_delay = {1, 0};

                int NbEvt = 0;
                int relVal = select(MAX_SOCKET() + 1, &readmask, NULL, NULL, &timer_delay);

                if (relVal == 0)
                {
                    continue;
                }
                else if (relVal < 0)
                {
                    int e = errno;
                    if (e != EINTR)
                    {
                        PRINTERROR("SELECT returned error");
                        goto CLEANUP;
                    }
                }

                for(Index = 0; (NbEvt < relVal) && (Index < AddrCount); Index++)
                {
                    if(FD_ISSET(SocketList[Index], &readmask))
                    {
                        ++NbEvt;

                        nRet = recvfrom(SocketList[Index], szBuffer, (int)nBufSize, 0, NULL, NULL);
                        if(nRet < 0)
                        {
                            nBufSize = 0; nTempo = 0;
                            PRINTERROR("recvfrom()");
                            goto CLEANUP;
                        }
                        nCount = nRet;
                        nRecvCount++;

                        if(DestPort)
                        {
                            char*   _szTmpBuffer = szBuffer;
                            size_t  _nCount = nCount;

                            while(_nCount > 0)
                            {
                                nRet = sendto(DestSocket, _szTmpBuffer, (int)_nCount, MSG_DONTROUTE, (struct sockaddr *)&DestAddr, sizeof(DestAddr));
                                if(nRet < 0)
                                {
                                    nBufSize = 0; nTempo= 0;
                                    PRINTERROR("sendto()");
                                    goto CLEANUP;
                                }
                                _szTmpBuffer += nRet;
                                _nCount -= nRet;
                            }

                            if(AddrCount > 1) fprintf(stdout, "[%s-%s:%d] ", DescAddr[Index], (DescInter[Index][0] ? DescInter[Index] : "INADDR_ANY"), PortList[Index]);
                            if(nBinary)
                            {
                                PrintString(szBuffer, stdout, (int)nCount);
                            }
                            else
                            {
                                fwrite(szBuffer, 1, nCount, stdout);
                            }
                            fputs("\n", stdout);
                        }
                        else if(fpDataFile == stdout)
                        {
                            if(AddrCount > 1) fprintf(stdout, "[%s-%s:%d] ", DescAddr[Index], (DescInter[Index][0] ? DescInter[Index] : "INADDR_ANY"), PortList[Index]);
                            if(nBinary)
                            {
                                PrintString(szBuffer, stdout, (int)nCount);
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
                }
            }

            AllocateBuffer(&szBuffer, 0);
            if(fpDataFile && (fpDataFile != stdout)) fclose(fpDataFile);
        }
        while(false);

        printf("\nRecvFromCount[%d]\n", nRecvCount);
    }
    else
    {
        size_t nSendCount = 0;

        do
        {
            size_t nBufSize = 1024;
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
                    if(!fgets(szBuffer, (int)nBufSize, stdin))
                    {
                        goto CLEANUP;
                    }

                    nCount = strlen(szBuffer);
                    if(!nCount)
                    {
                        goto CLEANUP;
                    }

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
                        goto CLEANUP;
                    }

                    nCount = DecodeI2((unsigned char *)&nCount);

                    if(nCount > 1024 * 32) BREAKERROR_2("\nInvalid run length in file[%s]\n", szDataFile);

                    if(nCount > nBufSize)
                        if(!(nBufSize = AllocateBuffer(&szBuffer, nCount)))
                        {
                            BREAKERROR_1("\nFailed to allocate memory\n");
                        }

                    if(fread(szBuffer, nCount, 1, fpDataFile) != 1) BREAKERROR_2("\nInvalid data format in file[%s]\n", szDataFile);

                    szTmpBuffer = szBuffer + 1; nCount--;
                }

                for(Index = 0; Index < AddrCount; Index++)
                {
                    char*   _szTmpBuffer = szTmpBuffer;
                    size_t  _nCount = nCount;

                    while(_nCount > 0)
                    {
                        nRet = sendto(SocketList[Index], _szTmpBuffer, (int)_nCount, MSG_DONTROUTE, (struct sockaddr *)&AddrList[Index], sizeof(AddrList[Index]));
                        if(nRet < 0)
                        {
                            nBufSize = 0; nTempo= 0;
                            PRINTERROR("sendto()");
                            goto CLEANUP;
                        }
                        _szTmpBuffer += nRet;
                        _nCount -= nRet;
                        nSendCount++;
                    }
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

CLEANUP:

    for(Index = 0; Index < AddrCount; Index++)
    {
#ifdef WIN32
        shutdown(SocketList[Index], SD_BOTH);
        closesocket(SocketList[Index]);
#else
        shutdown(SocketList[Index], SHUT_RDWR);
        close(SocketList[Index]);
#endif
    }

    return;
}

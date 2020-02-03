#include "Logging.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#else
#define SOCKET int
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#endif

void DisplayUsage(FILE * file)
{
    if(file)
    {
        fprintf(file, "\nSyntax: Proxy.exe [SERVER_PORT] [HOST_NAME] [HOST_PORT]\n\n");
        fflush(file);
    }
}

void DisplayDetailedUsage(FILE * file)
{
    if(file)
    {
        fprintf(file, "\nSyntax: Proxy.exe {OPTIONS} [SERVER_PORT] [HOST_NAME] [HOST_PORT] {PROXY_NAME} {PROXY_PORT}\n");
        fprintf(file, "\n");
        fprintf(file,   "  Options:\n");
        fprintf(file,   "           --rawLog\n");
        fprintf(file,   "           --GLLog\n");
        fprintf(file,   "           --ProxyServer\n");
        fprintf(file,   "           --ProxyClient\n");
        fprintf(file,   "           --lzo\n");
        fprintf(file,   "           --ice\n");
        fprintf(file,   "           --tea\n");
        fprintf(file,   "           --fish\n");
        fprintf(file,   "           --statistics\n");
        fprintf(file,   "           --help\n");
        fprintf(file, "\n");
        fflush(file);
    }
}

void GetLocalTimeAsString(char* szBuffer)
{
    time_t		sysTime;
    struct tm	localTime;

    time(&sysTime);
#ifdef WIN32
    localtime_s(&localTime, &sysTime);
#else
    localtime_r(&sysTime, &localTime);
#endif

    sprintf(szBuffer, "%.4d/%.2d/%.2d %.2d:%.2d:%.2d",
        localTime.tm_year+1900,
        localTime.tm_mon+1,
        localTime.tm_mday,
        localTime.tm_hour,
        localTime.tm_min,
        localTime.tm_sec);
}

void DisplayServerStartUp(short nServerPort, char* nHostName, short nHostPort, char* nProxyName, short nProxyPort, PSERVER_FLAGS flags, PSERVER_KEYS keys, FILE * file)
{
    if(file)
    {
        char szBuf[256];
        char szCurrentTime[32];

        if(gethostname(szBuf, sizeof(szBuf))) sprintf(szBuf, "127.0.0.1");

        GetLocalTimeAsString(szCurrentTime);

        // Show the server name and port number
        if(nHostName && nHostName[0])
        {
            if(nProxyName && nProxyName[0])
            {
                fprintf(file, "\n%s : Proxy[%s:%d]; Host[%s:%d]; Proxy[%s:%d]\n", szCurrentTime, szBuf, nServerPort, nHostName, nHostPort, nProxyName, nProxyPort);
            }
            else
                fprintf(file, "\n%s : Proxy[%s:%d]; Host[%s:%d];\n", szCurrentTime, szBuf, nServerPort, nHostName, nHostPort);
        }
        else
            fprintf(file, "\n%s : Proxy[%s:%d];\n", szCurrentTime, szBuf, nServerPort);

        if(flags)
        {
            if(flags->RawLog || flags->GLLog || flags->ProxyServer || flags->ProxyClient ||
                flags->LZO || flags->ICE || flags->TEA || flags->FISH || flags->Statistics)
                fputs("Running with options: \n", file);

            if(flags->RawLog)      fputs(" --rawLog\n", file);
            if(flags->GLLog)       fputs(" --GLLog\n", file);
            if(flags->ProxyServer) fputs(" --ProxyServer\n", file);
            if(flags->ProxyClient) fputs(" --ProxyClient\n", file);
            if(flags->LZO)         fputs(" --lzo\n", file);
            if(flags->ICE)         fprintf(file, " --ice%d\n", keys->Ice);
            if(flags->TEA)         fprintf(file, " --tea%d\n", keys->Tea);
            if(flags->FISH)        fprintf(file, " --fish\n");
            if(flags->Statistics)  fputs(" --statistics\n", file);
        }

        fflush(file);
    }
}

void LogRawData(char* szData, int nCount, FILE * file, char* szBuffer)
{
    if(file)
    {
        int nCountOut = ConvertString(szData, szBuffer, nCount);
        char szCurrentTime[32];

        szBuffer[nCountOut] = '\0';

        GetLocalTimeAsString(szCurrentTime);

        fprintf(file, "%s : -->>> RawData[%.4d]{%.*s}\n", szCurrentTime, nCount, nCountOut, szBuffer);
        fflush(file);
    }
}
void LogConnection(int IsOpen, char* szRemoteName, char* type, int id, int total, FILE * file)
{
    if(file)
    {
        char* szOpen = "Open";
        char* szClose = "Close";

        char szCurrentTime[32];
        GetLocalTimeAsString(szCurrentTime);

        fprintf(file, "%s : %s %s [%s][%02d/%02d] {%s connection}\n", szCurrentTime, szRemoteName, (IsOpen ? "--->>": "<<---"), type, id, total, (IsOpen ? szOpen : szClose));
        fflush(file);
    }
}

void LogProxy(char* szRemoteName, char* message, char* hostName, int hostPort, int id, FILE * file)
{
    if(file)
    {
        char szCurrentTime[32];
        GetLocalTimeAsString(szCurrentTime);

        fprintf(file, "%s : %s %s [INFO][%d] {%s <%s:%d>}\n", szCurrentTime, szRemoteName, "-----", id, message, hostName, hostPort);
        fflush(file);
    }
}

void LogSocketStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file)
{
    if(file)
    {
        char szCurrentTime[32];
        GetLocalTimeAsString(szCurrentTime);
        if(id)
            fprintf(file, "%s : %s %s [STATISTICS][%d] {RecvBytes<%u> SendBytes<%u>}\n", szCurrentTime, szRemoteName, "-----", id, stats->RecvBytes, stats->SendBytes);
        else
            fprintf(file, "%s : %s %s [STATISTICS] {RecvBytes<%u> SendBytes<%u>}\n", szCurrentTime, szRemoteName, "-----", stats->RecvBytes, stats->SendBytes);

        fflush(file);
    }
}

void LogLzoStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file)
{
    if(file)
    {
        char szCurrentTime[32];
        GetLocalTimeAsString(szCurrentTime);

        if(id)
            fprintf(file, "%s : %s %s [STATISTICS][%d] {LzoIn<%lld> LzoOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", id, stats->LzoIn, stats->LzoOut);
        else
            fprintf(file, "%s : %s %s [STATISTICS] {LzoIn<%lld> LzoOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", stats->LzoIn, stats->LzoOut);

        fflush(file);
    }
}

void LogFishStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file)
{
    if(file)
    {
        char szCurrentTime[32];
        GetLocalTimeAsString(szCurrentTime);

        if(id)
            fprintf(file, "%s : %s %s [STATISTICS][%d] {FishIn<%lld> FishOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", id, stats->FishIn, stats->FishOut);
        else
            fprintf(file, "%s : %s %s [STATISTICS] {FishIn<%lld> FishOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", stats->FishIn, stats->FishOut);

        fflush(file);
    }
}

void LogIceStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file)
{
    if(file)
    {
        char szCurrentTime[32];
        GetLocalTimeAsString(szCurrentTime);

        if(id)
            fprintf(file, "%s : %s %s [STATISTICS][%d] {IceIn<%lld> IceOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", id, stats->IceIn, stats->IceOut);
        else
            fprintf(file, "%s : %s %s [STATISTICS] {IceIn<%lld> IceOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", stats->IceIn, stats->IceOut);

        fflush(file);
    }
}

void LogTeaStatistics(char* szRemoteName, PSERVER_STATS stats, int id, FILE * file)
{
    if(file)
    {
        char szCurrentTime[32];
        GetLocalTimeAsString(szCurrentTime);

        if(id)
            fprintf(file, "%s : %s %s [STATISTICS][%d] {TeaIn<%lld> TeaOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", id, stats->TeaIn, stats->TeaOut);
        else
            fprintf(file, "%s : %s %s [STATISTICS] {TeaIn<%lld> TeaOut<%lld>}\n", szCurrentTime, szRemoteName, "-----", stats->TeaIn, stats->TeaOut);

        fflush(file);
    }
}

#define  PLUS(td)     (td + 32)
#define  P1(oct,bit)  ( oct >> bit )
#define  P0(oct,bit)  ( oct - (( oct >> bit ) << bit ))

void To_2_oct ( short l , unsigned char *tb )
{
    tb[1] = (unsigned char)PLUS(P1(l, 7));
    tb[0] = (unsigned char)PLUS(P0(l, 7));
}

#define BLOCK_FEED_MAX (28670)

void WriteGLFeed(int LgMess, char* Mess, FILE * file)
{
    if(file)
    {
        char chaine[16];
        int NbBytes, Bytes2Write;

        for(NbBytes = 0; NbBytes < LgMess; NbBytes += Bytes2Write)
        {
            Bytes2Write = (NbBytes + BLOCK_FEED_MAX > LgMess) ? LgMess - NbBytes : BLOCK_FEED_MAX;

            To_2_oct((short)(Bytes2Write+1),(unsigned char *)chaine);
            fprintf(file, "%-2.2s ", chaine);

            fwrite(&Mess[NbBytes], 1, Bytes2Write, file);
        }
        fflush(file);
    }
}

static long long    gFrequency = 0;

#ifdef WIN32

void InitPrecisionTime()
{
    LARGE_INTEGER   counter;

    QueryPerformanceFrequency(&counter);
    gFrequency = counter.QuadPart / 1000 / 1000;
}

long long GetSimplePrecisionTime()
{
    LARGE_INTEGER   counter;
    if(gFrequency == 0) InitPrecisionTime();

    QueryPerformanceCounter(&counter);
    return  counter.QuadPart / gFrequency;
}

#else

long long GetSimplePrecisionTime()
{
    struct timeval  tpcur;
    memset(&tpcur, 0, sizeof(tpcur));
    gettimeofday(&tpcur,0);
    return (tpcur.tv_usec + (tpcur.tv_sec * 1000 * 1000));
}

#endif

#ifdef WIN32
long GetElapsedTime()
{
    return (long)GetTickCount();
}
#else
long GetElapsedTime()
{
    return (long)clock();
}
#endif

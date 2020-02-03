#include "Logging.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#else
#define SOCKET int
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#endif

void DisplayUsage(FILE * file)
{
    if(file)
    {
        fprintf(file, "\nSyntax: SimpleProxy  {-glnrs} [SERVER_PORT] [HOST_NAME] [HOST_PORT] {SO_SNDBUF} {SO_RCVBUF}\n");
        fprintf(file, "\n");
        fprintf(file, "  Options:\n");
        fprintf(file, "    -l  write raw log file\n");
        fprintf(file, "    -g  write GL feed log\n");
        fprintf(file, "    -r  replace hostname in datastream\n");
        fprintf(file, "    -s  use simple time when logging\n");
        fprintf(file, "    -n  don't flush log files\n");
        fprintf(file, "\n");

        fflush(file);
    }
}

#ifdef WIN32

static long long    gFrequency = 0;
static long long    gCreateTime = 0;
static int          gDateSnapshot;
static int          gTimeSnapshot;
static int          gIsDSTSnapshot;

void InitTimeSnapshot(int* dateSnapshot, int* timeSnapshot, int* isDSTSnapshot)
{
    time_t      sysTime;
    struct tm   localTime;

    time(&sysTime);
    localtime_s(&localTime, &sysTime);

    (*isDSTSnapshot) = localTime.tm_isdst;

    (*dateSnapshot) = 0;
    (*dateSnapshot) += localTime.tm_mday;
    (*dateSnapshot) += (localTime.tm_mon + 1) * 100;
    (*dateSnapshot) += (localTime.tm_year + 1900) * 10000;

    (*timeSnapshot) = 0;
    (*timeSnapshot) += localTime.tm_sec;
    (*timeSnapshot) += (localTime.tm_min) * 100;
    (*timeSnapshot) += (localTime.tm_hour) * 10000;
}

void InitPrecisionTime()
{
    LARGE_INTEGER   counter;

    QueryPerformanceFrequency(&counter);
    gFrequency = counter.QuadPart / 1000 / 1000;

    QueryPerformanceCounter(&counter);
    gCreateTime = counter.QuadPart;

    InitTimeSnapshot(&gDateSnapshot, &gTimeSnapshot, &gIsDSTSnapshot);
}

long long GetSimplePrecisionTime()
{
    LARGE_INTEGER   counter;
    if(gFrequency == 0) InitPrecisionTime();

    QueryPerformanceCounter(&counter);
    return  counter.QuadPart / gFrequency;
}

void GetPrecisionTime(int* pDate, int* pTime, int* pMilli, int* pMicro)
{
    LARGE_INTEGER   counter;
    long long       tmpTime, time = 0;

    int dateSnapshot;
    int timeSnapshot;
    int isDSTSnapshot;
    int hour, min, sec;

    if(gFrequency == 0 || gCreateTime == 0) InitPrecisionTime();

    {
        QueryPerformanceCounter(&counter);

        time += (counter.QuadPart - gCreateTime) / gFrequency;

        InitTimeSnapshot(&dateSnapshot, &timeSnapshot, &isDSTSnapshot);

        if(dateSnapshot != gDateSnapshot) 
        {
            time = 0;
            gCreateTime = counter.QuadPart;
            gDateSnapshot = dateSnapshot;
            gTimeSnapshot = timeSnapshot;
            gIsDSTSnapshot = isDSTSnapshot;
        }
        else
        {
            tmpTime = (time / 1000000);

            hour = (int)(tmpTime / 60 / 60);
            tmpTime -= (hour * 60 * 60);

            min = (int)(tmpTime / 60);
            tmpTime -= (min * 60);

            sec = (int)(tmpTime);
            tmpTime -= (sec);

            dateSnapshot = gDateSnapshot;
            timeSnapshot = gTimeSnapshot;
            isDSTSnapshot = gIsDSTSnapshot;

            ChangeDateTime(&dateSnapshot, &timeSnapshot, &isDSTSnapshot, 0, 0, 0, 0, hour, min, sec);
        }

        (*pDate) = dateSnapshot;
        (*pTime) = timeSnapshot;

        tmpTime = time % 1000000;

        (*pMilli) = (int)(tmpTime / 1000);
        (*pMicro) = (int)(tmpTime % 1000);
    }
}

#else

long long GetSimplePrecisionTime()
{
    struct timeval  tpcur;
    memset(&tpcur, 0, sizeof(tpcur));
    gettimeofday(&tpcur,0);
    return (tpcur.tv_usec + (tpcur.tv_sec * 1000 * 1000));
}

void GetPrecisionTime(int* pDate, int* pTime, int* pMilli, int* pMicro)
{
    struct timeval  tpcur;
    struct tm       localTime;
    time_t          sysTime;

    memset(&tpcur, 0, sizeof(tpcur));

    gettimeofday(&tpcur,0);

    (*pMilli) = tpcur.tv_usec / 1000;
    (*pMicro) = tpcur.tv_usec % 1000;

    sysTime = tpcur.tv_sec;
    localtime_r(&sysTime, &localTime);

    (*pDate) = 0;
    (*pDate) += localTime.tm_mday;
    (*pDate) += (localTime.tm_mon + 1) * 100;
    (*pDate) += (localTime.tm_year + 1900) * 10000;

    (*pTime) = 0;
    (*pTime) += localTime.tm_sec;
    (*pTime) += (localTime.tm_min) * 100;
    (*pTime) += (localTime.tm_hour) * 10000;
}

#endif

void ChangeDateTime(int* iDate, int* iTime, int* isDST, int mday, int mon, int year, int wday, int hour, int min, int sec)
{
    time_t      sysTime;
    struct tm   localTime;

    while(1)
    {
        int fixForDST = (*isDST) >= 0 ? 1 : 0;
        memset(&localTime, 0, sizeof(localTime));

        do
        {
            localTime.tm_isdst = (*isDST) == 1 ? 1 : 0;
            localTime.tm_year = (((*iDate) / 10000) - 1900) + year;
            localTime.tm_mon  = ((((*iDate) / 100) % 100) - 1) + mon;
            localTime.tm_mday = ((*iDate) % 100) + mday + wday;

            localTime.tm_hour = ((*iTime) / 10000) + hour;
            localTime.tm_min  = (((*iTime) / 100) % 100) + min;
            localTime.tm_sec  = ((*iTime) % 100) + sec;

            sysTime = mktime(&localTime);

            if(!fixForDST && localTime.tm_isdst == 1)
            {
                memset(&localTime, 0, sizeof(localTime));
                localTime.tm_isdst = 1;
                fixForDST = 1;
                continue;
            }
            else
                break;
        }
        while(1);

#ifdef WIN32
        localtime_s(&localTime, &sysTime);
#else
        localtime_r(&sysTime, &localTime);
#endif

        if(wday) 
        {
            if(localTime.tm_wday == 0)
                mday += 1;
            else if(localTime.tm_wday == 6)
                mday += 2;
            else
                break;
        }
        else
        {
            break;
        }
    }

    (*iDate) = 0;
    (*iDate) += localTime.tm_mday;
    (*iDate) += (localTime.tm_mon + 1) * 100;
    (*iDate) += (localTime.tm_year + 1900) * 10000;

    (*iTime) = 0;
    (*iTime) += localTime.tm_sec;
    (*iTime) += (localTime.tm_min) * 100;
    (*iTime) += (localTime.tm_hour) * 10000;

    (*isDST) = localTime.tm_isdst;
}

void CalcTime(int SimpleTime, ULONG_LONG* pSimpleTime, char* timeStr, int* pDate, int* pTime, int* pMilli, int* pMicro)
{
    if(SimpleTime)
    {
        (*pSimpleTime).QuadPart = GetSimplePrecisionTime();
        if((*pSimpleTime).QuadPart < 0)
            sprintf(timeStr, "-%019lld", ((*pSimpleTime).QuadPart * -1));
        else
            sprintf(timeStr, "+%019lld", (*pSimpleTime).QuadPart);
    }
    else
    {
        GetPrecisionTime(pDate, pTime, pMilli, pMicro);
        sprintf(timeStr, "%08d.%06d.%03d.%03d", (*pDate),(*pTime), (*pMilli), (*pMicro));
    }
}

void GetLocalTimeAsString(char* szBuffer)
{
    struct tm   localTime;
    int         milliseconds;

#ifdef WIN32
    SYSTEMTIME v_tpcur;
    GetLocalTime(&v_tpcur);
    milliseconds = v_tpcur.wMilliseconds;
    localTime.tm_sec = v_tpcur.wSecond;
    localTime.tm_min = v_tpcur.wMinute;
    localTime.tm_hour = v_tpcur.wHour;
    localTime.tm_mday = v_tpcur.wDay;
    localTime.tm_mon = v_tpcur.wMonth - 1;
    localTime.tm_year = v_tpcur.wYear - 1900;
#else
    struct timeval v_tpcur;
    gettimeofday(&v_tpcur,0);
    milliseconds=v_tpcur.tv_usec / 1000;
    time_t timer = v_tpcur.tv_sec;
    localtime_r(&timer, &localTime);
#endif

    sprintf(szBuffer, "%.4d%.2d%.2d.%.2d%.2d%.2d.%.3d", 
        localTime.tm_year+1900, 
        localTime.tm_mon+1, 
        localTime.tm_mday, 
        localTime.tm_hour, 
        localTime.tm_min, 
        localTime.tm_sec, 
        milliseconds);
}

void DisplayServerStartUp(PSERVER_ARGS serverArgs, FILE * file)
{
    if(file)
    {
        char szBuf[256];
        char timeStr[32];
        int date, time, milli, micro;
        ULONG_LONG simpleTime;

        if(gethostname(szBuf, sizeof(szBuf))) sprintf(szBuf, "127.0.0.1");

        CalcTime(serverArgs->flags.simpleTime, &simpleTime, timeStr, &date, &time, &milli, &micro);
        fprintf(file, "\n%s : Proxy[%s:%d];  Host[%s:%d]; SndBuf[%d]; RcvBuf[%d];\n", timeStr, szBuf, serverArgs->nServerPort, serverArgs->sHostName, serverArgs->nHostPort, serverArgs->nSndBuf, serverArgs->nRcvBuf);
        fflush(file);
    }
}

size_t LogRawDataCallback(char *buffer, size_t size, size_t nitems, void *userp)
{
    PLogInfo info = (PLogInfo)userp;
    LogRawData(info->flags, buffer, (int)(size*nitems), info->file, info->direction, info->szBuffer);
    return size*nitems;
}

void LogRawData(PSERVER_FLAGS flags, char* szData, int nCount, FILE * file, int direction, char* szBuffer)
{
    if(file)
    {
        char timeStr[32];
        int date, time, milli, micro;
        ULONG_LONG simpleTime;

        int nCountOut = ConvertString(szData, szBuffer, nCount);
        char* prefix = "-----";

        szBuffer[nCountOut] = '\0';

        switch(direction)
        {
        case DataIn:
            prefix = "<<---";
            break;
        case DataOut:
            prefix = "--->>";
            break;
        }

        CalcTime(flags->simpleTime, &simpleTime, timeStr, &date, &time, &milli, &micro);
        fprintf(file, "%s : %s RawData[%.4d]{%.*s}\n", timeStr, prefix, nCount, nCountOut, szBuffer);
        if(!flags->noFlush) fflush(file);
    }
}
void LogConnection(PSERVER_FLAGS flags, int IsOpen, char* szRemoteName, char* type, FILE * file)
{
    if(file)
    {
        char timeStr[32];
        int date, time, milli, micro;
        ULONG_LONG simpleTime;

        char* szOpen = "Open";
        char* szClose = "Close";

        CalcTime(0, &simpleTime, timeStr, &date, &time, &milli, &micro);
        fprintf(file, "%s : %s %s [%s] {%s connection}\n", timeStr, szRemoteName, (IsOpen ? "--->>": "<<---"), type, (IsOpen ? szOpen : szClose));
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

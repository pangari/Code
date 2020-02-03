#include "Logging.h"
#include "base64.h"

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
        fprintf(file, "\nSyntax: Talker.exe [SERVER_PORT]\n\n");
        fflush(file);
    }
}

void DisplayDetailedUsage(FILE * file)
{
    if(file)
    {
        fprintf(file, "\nSyntax: Talker.exe {OPTIONS} [SERVER_PORT]\n");
        fprintf(file, "\n");
        fprintf(file,   "  Options:\n");
        fprintf(file,   "           --rawLog\n");
        fprintf(file,   "           --screenLog\n");
        fprintf(file,   "           --base64\n");
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

void DisplayServerStartUp(short nServerPort, PSERVER_FLAGS flags, FILE * file)
{
    if(file)
    {
        char szBuf[256];
        char szCurrentTime[32];

        if(gethostname(szBuf, sizeof(szBuf))) sprintf(szBuf, "127.0.0.1");

        GetLocalTimeAsString(szCurrentTime);

        fprintf(file, "\n%s : Talker[%s:%d];\n", szCurrentTime, szBuf, nServerPort);

        if(flags)
        {
            if(flags->RawLog || flags->ScreenLog || flags->Base64)
                fputs("Running with options: \n", file);

            if(flags->RawLog)      fputs(" --rawLog\n", file);
            if(flags->ScreenLog)   fputs(" --screenLog\n", file);
            if(flags->Base64)      fputs(" --base64\n", file);
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

void LogToScreen(char* szData, int nCount, FILE * file, char* szBuffer)
{
    if(file)
    {
        int nCountOut = ConvertString(szData, szBuffer, nCount);
        //szBuffer[nCountOut++] = '\r';
        szBuffer[nCountOut++] = '\n';
        szBuffer[nCountOut] = '\0';
        fputs(szBuffer, file);
        fflush(file);
    }
}

void LogToScreenBase64(char* szData, int nCount, FILE * file, char* szBuffer)
{
    if(file)
    {
        int nWrite = 0;
        char* inBuf = szData;
        char* outBuf = szBuffer;

        while(nCount > 0)
        {
            encodeBase64((unsigned char*)inBuf, (unsigned char*)outBuf, ((nCount  < 3) ? nCount : 3));
            nCount -= 3;
            inBuf += 3;
            outBuf += 4;
            nWrite +=4;
            if(nWrite >= 72)
            {
                (*outBuf++) = '\n';
                (*outBuf++) = '\0';
                fputs(szBuffer, file);
                fflush(file);
                outBuf = szBuffer;
                nWrite = 0;
            }
        }

        //(*outBuf++) = '\r';
        (*outBuf++) = '\n';
        (*outBuf++) = '\0';
        fputs(szBuffer, file);
        fflush(file);
    }
}

void LogConnection(int IsOpen, char* szRemoteName, char* type, int id, int total, FILE * file)
{
    if(file)
    {
        const char* szOpen = "Open";
        const char* szClose = "Close";

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
